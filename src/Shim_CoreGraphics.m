#import <Foundation/Foundation.h>
#import <objc/runtime.h>
static id ShimStubIMP(id self, SEL _cmd, ...) { return nil; }
static id ShimStubInit(id self, SEL _cmd, ...) { return self; }
static BOOL ShimStubResolve(Class c, SEL s, BOOL meta) {
    NSLog(@"SHIM MISSING %c[%@ %@]", meta ? '+' : '-', NSStringFromClass(c), NSStringFromSelector(s));
    BOOL init = !meta && [NSStringFromSelector(s) hasPrefix:@"init"];
    class_addMethod(meta ? object_getClass(c) : c, s, (IMP)(init ? ShimStubInit : ShimStubIMP), "@@:"); return YES; }
__attribute__((visibility("default"))) void *shim_f0(void) __asm__("_CGDisplayIOServicePort");
void *shim_f0(void) { static int n; if (!n++) NSLog(@"SHIM STUB CoreGraphics CGDisplayIOServicePort"); return 0; }
__attribute__((visibility("default"))) void *shim_f1(void) __asm__("_CGEventSourceCreate");
void *shim_f1(void) { static int n; if (!n++) NSLog(@"SHIM STUB CoreGraphics CGEventSourceCreate"); return 0; }
__attribute__((visibility("default"))) void *shim_f2(void) __asm__("_CGEventSourceSetLocalEventsSuppressionInterval");
void *shim_f2(void) { static int n; if (!n++) NSLog(@"SHIM STUB CoreGraphics CGEventSourceSetLocalEventsSuppressionInterval"); return 0; }
__attribute__((visibility("default"))) NSString *shim_d3 __asm__("_kCGDisplayShowDuplicateLowResolutionModes") = @"kCGDisplayShowDuplicateLowResolutionModes";
