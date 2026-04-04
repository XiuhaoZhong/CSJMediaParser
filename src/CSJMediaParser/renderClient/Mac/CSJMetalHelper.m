#import "CSJMetalHelper.h"
#include <Metal/Metal.h>

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

    NSUInteger width = CGImageGetWidth(cgImage);
    NSUInteger height = CGImageGetHeight(cgImage);

    MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    desc.width = width;
    desc.height = height;
    desc.usage = MTLTextureUsageShaderRead;

    id<MTLTexture> texture = [device newTextureWithDescriptor:desc];

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *imageData = malloc(width * height * 4);

    CGContextRef context = CGBitmapContextCreate(imageData, width, height, 8, width * 4, 
                                                 colorSpace, 
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);

    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [texture replaceRegion:region mipmapLevel:0 withBytes:imageData bytesPerRow:width * 4];

    free(imageData);
    return texture;
}