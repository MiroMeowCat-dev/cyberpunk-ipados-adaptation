// Full touch gamepad for this Mac->iPad compatibility layer.
// Uses the public writable GCController snapshot API. The adapter enumerates the
// snapshot and delivers its handlers exactly once; physical controllers delegate
// to Apple's unchanged implementations. No game or service code is patched.
#import <UIKit/UIKit.h>
#import <GameController/GameController.h>
#import <objc/runtime.h>
#import "PortPadMath.h"
#import "PortInputSources.h"
extern void PortCancelShortcutKeys(void);

extern void PortPostShortcutKey(unsigned slot,unsigned short code,BOOL held,NSString *characters);







static GCController *gTouchController;
static NSSet *gOwnedElements;
static BOOL gPublished;

static UIWindow *gTouchWindow;


static unsigned gLoggedInputs, gLoggedHandlers;
static BOOL gPhysicalMode;
static NSMutableDictionary<NSValue *,NSNumber *> *gButtonSources;

typedef struct { Class cls; SEL sel; IMP original; } PortHandlerHook;
static PortHandlerHook gHandlerHooks[16];
static int gHandlerHookCount;
static IMP gControllersOriginal, gCurrentOriginal;

static BOOL PortOwns(id object) { return gOwnedElements && [gOwnedElements containsObject:object]; }
static IMP PortOriginalHandler(id object, SEL selector) {
    for (int i=0;i<gHandlerHookCount;i++)
        if (gHandlerHooks[i].sel==selector && [object isKindOfClass:gHandlerHooks[i].cls])
            return gHandlerHooks[i].original;
    return NULL;
}
static void PortSetHandler(id object, SEL selector, id handler) {
    if (PortOwns(object)) {
        objc_setAssociatedObject(object,(const void *)selector,handler,OBJC_ASSOCIATION_COPY);
        if (handler && gLoggedHandlers++<8) NSLog(@"PORTPAD handler registered %@",NSStringFromSelector(selector));
        return;
    }
    IMP original=PortOriginalHandler(object,selector);
    if (original) ((void(*)(id,SEL,id))original)(object,selector,handler);
}
static void PortHookHandler(Class cls,SEL sel,IMP implementation) {
    Method method=class_getInstanceMethod(cls,sel);
    if (!method || gHandlerHookCount>=16) return;
    // Avoid replacing an inherited method on an unrelated base class.
    IMP original=method_getImplementation(method);
    class_replaceMethod(cls,sel,implementation,method_getTypeEncoding(method));
    gHandlerHooks[gHandlerHookCount++]=(PortHandlerHook){cls,sel,original};
}
static NSArray *PortControllers(id cls,SEL selector) {
    NSArray *real=((id(*)(id,SEL))gControllersOriginal)(cls,selector);
    if(gTouchController && [real containsObject:gTouchController]) {
        NSMutableArray *filtered=[real mutableCopy];[filtered removeObjectIdenticalTo:gTouchController];real=[filtered copy];
    }
    if (!gPublished || !gTouchController) return real;
    for(GCController *controller in real)if(controller!=gTouchController && !controller.isSnapshot && controller.extendedGamepad)return real;
    return [(real ?: @[]) arrayByAddingObject:gTouchController];
}
static GCController *PortCurrent(id cls,SEL selector) {
    GCController *real=gCurrentOriginal ? ((id(*)(id,SEL))gCurrentOriginal)(cls,selector) : nil;
    if(real && real!=gTouchController)return real;
    NSArray *physical=gControllersOriginal?((id(*)(id,SEL))gControllersOriginal)(cls,@selector(controllers)):@[];
    for(GCController *controller in physical)if(controller!=gTouchController && !controller.isSnapshot && controller.extendedGamepad)return controller;
    return gPublished ? gTouchController : nil;
}
static id PortHandler(id object,SEL setter) { return objc_getAssociatedObject(object,(const void *)setter); }
static void PortDeliver(dispatch_block_t delivery) {
    dispatch_queue_t queue=gTouchController.handlerQueue ?: dispatch_get_main_queue();
    if (queue==dispatch_get_main_queue() && NSThread.isMainThread) delivery();
    else dispatch_async(queue,delivery);
}
static void PortNotifyButton(GCControllerButtonInput *button,float old) {
    if (!button || fabsf(old-button.value)<0.00001f) return;
    GCControllerButtonValueChangedHandler changed=PortHandler(button,@selector(setValueChangedHandler:));
    GCControllerButtonValueChangedHandler pressed=PortHandler(button,@selector(setPressedChangedHandler:));
    float value=button.value;BOOL isPressed=button.isPressed;
    if (changed) PortDeliver(^{changed(button,value,isPressed);});
    if (pressed && (old>0.5f)!=isPressed) PortDeliver(^{pressed(button,value,isPressed);});
}
static void PortNotifyGamepad(GCControllerElement *element) {
    GCExtendedGamepad *pad=gTouchController.extendedGamepad;
    GCExtendedGamepadValueChangedHandler handler=PortHandler(pad,@selector(setValueChangedHandler:));
    if (handler) PortDeliver(^{handler(pad,element);});
}
static void PortSetButton(GCControllerButtonInput *button,BOOL held) {
    if (!button) return;
    float old=button.value, value=held?1:0;
    if (fabsf(old-value)<0.00001f) return;
    [button setValue:value];
    PortNotifyButton(button,old);PortNotifyGamepad(button);
    if (gLoggedInputs++<8) NSLog(@"PORTPAD button value=%.0f handler=%d",value,PortHandler(button,@selector(setValueChangedHandler:))!=nil);
}
static void PortSetButtonSource(GCControllerButtonInput *button,BOOL held,unsigned source) {
    if(!button)return;if(!gButtonSources)gButtonSources=[NSMutableDictionary new];
    NSValue *key=[NSValue valueWithNonretainedObject:button];
    uint32_t old=[gButtonSources[key] unsignedIntValue],next=PortInputSources(old,source,held);
    gButtonSources[key]=@(next);if((old!=0)!=(next!=0))PortSetButton(button,next!=0);
}
static NSArray<NSDictionary *> *PortShortcutChoices(void) {
    static NSArray *choices;static dispatch_once_t once;
    dispatch_once(&once, ^{choices=@[
        @{@"id":@"esc",@"label":@"Esc",@"kind":@"key",@"code":@53,@"chars":@"\e"},
        @{@"id":@"i",@"label":@"I",@"kind":@"key",@"code":@34,@"chars":@"i"},
        @{@"id":@"m",@"label":@"M",@"kind":@"key",@"code":@46,@"chars":@"m"},
        @{@"id":@"j",@"label":@"J",@"kind":@"key",@"code":@38,@"chars":@"j"},
        @{@"id":@"f",@"label":@"F",@"kind":@"key",@"code":@3,@"chars":@"f"},
        @{@"id":@"e",@"label":@"E",@"kind":@"key",@"code":@14,@"chars":@"e"},
        @{@"id":@"q",@"label":@"Q",@"kind":@"key",@"code":@12,@"chars":@"q"},
        @{@"id":@"tab",@"label":@"Tab",@"kind":@"key",@"code":@48,@"chars":@"\t"},
        @{@"id":@"enter",@"label":@"Enter",@"kind":@"key",@"code":@36,@"chars":@"\r"},
        @{@"id":@"space",@"label":@"空格",@"kind":@"key",@"code":@49,@"chars":@" "},
        @{@"id":@"shift",@"label":@"Shift",@"kind":@"key",@"code":@56,@"chars":@""},
        @{@"id":@"padA",@"label":@"手柄 A",@"kind":@"pad",@"property":@"buttonA"},
        @{@"id":@"padB",@"label":@"手柄 B",@"kind":@"pad",@"property":@"buttonB"},
        @{@"id":@"padX",@"label":@"手柄 X",@"kind":@"pad",@"property":@"buttonX"},
        @{@"id":@"padY",@"label":@"手柄 Y",@"kind":@"pad",@"property":@"buttonY"},
        @{@"id":@"padLT",@"label":@"手柄 LT",@"kind":@"pad",@"property":@"leftTrigger"},
        @{@"id":@"padRT",@"label":@"手柄 RT",@"kind":@"pad",@"property":@"rightTrigger"},
        @{@"id":@"padLB",@"label":@"手柄 LB",@"kind":@"pad",@"property":@"leftShoulder"},
        @{@"id":@"padRB",@"label":@"手柄 RB",@"kind":@"pad",@"property":@"rightShoulder"},
        @{@"id":@"padL3",@"label":@"手柄 L3",@"kind":@"pad",@"property":@"leftThumbstickButton"},
        @{@"id":@"padR3",@"label":@"手柄 R3",@"kind":@"pad",@"property":@"rightThumbstickButton"},
        @{@"id":@"padMenu",@"label":@"手柄菜单",@"kind":@"pad",@"property":@"buttonMenu"},
        @{@"id":@"padView",@"label":@"手柄视图",@"kind":@"pad",@"property":@"buttonOptions"}
    ];});return choices;
}
static NSDictionary *PortShortcut(unsigned slot) {
    NSString *fallback=slot?@"esc":@"space";
    NSString *identifier=[NSUserDefaults.standardUserDefaults stringForKey:slot?@"PortPadShortcut2":@"PortPadShortcut1"]?:fallback;
    for(NSDictionary *choice in PortShortcutChoices())if([choice[@"id"] isEqual:identifier])return choice;
    for(NSDictionary *choice in PortShortcutChoices())if([choice[@"id"] isEqual:fallback])return choice;
    return PortShortcutChoices().firstObject;
}
static void PortRunShortcut(unsigned slot,BOOL held) {
    NSDictionary *binding=PortShortcut(slot);NSString *kind=binding[@"kind"];
    if([kind isEqual:@"key"])PortPostShortcutKey(slot,[binding[@"code"] unsignedShortValue],held,binding[@"chars"]);
    else if([kind isEqual:@"pad"])PortSetButtonSource([gTouchController.extendedGamepad valueForKey:binding[@"property"]],held,20+slot);
}
static void PortSetPad(GCControllerDirectionPad *pad,float x,float y) {
    if (!pad) return;
    float oldX=pad.xAxis.value,oldY=pad.yAxis.value;
    if (fabsf(oldX-x)<0.00001f && fabsf(oldY-y)<0.00001f) return;
    float oldButtons[]={pad.left.value,pad.right.value,pad.up.value,pad.down.value};
    [pad setValueForXAxis:x yAxis:y];
    GCControllerAxisValueChangedHandler hx=PortHandler(pad.xAxis,@selector(setValueChangedHandler:));
    GCControllerAxisValueChangedHandler hy=PortHandler(pad.yAxis,@selector(setValueChangedHandler:));
    float valueX=pad.xAxis.value,valueY=pad.yAxis.value;
    if (hx && fabsf(oldX-x)>0.00001f) PortDeliver(^{hx(pad.xAxis,valueX);});
    if (hy && fabsf(oldY-y)>0.00001f) PortDeliver(^{hy(pad.yAxis,valueY);});
    PortNotifyButton(pad.left,oldButtons[0]);PortNotifyButton(pad.right,oldButtons[1]);
    PortNotifyButton(pad.up,oldButtons[2]);PortNotifyButton(pad.down,oldButtons[3]);
    GCControllerDirectionPadValueChangedHandler handler=PortHandler(pad,@selector(setValueChangedHandler:));
    if (handler) PortDeliver(^{handler(pad,valueX,valueY);});
    PortNotifyGamepad(pad);
}
static void PortCreateController(void) {
    if (gTouchController) return;
    gTouchController=[GCController controllerWithExtendedGamepad];
    gTouchController.playerIndex=GCControllerPlayerIndex1;
    GCExtendedGamepad *pad=gTouchController.extendedGamepad;
    if (!pad || !gTouchController.isSnapshot) {NSLog(@"PORTPAD snapshot creation failed");gTouchController=nil;return;}
    NSMutableArray *buttons=[NSMutableArray new];
    for (NSString *name in @[@"buttonA",@"buttonB",@"buttonX",@"buttonY",@"leftShoulder",@"rightShoulder",@"leftTrigger",@"rightTrigger",@"buttonMenu",@"buttonOptions",@"leftThumbstickButton",@"rightThumbstickButton"]) {
        id button=[pad valueForKey:name];if (button) [buttons addObject:button];
    }

    NSMutableSet *owned=[NSMutableSet setWithArray:buttons];[owned addObject:pad];
    for (GCControllerDirectionPad *direction in @[pad.leftThumbstick,pad.rightThumbstick,pad.dpad])
        [owned addObjectsFromArray:@[direction,direction.xAxis,direction.yAxis,direction.up,direction.down,direction.left,direction.right]];
    gOwnedElements=[owned copy];
    PortHookHandler(GCControllerButtonInput.class,@selector(setValueChangedHandler:),(IMP)PortSetHandler);
    PortHookHandler(GCControllerButtonInput.class,@selector(setPressedChangedHandler:),(IMP)PortSetHandler);
    for (Class cls in @[GCControllerAxisInput.class,GCControllerDirectionPad.class,GCExtendedGamepad.class]) {
        PortHookHandler(cls,@selector(setValueChangedHandler:),(IMP)PortSetHandler);
    }
    Method list=class_getClassMethod(GCController.class,@selector(controllers));
    gControllersOriginal=method_setImplementation(list,(IMP)PortControllers);
    Method current=class_getClassMethod(GCController.class,@selector(current));
    if (current) gCurrentOriginal=method_setImplementation(current,(IMP)PortCurrent);
    gPublished=YES;
    NSLog(@"PORTPAD ready buttons=%lu menu=%d options=%d L3=%d R3=%d",(unsigned long)buttons.count,
        pad.buttonMenu!=nil,pad.buttonOptions!=nil,pad.leftThumbstickButton!=nil,pad.rightThumbstickButton!=nil);
}


#include "PortGyroMath.h"
static BOOL gPortRightTouchActive,gPortGyroAllowed;
static PortPadVector gPortGyroAxis;
static BOOL PortGyroEnabled(void);
static int PortGyroMode(void);
static double PortGyroGain(void);
static double PortGyroAxisGain(BOOL vertical);
static NSString *PortGyroStatus(void);
static void PortGyroRecenter(void);
static void PortGyroSuspend(void);
static void PortGyroTick(void);
static BOOL gPortControlUIDirty;
#include "PortZeldaCanvas.inc"
#include "PortZeldaLayout.inc"
#include "PortZeldaWindow.inc"
#include "PortGyroInput.inc"

__attribute__((visibility("default"))) void PortCommitTouchUI(void) {
    if(!NSThread.isMainThread)return;
    PortGyroTick();
    if(!gPortControlUIDirty)return;
    gPortControlUIDirty=NO;
    [gTouchWindow layoutIfNeeded];
    [gZeldaSurface.layer displayIfNeeded];
    [CATransaction flush];
}
