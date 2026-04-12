#import "CSJMetalHelper.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

id<MTLTexture> createTextureFromImageFile(id<MTLDevice> device, NSString *imagePath) {
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:imagePath];
    if (!image) {
        return nil;
    }

    CGImageRef cgImage = [image CGImageForProposedRect:nil context:nil hints:nil];

    return createTextureFromCGImage(device, cgImage);
}

id<MTLTexture> createTextureFromCGImage(id<MTLDevice> device, CGImageRef cgImage) {
    if (!device || !cgImage) {
        return nil;
    }

    MTKTextureLoader *textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
    NSDictionary *options = @{MTKTextureLoaderOptionSRGB: @NO,
                              MTKTextureLoaderOptionAllocateMipmaps: @NO};

    NSError *error = nil;
    id<MTLTexture> texture = [textureLoader newTextureWithCGImage:cgImage options:options error:&error];
    if (error) {
        return nil;
    }

    return texture;
}