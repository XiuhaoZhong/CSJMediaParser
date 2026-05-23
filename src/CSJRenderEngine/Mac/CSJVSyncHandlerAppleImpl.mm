#include "CSJVSyncHandlerAppleImpl.h"

#include "CSJUtils/CSJLogger.h" 

#import <AppKit/AppKit.h>
#import <QuartzCore/CADisplayLink.h>
//#import <Foundation/Foundation.h>

@interface WeakProxy : NSProxy
@property (nonatomic, weak, readonly) id target;
+ (instancetype)proxyWithTarget:(id)target;
@end

@implementation WeakProxy
- (instancetype)initWithTarget:(id)target {
    _target = target;
    return self;
}
+ (instancetype)proxyWithTarget:(id)target {
    return [[WeakProxy alloc] initWithTarget:target];
}

- (id)originalTarget {
    return self.target;
}

// 关键方法1:返回方法签名
- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel {
    // 如果 target 还存在，返回它的方法签名
    if (self.target) {
        return [self.target methodSignatureForSelector:sel];
    }
    // target 已释放，返回一个空的方法签名(避免 crash)
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];
}

// 关键方法2:转发消息
- (void)forwardInvocation:(NSInvocation *)invocation {
    if (self.target) {
        [invocation invokeWithTarget:self.target];
    }
    // 如果 target 已释放，什么都不做
}

@end

@interface CSJVSyncHelper : NSObject 

@property (nonatomic, assign) BOOL           isRendering;
@property (nonatomic, strong) NSThread      *renderThread;
@property (nonatomic, strong) CADisplayLink *displayLink;

- (void)vsyncTick:(CADisplayLink *)link;

- (void)invalidateLink;

- (void)setSyncImpl:(nullable void *)syncImpl;
@end

@implementation CSJVSyncHelper {
    csjrenderengine::CSJVSyncHandlerAppleImpl *_syncImpl;
}

- (void)setupRenderThread {
    self.isRendering = YES;

    WeakProxy *selfProxy = [WeakProxy proxyWithTarget:self];
    self.renderThread = [[NSThread alloc] initWithBlock:^{
        CSJVSyncHelper *helper = [selfProxy originalTarget];
        if (!helper) {
            return ;
        }
        NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
        [runLoop addPort:[NSPort port] forMode:NSDefaultRunLoopMode];

        while (helper.isRendering && [runLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]]);
    }];

    self.renderThread.name = @"CSJ.Rendering.Thread";
    [self.renderThread start];
}

- (void)setupDisplayLink:(NSView *)currentView {

    [self setupRenderThread];

    WeakProxy *selfProxy = [WeakProxy proxyWithTarget:self];
    dispatch_async(dispatch_get_main_queue(), ^{
        BOOL setupSuccess = YES;
        CADisplayLink *link = nil;
        CSJVSyncHelper *helper = [selfProxy originalTarget];
        do {
            if (!helper) {
                LOG_Error("Vsync helper is null, failed to setup display link!");
                setupSuccess = NO;
                break ;
            }

            if (currentView) {
                link = [currentView displayLinkWithTarget:helper selector:@selector(vsyncTick:)];
            } else {
                NSScreen *screen = [NSScreen mainScreen];
                link = [screen displayLinkWithTarget:helper selector:@selector(vsyncTick:)];
            }

            if (!link) {
                LOG_Error("Create displaylink with target failed!");
                setupSuccess = NO;
                break ;
            }
        } while (0);

        if (!setupSuccess || !link) {
            [helper stopDisplayLink];
            return ;
        }

        // key: using NSRunLoopCommonModes, which can be compatible with Qt event loop.
        [link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
        helper.displayLink = link;
    });
}

- (void)stopDisplayLink {
    if (!self.isRendering) {
        return ;
    }

    [self invalidateLink];

    self.isRendering = NO;
    [self.renderThread cancel];
    self.renderThread = nil;
}

- (void)vsyncTick:(CADisplayLink *)link {
    CFTimeInterval timeStamp = link.timestamp;

    [self performSelector:@selector(renderFunc:)
                 onThread:self.renderThread
               withObject:@(timeStamp)
            waitUntilDone:NO];
}

- (void)renderFunc:(NSNumber *)timeStampNumber {
    if (!_syncImpl) {
        return ;
    }

    CFTimeInterval timeStamp = timeStampNumber.doubleValue;

    csjrenderengine::CSJVSyncHandlerAppleImpl *impl = static_cast<csjrenderengine::CSJVSyncHandlerAppleImpl *>(_syncImpl);
    impl->display(timeStamp);
}

- (void)setSyncImpl:(void *)syncImpl {
    _syncImpl = static_cast<csjrenderengine::CSJVSyncHandlerAppleImpl *>(syncImpl);
}

- (void)invalidateLink {
    if (_displayLink) {
        [_displayLink invalidate];
        _displayLink = nil;
    }
}

@end

namespace csjrenderengine {

CSJVSyncHandlerAppleImpl::~CSJVSyncHandlerAppleImpl() {
    stop();
    m_pVsyncHelper = nullptr;
    m_callback = nullptr;
}

void CSJVSyncHandlerAppleImpl::start(void* currentView, CSJVSyncCallback callback) {
    if (!currentView) {
        LOG_Error("Target nsview is null, failed to start rendering!");
        return ;
    }

    m_callback = callback;

    NSView * renderView = (__bridge NSView *)currentView;
    CSJVSyncHelper *vsyncHelper = [[CSJVSyncHelper alloc] init];

    [vsyncHelper setSyncImpl:(void *)this];
    [vsyncHelper setupDisplayLink:renderView];

    m_pVsyncHelper = (__bridge_retained void *)vsyncHelper;

    LOG_Info("VSync displaylink started!");
}

void CSJVSyncHandlerAppleImpl::stop() {
    if (!m_pVsyncHelper) {
        return ;
    }

    [(__bridge id)m_pVsyncHelper stopDisplayLink];

    id obj = (__bridge_transfer id)m_pVsyncHelper;
    // obj will be released after leaving the scope.
    (void)obj;  // Eliminating the unuse warning.

    m_pVsyncHelper = nullptr;
    m_callback = nullptr;
}

void CSJVSyncHandlerAppleImpl::display(double timeStamp) {
    if (!m_callback) {
        LOG_Error("Rendering callback is null, can't execute rendering function!");
        return ;
    }

    m_callback(timeStamp);
}

} // namespace csjrenderengine