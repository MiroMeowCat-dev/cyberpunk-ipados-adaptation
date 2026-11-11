static __weak PortZeldaTouchPad *gZeldaSurface;
static void PortRefreshZeldaPhysical(void) {
    NSArray *real=gControllersOriginal?((id(*)(id,SEL))gControllersOriginal)(GCController.class,@selector(controllers)):@[];
    BOOL physical=NO;
    for(GCController *c in real)if(c!=gTouchController&&!c.isSnapshot&&c.extendedGamepad){physical=YES;break;}
    if(gPhysicalMode==physical && gPublished==!physical)return;
    if(physical)PortGyroSuspend();
    BOOL wasPublished=gPublished;gPhysicalMode=physical;gPublished=!physical;
    if(gZeldaSurface) {
        [gZeldaSurface setPhysicalMode:physical];
        if(physical && state(gZeldaSurface).editor)[state(gZeldaSurface) finish:NO];
        [gZeldaSurface setNeedsDisplay];
    }
    if(wasPublished!=gPublished)[NSNotificationCenter.defaultCenter postNotificationName:gPublished?GCControllerDidConnectNotification:GCControllerDidDisconnectNotification object:gTouchController];
}
@interface PortZeldaViewController : UIViewController @end
@implementation PortZeldaViewController
- (void)loadView {self.view=[PortZeldaTouchPad new];}
- (BOOL)prefersStatusBarHidden {return YES;}
- (BOOL)prefersHomeIndicatorAutoHidden {return YES;}
- (UIInterfaceOrientationMask)supportedInterfaceOrientations {return UIInterfaceOrientationMaskLandscape;}
@end
@interface PortZeldaWindow : UIWindow @end
@implementation PortZeldaWindow
- (UIView *)hitTest:(CGPoint)p withEvent:(UIEvent *)event {
    UIView *hit=[super hitTest:p withEvent:event];
    if(hit==self)return nil;
    if(hit==self.rootViewController.view && (gPhysicalMode||!controlsVisible()))return nil;
    return hit;
}
@end
__attribute__((visibility("default"))) void PortInstallTouchGamepad(UIWindowScene *scene) {
    if(gTouchWindow||![NSBundle.mainBundle.infoDictionary[@"PortTouchGamepadEnabled"] boolValue])return;
    PortCreateController();if(!gTouchController)return;
    gTouchWindow=[[PortZeldaWindow alloc] initWithWindowScene:scene];gTouchWindow.windowLevel=UIWindowLevelNormal+10;
    gTouchWindow.backgroundColor=UIColor.clearColor;gTouchWindow.rootViewController=[PortZeldaViewController new];
    gZeldaSurface=(PortZeldaTouchPad *)gTouchWindow.rootViewController.view;
    gTouchWindow.hidden=NO;[gZeldaSurface setNeedsLayout];PortRefreshZeldaPhysical();
    [NSNotificationCenter.defaultCenter addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *n){
        PortGyroSuspend();[gZeldaSurface releaseAllForReason:@"app-inactive"];if(state(gZeldaSurface).editor)[state(gZeldaSurface) finish:NO];
    }];
    for(NSString *name in @[GCControllerDidConnectNotification,GCControllerDidDisconnectNotification])
        [NSNotificationCenter.defaultCenter addObserverForName:name object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *n){
            if(n.object!=gTouchController)PortRefreshZeldaPhysical();
        }];
    dispatch_async(dispatch_get_main_queue(),^{
        if(gPublished)[NSNotificationCenter.defaultCenter postNotificationName:GCControllerDidConnectNotification object:gTouchController];
        NSLog(@"PORTPAD unified Zelda canvas installed; touch mouse removed; controls=18 editor=ready");
    });
}
