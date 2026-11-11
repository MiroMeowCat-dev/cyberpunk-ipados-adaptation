// Compatibility for the reviewed Cyberpunk 2.5 HDR gate only.
// The build script pins the executable hash and the private query return offset.
static IMP gPortHDRSupportsFeatureSet;
static BOOL gPortHDRModernGPU;
static BOOL gPortHDRHalfFloatTarget;

static BOOL PortHDRSupportsFeatureSet(id self, SEL selector, NSUInteger featureSet) {
    BOOL native = ((BOOL (*)(id, SEL, NSUInteger))gPortHDRSupportsFeatureSet)(
        self, selector, featureSet);
    if (featureSet != 10001) return native;

    void *caller = __builtin_return_address(0);
    Dl_info image;
    BOOL scoped = NO;
    if (dladdr(caller, &image) && image.dli_fbase && image.dli_fname) {
        const char *leaf = strrchr(image.dli_fname, '/');
        leaf = leaf ? leaf + 1 : image.dli_fname;
        uintptr_t offset = (uintptr_t)caller - (uintptr_t)image.dli_fbase;
        NSNumber *expected = NSBundle.mainBundle.infoDictionary[@"PortHDRLegacyQueryReturnOffset"];
        scoped = expected && strcmp(leaf, "Cyberpunk2077") == 0
            && offset == expected.unsignedLongLongValue;
    }
    BOOL compatible = scoped && gPortHDRModernGPU && gPortHDRHalfFloatTarget;
    static unsigned reports;
    if (reports++ < 8)
        NSLog(@"PORTHDR legacy feature=%lu native=%d scoped=%d modern=%d half-float=%d result=%d",
              (unsigned long)featureSet, native, scoped, gPortHDRModernGPU,
              gPortHDRHalfFloatTarget, native || compatible);
    return native || compatible;
}

static void PortInstallHDRLegacyGate(id<MTLDevice> device, Class deviceClass) {
    if (![NSBundle.mainBundle.infoDictionary[@"PortHDRLegacyQueryReturnOffset"] isKindOfClass:NSNumber.class])
        return;
    // Use the saved native family implementation, avoiding the existing Mac2 adapter.
    BOOL metal4 = ((BOOL (*)(id, SEL, MTLGPUFamily))gSupportsFamily)(
        device, @selector(supportsFamily:), MTLGPUFamilyMetal4);
    BOOL apple9 = ((BOOL (*)(id, SEL, MTLGPUFamily))gSupportsFamily)(
        device, @selector(supportsFamily:), MTLGPUFamilyApple9);
    gPortHDRModernGPU = metal4 && apple9;
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA16Float
        width:2 height:2 mipmapped:NO];
    descriptor.usage = MTLTextureUsageRenderTarget;
    descriptor.storageMode = MTLStorageModePrivate;
    id<MTLTexture> probe = [device newTextureWithDescriptor:descriptor];
    gPortHDRHalfFloatTarget = probe != nil;
    Method method = class_getInstanceMethod(deviceClass, @selector(supportsFeatureSet:));
    if (method)
        gPortHDRSupportsFeatureSet = method_setImplementation(method, (IMP)PortHDRSupportsFeatureSet);
    NSLog(@"PORTHDR legacy gate installed=%d modern=%d half-float=%d",
          gPortHDRSupportsFeatureSet != NULL, gPortHDRModernGPU, gPortHDRHalfFloatTarget);
}
