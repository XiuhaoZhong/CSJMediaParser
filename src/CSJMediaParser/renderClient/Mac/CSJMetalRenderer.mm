#include "CSJMediaEngine/CSJMediaRawData.h"

#include <atomic>

#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

using csjmediaengine::CSJVideoData;
using csjmediaengine::CSJVideoFormatType;

#import "CSJMediaShaderTypes.h"
#import "CSJMetalRenderer.h"
#import "CSJMetalHelper.h"

// Define the index array.
// Triangle 1: 0 -> 1 -> 2 (left-bottom, right-bottom, left-top)
// Triangle 2: 1 -> 3 -> 2 (right-bottom, right-top, left-top)
// Notice: In metal clockwise is the positive direction.
static uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };

@interface CSJMetalRenderer ()

@property(nonatomic, assign) CGFloat width;
@property(nonatomic, assign) CGFloat height;

@property(nonatomic, assign) NSInteger videoWidth;
@property(nonatomic, assign) NSInteger videoHeight;

@property(nonatomic, assign) MTLPixelFormat                  videoPixelFmt;
@property(nonatomic, strong) MTLVertexDescriptor            *vertexDescriptor;
@property(nonatomic, strong) MTLRenderPipelineDescriptor    *renderPipeLineDesc;
@property(nonatomic, strong) CAMetalLayer                   *metalLayer;

@property(nonatomic, strong) id<MTLTexture>                  texY;
@property(nonatomic, strong) id<MTLTexture>                  texU;
@property(nonatomic, strong) id<MTLTexture>                  texV;
@property(nonatomic, strong) id<MTLTexture>                  texRGBA;

@property(nonatomic, strong) id<MTLDevice>                   device;
@property(nonatomic, strong) id<MTLCommandQueue>             commandQueue;

@property(nonatomic, strong) MTLVertexDescriptor            *rgbaVertDesc;
@property(nonatomic, strong) id<MTLRenderPipelineState>      rgbaPipeline;
@property(nonatomic, strong) id<MTLBuffer>                   rgbaVertexBuffer;
@property(nonatomic, strong) id<MTLBuffer>                   rgbaIndexBuffer;
@property(nonatomic, strong) NSMutableArray<id<MTLTexture>> *rgbaTexs;
@property(nonatomic, strong) MTLTextureDescriptor           *texDescriptor;

@property(nonatomic, assign) NSUInteger                      currentTexIdx;
@property(nonatomic, assign) NSUInteger                      renderMode; // 0: null, 1:rgba, 2:yuv

@property(nonatomic, strong) NSString                       *imagePath;
@property(nonatomic, strong) NSLock                         *videoDataLock;

@end

@implementation CSJMetalRenderer {
    std::atomic<bool>     _contentUpdate;
    CSJVideoFormatType    _videoFmt;;
}

- (instancetype)initWithFrameWithView:(NSView *)view 
                                frame:(CGRect)frame 
                           pixelRatio:(float)pixelRatio {
    self = [super init];

    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];

    _width = frame.size.width;
    _height = frame.size.height;
    view.wantsLayer = YES;

    _metalLayer = [CAMetalLayer layer];
    _metalLayer.device = _device;
    _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    _metalLayer.framebufferOnly = YES;
    _metalLayer.opaque = YES;

    view.layer = _metalLayer;

    [self updateDrawable:CGRectGetWidth(frame)
                  height:CGRectGetHeight(frame)
              pixelRatio:pixelRatio];

    _videoWidth = 0;
    _videoHeight = 0;

    _rgbaTexs = nil;
    _currentTexIdx = 0;
    _renderMode = 0;

    _rgbaPipeline = nil;

    _contentUpdate = false;

    return self;
}

- (void)dealloc {
    NSLog(@"CSJMTKRenderer is dealloc");
}

- (void)releaseResources {

}

