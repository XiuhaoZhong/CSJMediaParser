#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>
#import <Foundation/Foundation.h>

@interface CSJMTKRenderer : NSObject <MTKViewDelegate>

- (instancetype)initWithFrame:(CGRect)frame parent:(NSView *)parentView;

- (void)loadVideoComponentWithPixelFmt:(NSInteger)fmtType width:(NSInteger)width height:(NSInteger)height;

- (void)updateVideoFrameWithData:(void *)pData;

- (void)drawContent;

@end
