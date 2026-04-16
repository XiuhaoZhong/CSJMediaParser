#include <CoreGraphics/CGImage.h>
#import <Metal/Metal.h>
#import <AppKit/AppKit.h>

id<MTLTexture> createTextureFromImageFile(id<MTLDevice> device, NSString *imagePath);

id<MTLTexture> createTextureFromCGImage(id<MTLDevice> device, CGImageRef cgImage);