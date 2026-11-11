// CoreGraphics display APIs (macOS-only): one display, the iPad screen, one mode.
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>

typedef uint32_t CGDirectDisplayID;
typedef struct ShimDisplayMode *CGDisplayModeRef;
struct ShimDisplayMode { int index; };
static struct ShimDisplayMode gModes[] = {{0}, {1}, {2}, {3}};
enum { kShimDisplay = 1 };

// Measured once on the main thread at load (game threads must not touch UIKit).
static CGSize gPixels = {2752, 2064};
static double gHz = 120;
__attribute__((constructor)) static void MeasureScreen(void) {
    CGSize s = UIScreen.mainScreen.nativeBounds.size;
    if (s.width > 0) gPixels = s.width >= s.height ? s : CGSizeMake(s.height, s.width);  // games run landscape
    if (UIScreen.mainScreen.maximumFramesPerSecond > 0) gHz = UIScreen.mainScreen.maximumFramesPerSecond;
}

static CGSize ScreenPixels(void) { return gPixels; }

// A virtual 1x game mode sized to the panel aspect ratio. UIKit coordinates
// and physical display queries continue to describe the actual screen.
static CGSize GameModePixels(CGDisplayModeRef mode) {
    if (mode == &gModes[0]) return CGSizeMake(floor(gPixels.width / 2), floor(gPixels.height / 2));
    if (mode == &gModes[1]) return CGSizeMake(floor(gPixels.width * 3 / 4), floor(gPixels.height * 3 / 4));
    if (mode == &gModes[3]) return gPixels;
    CGFloat width = MIN(1920, gPixels.width);
    CGFloat height = 2 * round(width * gPixels.height / gPixels.width / 2);
    return CGSizeMake(width, height);
}


#define EXPORT __attribute__((visibility("default")))

EXPORT int32_t CGGetActiveDisplayList(uint32_t max, CGDirectDisplayID *displays, uint32_t *count) {
    if (displays && max) displays[0] = kShimDisplay;
    if (count) *count = 1;
    return 0;
}
EXPORT int32_t CGGetOnlineDisplayList(uint32_t max, CGDirectDisplayID *displays, uint32_t *count) {
    return CGGetActiveDisplayList(max, displays, count);
}
EXPORT CGDirectDisplayID CGMainDisplayID(void) { return kShimDisplay; }
EXPORT CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID d) { return &gModes[2]; }
EXPORT CFArrayRef CGDisplayCopyAllDisplayModes(CGDirectDisplayID d, CFDictionaryRef options) {
    // The current mode remains 1920x1324. Each enumerated handle describes
    // a distinct rendering size for the game's video menu.
    const void *modes[] = { &gModes[0], &gModes[1], &gModes[2], &gModes[3] };
    return CFArrayCreate(NULL, modes, 4, NULL);
}
EXPORT void CGDisplayModeRelease(CGDisplayModeRef m) {}
EXPORT CGDisplayModeRef CGDisplayModeRetain(CGDisplayModeRef m) { return m; }
EXPORT size_t CGDisplayModeGetWidth(CGDisplayModeRef m) { return GameModePixels(m).width; }   // points
EXPORT size_t CGDisplayModeGetHeight(CGDisplayModeRef m) { return GameModePixels(m).height; }
EXPORT size_t CGDisplayModeGetPixelWidth(CGDisplayModeRef m) { return GameModePixels(m).width; }
EXPORT size_t CGDisplayModeGetPixelHeight(CGDisplayModeRef m) { return GameModePixels(m).height; }
EXPORT double CGDisplayModeGetRefreshRate(CGDisplayModeRef m) { return gHz; }
EXPORT CGRect CGDisplayBounds(CGDirectDisplayID d) { CGSize p = ScreenPixels(); return CGRectMake(0, 0, p.width / 2, p.height / 2); }
EXPORT size_t CGDisplayPixelsWide(CGDirectDisplayID d) { return ScreenPixels().width; }
EXPORT size_t CGDisplayPixelsHigh(CGDirectDisplayID d) { return ScreenPixels().height; }
EXPORT int32_t CGWarpMouseCursorPosition(CGPoint p) { return 0; }
EXPORT int32_t CGAssociateMouseAndMouseCursorPosition(int connected) { return 0; }
EXPORT int CGDisplayIsBuiltin(CGDirectDisplayID d) { return 1; }
EXPORT int CGDisplayIsInMirrorSet(CGDirectDisplayID d) { return 0; }
EXPORT int CGDisplayModeIsUsableForDesktopGUI(CGDisplayModeRef m) { return 1; }
EXPORT uint32_t CGDisplayModeGetIOFlags(CGDisplayModeRef m) { return 0x3; }  // valid + safe
EXPORT uint32_t CGDisplayVendorNumber(CGDirectDisplayID d) { return 0x610; }  // Apple
EXPORT uint32_t CGDisplayModelNumber(CGDirectDisplayID d) { return 1; }
EXPORT uint32_t CGDisplaySerialNumber(CGDirectDisplayID d) { return 1; }
EXPORT CGSize CGDisplayScreenSize(CGDirectDisplayID d) {  // millimetres; iPad Pro/Air panels are 264 ppi
    CGSize p = ScreenPixels(); return CGSizeMake(p.width / 264.0 * 25.4, p.height / 264.0 * 25.4);
}
EXPORT id<MTLDevice> CGDirectDisplayCopyCurrentMetalDevice(CGDirectDisplayID d) NS_RETURNS_RETAINED {
    return MTLCreateSystemDefaultDevice();
}
