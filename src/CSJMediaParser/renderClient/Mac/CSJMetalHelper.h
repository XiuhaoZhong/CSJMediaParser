#import <Metal/Metal.h>
#import <AppKit/AppKit.h>

id<MTLTexture> createTextureFromImageFile(id<MTLDevice> device, NSString *imagePath);