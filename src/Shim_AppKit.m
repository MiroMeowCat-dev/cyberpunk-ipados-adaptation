#import <Foundation/Foundation.h>
#import <objc/runtime.h>
static id ShimStubIMP(id self, SEL _cmd, ...) { return nil; }
static id ShimStubInit(id self, SEL _cmd, ...) { return self; }
static BOOL ShimStubResolve(Class c, SEL s, BOOL meta) {
    NSLog(@"SHIM MISSING %c[%@ %@]", meta ? '+' : '-', NSStringFromClass(c), NSStringFromSelector(s));
    BOOL init = !meta && [NSStringFromSelector(s) hasPrefix:@"init"];
    class_addMethod(meta ? object_getClass(c) : c, s, (IMP)(init ? ShimStubInit : ShimStubIMP), "@@:"); return YES; }
__attribute__((visibility("default"))) NSString *shim_d0 __asm__("_NSApplicationDidBecomeActiveNotification") = @"NSApplicationDidBecomeActiveNotification";
__attribute__((visibility("default"))) NSString *shim_d1 __asm__("_NSApplicationDidChangeScreenParametersNotification") = @"NSApplicationDidChangeScreenParametersNotification";
__attribute__((visibility("default"))) NSString *shim_d2 __asm__("_NSApplicationDidResignActiveNotification") = @"NSApplicationDidResignActiveNotification";
__attribute__((visibility("default"))) NSString *shim_d3 __asm__("_NSApplicationWillResignActiveNotification") = @"NSApplicationWillResignActiveNotification";
__attribute__((visibility("default"))) NSString *shim_d4 __asm__("_NSEventTrackingRunLoopMode") = @"NSEventTrackingRunLoopMode";
__attribute__((visibility("default"))) NSString *shim_d5 __asm__("_NSImageHintInterpolation") = @"NSImageHintInterpolation";
__attribute__((visibility("default"))) void *shim_f6(void) __asm__("_NSRectFill");
void *shim_f6(void) { static int n; if (!n++) NSLog(@"SHIM STUB AppKit NSRectFill"); return 0; }
__attribute__((visibility("default"))) NSString *shim_d7 __asm__("_NSWindowDidBecomeKeyNotification") = @"NSWindowDidBecomeKeyNotification";
__attribute__((visibility("default"))) NSString *shim_d8 __asm__("_NSWindowDidResignKeyNotification") = @"NSWindowDidResignKeyNotification";
__attribute__((visibility("default"))) NSString *shim_d9 __asm__("_NSWindowDidResizeNotification") = @"NSWindowDidResizeNotification";
__attribute__((visibility("default"))) NSString *shim_d10 __asm__("_NSWindowWillCloseNotification") = @"NSWindowWillCloseNotification";
__attribute__((visibility("default"))) NSString *shim_d11 __asm__("_NSWindowWillExitFullScreenNotification") = @"NSWindowWillExitFullScreenNotification";
@interface NSEvent : NSObject @end
@interface NSRunningApplication : NSObject @end
@interface NSWorkspace : NSObject @end
@interface NSResponder : NSObject @end
@interface NSView : NSResponder @end
@interface NSViewController : NSResponder @end
@interface NSScreen : NSObject @end
@interface NSWindow : NSResponder @end
@interface NSWindowController : NSResponder @end
@interface NSApplication : NSResponder @end
@interface NSAlert : NSObject @end
__attribute__((visibility("default"))) @interface NSBezierPath : NSObject @end
@implementation NSBezierPath
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSButton : NSObject @end
@implementation NSButton
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSCursor : NSObject @end
@implementation NSCursor
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSImage : NSObject @end
@implementation NSImage
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSMenu : NSObject @end
@implementation NSMenu
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSScrollView : NSObject @end
@implementation NSScrollView
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSScroller : NSObject @end
@implementation NSScroller
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSTextView : NSObject @end
@implementation NSTextView
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
__attribute__((visibility("default"))) @interface NSTrackingArea : NSObject @end
@implementation NSTrackingArea
+ (BOOL)resolveInstanceMethod:(SEL)s { return [super resolveInstanceMethod:s] || ShimStubResolve(self, s, NO); }
+ (BOOL)resolveClassMethod:(SEL)s { return [super resolveClassMethod:s] || ShimStubResolve(self, s, YES); }
@end
