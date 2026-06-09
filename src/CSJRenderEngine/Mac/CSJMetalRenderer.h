
using csjutils::CSJVideoFramePtr;
using csjutils::CSJPixelFormat;

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>
#import <Foundation/Foundation.h>

@interface CSJMetalRenderer : NSObject

- (instancetype)initWithFrameWithView:(NSView *)view 
                                frame:(CGRect)frame 
                           pixelRatio:(float)pixelRatio;

- (void)loadVideoComponentWithPixelFmt:(CSJPixelFormat)fmtType width:(NSInteger)width height:(NSInteger)height;

- (void)updateVideoFrameWithData:(CSJVideoFramePtr)videoFrame;

- (void)updateDrawableSizeWithWidth:(NSInteger)width 
                             height:(NSInteger)height 
                         pixelRatio:(float)pixelRatio;

- (void)updateDrawable:(NSInteger)width height:(NSInteger)height pixelRatio:(float)pixelRatio;

- (void)setImageWithPath:(NSString *)imagePath;

// if updateContent is NO, only render background.
- (void)drawContent:(BOOL)updateContent;

- (BOOL)updateSceneWithTimeStamp:(float)timeStamp;

@end
