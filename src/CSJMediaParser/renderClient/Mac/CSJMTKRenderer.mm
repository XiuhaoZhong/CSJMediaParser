#include "CSJMediaEngine/CSJMediaRawData.h"
#include <Metal/Metal.h>

using csjmediaengine::CSJVideoData;
using csjmediaengine::CSJVideoFormatType;

#import "CSJMediaShaderTypes.h"
#import "CSJMTKRenderer.h"

@interface CSJMTKRenderer ()

@property(nonatomic, assign) CGFloat width;
@property(nonatomic, assign) CGFloat height;

@property(nonatomic, assign) NSInteger videoWidth;
@property(nonatomic, assign) NSInteger videoHeight;

@property(nonatomic, strong) MTKView *renderView;
@property(nonatomic, assign) MTLPixelFormat videoPixelFmt;
@property(nonatomic, strong) MTLVertexDescriptor         *vertexDescriptor;
@property(nonatomic, strong) MTLRenderPipelineDescriptor *renderPipeLineDesc;

@property(nonatomic, strong) NSLock *videoDataLock;

@end

@implementation CSJMTKRenderer {
    id<MTLDevice> m_pDevice;
    id<MTLCommandQueue>  m_pCommandQueue;

    id<MTLTexture> m_texY;
    id<MTLTexture> m_texU;
    id<MTLTexture> m_texV;

    CSJVideoFormatType m_videoFmt;
    MTLTextureDescriptor *texDescriptor;

    //id<MTLIOCommandQueue> m_pCommandQueue;
}

- (instancetype)initWithFrame:(CGRect)frame parent:(NSView *)parentView {
    self = [super init];

    _renderView = [[MTKView alloc] initWithFrame:frame];
    _renderView.delegate = self;
    _renderView.wantsLayer = YES;
    _renderView.device = MTLCreateSystemDefaultDevice();
    _renderView.clearColor = MTLClearColorMake(0.0, 1.0, 1.0, 1.0);
    _renderView.preferredFramesPerSecond = 30;
    _renderView.enableSetNeedsDisplay = true;
    _renderView.delegate = self;

    m_pDevice = _renderView.device;
    m_pCommandQueue = [m_pDevice newCommandQueue];
    _renderPipeLineDesc = [[MTLRenderPipelineDescriptor alloc] init];

    _renderPipeLineDesc.colorAttachments[0].pixelFormat = _renderView.colorPixelFormat;

    _width = frame.size.width;
    _height = frame.size.height;

    [parentView addSubview:_renderView];

    //[self createTextureForYUV420:nil width:960 height:640];
    return self;
}

- (void)dealloc {
    NSLog(@"CSJMTKRenderer is dealloc");
#if __has_feature(objc_arc)

#else
    [_renderPipeLineDesc release];
    [_renderView release];

    [m_texY release];
    [m_texU release];
    [m_texV release];


    [super dealloc];
#endif
}

- (void)releaseResources {

}

- (void)drawContent {
    [_renderView draw];
}

// invoke when draw content.
- (void)drawInMTKView:(MTKView *)view {

    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [m_pCommandQueue commandBuffer];
        commandBuffer.label = @"Render Commands";

        MTLRenderPassDescriptor* onScreenDescriptor = view.currentRenderPassDescriptor;
        if (onScreenDescriptor != nil) {
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:onScreenDescriptor];

            //onScreenDescriptor.colorAttachments[0].texture = drawable.texture;
            //onScreenDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            //onScreenDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 1.0, 1.0, 1.0);
            _renderView.clearColor = MTLClearColorMake(0.0, 0.5, 0.5, 1.0);

            MTLViewport viewPort = {0, 0, _width, _height, 0.0, 1.0};
            [renderEncoder setViewport:viewPort];

            [renderEncoder endEncoding];

            if (view.currentDrawable) {
                [commandBuffer presentDrawable:view.currentDrawable];
            }
        }

        [commandBuffer commit];
    }
}

// invoked when view's layout changed, including change resolution, size.
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {

}

- (void)loadMetalWithView:(nonnull MTKView *)view {
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

    _vertexDescriptor = [[MTLVertexDescriptor alloc] init];

    // Position
    _vertexDescriptor.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
    _vertexDescriptor.attributes[VertexAttributePosition].offset = 0;
    _vertexDescriptor.attributes[VertexAttributePosition].bufferIndex = CSJMediaVertexBufferIndex;

    // texCoord
    _vertexDescriptor.attributes[VertexAttributeTexCoord].format = MTLVertexFormatFloat2;
    _vertexDescriptor.attributes[VertexAttributeTexCoord].offset = 12;
    _vertexDescriptor.attributes[VertexAttributeTexCoord].bufferIndex = CSJMediaVertexBufferIndex;
}

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
    m_videoFmt = (CSJVideoFormatType)pixelFmt;
}

- (void)updateVideoFrameWithData:(void *)pData {
    CSJVideoData *videoData = static_cast<CSJVideoData *>(pData);

    if (!videoData) {
        return ;
    }

    [_videoDataLock lock];

    if (videoData->getFmtType() != m_videoFmt ||
        videoData->getWidth() != _videoWidth ||
        videoData->getHeight() != _videoHeight) {

        // reallocate the video buffers.
        // reallocate the textures.
        [self loadVideoComponentWithPixelFmt:videoData->getFmtType()
                                    width:videoData->getWidth()
                                    height:videoData->getHeight()];
    }

    // opy data to buffer
    switch (m_videoFmt) {
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

    m_texY = [m_pDevice newTextureWithDescriptor:texDesc];

    texDesc.width = width / 2;
    texDesc.height = height / 2;

    m_texU = [m_pDevice newTextureWithDescriptor:texDesc];
    m_texV = [m_pDevice newTextureWithDescriptor:texDesc];
}

- (void)updateDatatoYUV420PWidthData:(CSJVideoData *)videoData {
    MTLRegion regionY = {{0,0,0}, {(NSUInteger)_videoWidth, (NSUInteger)_videoHeight, 1}};
    [m_texY replaceRegion:regionY mipmapLevel:0 withBytes:videoData->getyuvY() bytesPerRow:_videoWidth];

    MTLRegion regionUV = {{0,0,0}, {(NSUInteger)_videoWidth / 2, (NSUInteger)_videoHeight / 2, 1}};
    [m_texU replaceRegion:regionUV mipmapLevel:0 withBytes:videoData->getyuvU() bytesPerRow:_videoWidth / 4];

    [m_texV replaceRegion:regionUV mipmapLevel:0 withBytes:videoData->getyuvV() bytesPerRow:_videoWidth / 4];
}

- (void)loadRGBAComponentsWithWidth:(NSInteger)width height:(NSInteger)height {
    MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRG8Uint
                                                                                      width:(NSInteger)width
                                                                                      height:(NSInteger)height
                                                                                   mipmapped:NO];

    m_texY = [m_pDevice newTextureWithDescriptor:texDesc];
}

- (void)updateVideoDatatoRGBWithData:(CSJVideoData *)data {

}

@end