- (void)setupRGBAPipeline {
    if (!_device) {
        return ;
    }

    id<MTLLibrary> lib = [_device newDefaultLibrary];
    id<MTLFunction> vertFunc = [lib newFunctionWithName:@"rbgaVertShader"];
    id<MTLFunction> fragFunc = [lib newFunctionWithName:@"rgbaFragShader"];

    MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
    desc.vertexFunction = vertFunc;
    desc.fragmentFunction = fragFunc;
    desc.colorAttachments[0].pixelFormat = _metalLayer.pixelFormat;

    NSError *err = nil;
    _rgbaPipeline = [_device newRenderPipelineStateWithDescriptor:desc error:&err];

    _rgbaVertDesc = [[MTLVertexDescriptor alloc] init];
    _rgbaVertDesc.attributes[0].format = MTLVertexFormatFloat3;
    _rgbaVertDesc.attributes[0].offset = offsetof(struct Vertex, position);
    _rgbaVertDesc.attributes[0].bufferIndex = 0;

    _rgbaVertDesc.attributes[1].format = MTLVertexFormatFloat2;
    _rgbaVertDesc.attributes[1].offset = offsetof(struct Vertex, texCoord);
    _rgbaVertDesc.attributes[1].bufferIndex = 0;

    _rgbaVertDesc.layouts[0].stride = sizeof(struct Vertex);

    _rgbaVertexBuffer = [_device newBufferWithBytes:vertices
                                             length:sizeof(vertices)
                                            options:MTLResourceStorageModeShared];

    _rgbaIndexBuffer = [_device newBufferWithBytes:indices 
                                            length:sizeof(indices) 
                                           options:MTLResourceStorageModeShared];

    // Setting vertex buffer into encoder.
    // [encoder setVertexBuffer:_rgbaVertexBuffer offset:0 atIndex:0];
    // [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
}

- (void)updateDrawable:(NSInteger)width height:(NSInteger)height pixelRatio:(float)pixelRatio {
    if (!_metalLayer) {
        return ;
    }

    NSInteger w = width * pixelRatio;
    NSInteger h = height * pixelRatio;

    _metalLayer.drawableSize = CGSizeMake(w, h);
}

- (void)setImageWithPath:(NSString *)imagePath {
    _imagePath = [imagePath copy];
    _renderMode = 1;
    _contentUpdate = true;
}

- (void)updateDrawableSizeWithWidth:(NSInteger)width 
                             height:(NSInteger)height 
                         pixelRatio:(float)pixelRatio {

    if (!_metalLayer) {
        return ;
    }

    NSInteger w = width * pixelRatio;
    NSInteger h = height * pixelRatio;

    _metalLayer.drawableSize = CGSizeMake(w, h);
}

- (void)drawContent {
    if (!_metalLayer) {
        return ;
    }

    id<CAMetalDrawable> drawable = _metalLayer.nextDrawable;
    id<MTLCommandQueue> command_queue = _device.newCommandQueue;
    id<MTLCommandBuffer> command_buffer = command_queue.commandBuffer;

    MTLRenderPassDescriptor *pass = MTLRenderPassDescriptor.renderPassDescriptor;
    pass.colorAttachments[0].texture = drawable.texture;
    pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 1.0, 1.0);

    id<MTLRenderCommandEncoder> renderEncoder = [command_buffer renderCommandEncoderWithDescriptor:pass];

    // add render command;

    [renderEncoder endEncoding];

    [command_buffer presentDrawable:drawable];
    [command_buffer commit];
}

// invoke when draw content.
// - (void)drawInMTKView:(MTKView *)view {

//     @autoreleasepool {
//         id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
//         commandBuffer.label = @"Render Commands";

//         MTLRenderPassDescriptor* onScreenDescriptor = view.currentRenderPassDescriptor;
//         if (onScreenDescriptor != nil) {
//             id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:onScreenDescriptor];

//             //onScreenDescriptor.colorAttachments[0].texture = drawable.texture;
//             //onScreenDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
//             //onScreenDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 1.0, 1.0, 1.0);
//             _renderView.clearColor = MTLClearColorMake(0.0, 0.5, 0.5, 1.0);

//             MTLViewport viewPort = {0, 0, _width, _height, 0.0, 1.0};
//             [renderEncoder setViewport:viewPort];

//             [renderEncoder endEncoding];

//             if (view.currentDrawable) {
//                 [commandBuffer presentDrawable:view.currentDrawable];
//             }
//         }

//         [commandBuffer commit];
//     }
// }

// invoked when view's layout changed, including change resolution, size.
// - (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
//     NSLog(@"MTKView size has changed!");
// }

// - (void)loadMetalWithView:(nonnull MTKView *)view {
//     view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
//     view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

//     _vertexDescriptor = [[MTLVertexDescriptor alloc] init];

//     // Position
//     _vertexDescriptor.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
//     _vertexDescriptor.attributes[VertexAttributePosition].offset = 0;
//     _vertexDescriptor.attributes[VertexAttributePosition].bufferIndex = CSJMediaVertexBufferIndex;

//     // texCoord
//     _vertexDescriptor.attributes[VertexAttributeTexCoord].format = MTLVertexFormatFloat2;
//     _vertexDescriptor.attributes[VertexAttributeTexCoord].offset = 12;
//     _vertexDescriptor.attributes[VertexAttributeTexCoord].bufferIndex = CSJMediaVertexBufferIndex;
// }

- (void)loadVideoComponentWithPixelFmt:(NSInteger)pixelFmt width:(NSInteger)width height:(NSInteger)height {
    switch (pixelFmt) {
    case csjmediaengine::CSJVIDEO_FMT_YUV420P:
        [self loadYUV420PComponentsWithWidth:width height:height];
        break;
    default:
        break;
    }

    _videoWidth = width;
    _videoHeight = height;
    _videoFmt = (CSJVideoFormatType)pixelFmt;
}

- (void)updateVideoFrameWithData:(void *)pData {
    CSJVideoData *videoData = static_cast<CSJVideoData *>(pData);

    if (!videoData) {
        return ;
    }

    [_videoDataLock lock];

    if (videoData->getFmtType() != _videoFmt ||
        videoData->getWidth() != _videoWidth ||
        videoData->getHeight() != _videoHeight) {

        // reallocate the video buffers.
        // reallocate the textures.
        [self loadVideoComponentWithPixelFmt:videoData->getFmtType()
                                    width:videoData->getWidth()
                                    height:videoData->getHeight()];
    }

    // opy data to buffer
    switch (_videoFmt) {
    case csjmediaengine::CSJVIDEO_FMT_YUV420P:
        [self updateDatatoYUV420PWidthData:videoData];
        break;
    default:
        break;
    }

    [_videoDataLock unlock];
}

- (void)loadYUV420PComponentsWithWidth:(NSInteger)width height:(NSInteger)height {
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                                                                      width:(NSInteger)width
                                                                                      height:(NSInteger)height
                                                                                   mipmapped:NO];

    _texY = [_device newTextureWithDescriptor:texDesc];

    texDesc.width = width / 2;
    texDesc.height = height / 2;

    _texU = [_device newTextureWithDescriptor:texDesc];
    _texV = [_device newTextureWithDescriptor:texDesc];
}

- (void)updateDatatoYUV420PWidthData:(CSJVideoData *)videoData {
    MTLRegion regionY = {{0,0,0}, {(NSUInteger)_videoWidth, (NSUInteger)_videoHeight, 1}};
    [_texY replaceRegion:regionY mipmapLevel:0 withBytes:videoData->getyuvY() bytesPerRow:_videoWidth];

    MTLRegion regionUV = {{0,0,0}, {(NSUInteger)_videoWidth / 2, (NSUInteger)_videoHeight / 2, 1}};
    [_texU replaceRegion:regionUV mipmapLevel:0 withBytes:videoData->getyuvU() bytesPerRow:_videoWidth / 4];

    [_texV replaceRegion:regionUV mipmapLevel:0 withBytes:videoData->getyuvV() bytesPerRow:_videoWidth / 4];
}

- (void)loadRGBAComponentsWithWidth:(NSInteger)width height:(NSInteger)height {
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRG8Uint
                                                                                      width:(NSInteger)width
                                                                                      height:(NSInteger)height
                                                                                   mipmapped:NO];

    _texY = [_device newTextureWithDescriptor:texDesc];
}

- (void)updateVideoDatatoRGBWithData:(CSJVideoData *)data {

}

- (void)renderRGBAWithEncoder:(id<MTLRenderCommandEncoder>)encoder {
    if (!encoder) {
        return ;
    }

    // Bind rgba pipeline
    [encoder setRenderPipelineState:self.rgbaPipeline];
    
    // Bind vertex buffer
    [encoder setVertexBuffer:self.rgbaVertexBuffer offset:0 atIndex:0];
    
    // Draw primitives
    //[encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

    [encoder setFragmentTexture:self.rgbaTexs[self.currentTexIdx] atIndex:0];

    [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle 
                        indexCount:6
                         indexType:MTLIndexTypeUInt16 
                       indexBuffer:self.rgbaIndexBuffer 
                 indexBufferOffset:0];
}

@end
