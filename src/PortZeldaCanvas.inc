// Adapted from the pinned MIT TOTKTouchPad. Full layout editor is included below.
#import "botw_touch_button.h"
#include "PortZeldaInputState.h"
#include "PortTouchTrace.inc"
static const CGFloat kButtonSize=46;
enum {kButtonCount=18};
static uint64_t gZeldaLastBits;

static void PortPublishZelda(PortZeldaInputState s) {
    GCExtendedGamepad *p=gTouchController.extendedGamepad;
    NSArray *names=@[@"buttonA",@"buttonB",@"buttonX",@"buttonY",@"leftThumbstickButton",@"rightThumbstickButton",
        @"leftShoulder",@"rightShoulder",@"leftTrigger",@"rightTrigger",@"buttonMenu",@"buttonOptions"];
    for(unsigned i=0;i<names.count;i++)PortSetButtonSource([p valueForKey:names[i]],(s.buttons&(1ULL<<i))!=0,i);
    PortSetPad(p.dpad,((s.buttons&16384)?1:0)-((s.buttons&4096)?1:0),
        ((s.buttons&8192)?1:0)-((s.buttons&32768)?1:0));
    PortSetPad(p.leftThumbstick,s.left.x,s.left.y);PortPadVector right=port_gyro_merge(s.right,gPortRightTouchActive,gPortGyroAxis,gPortGyroAllowed);
    PortSetPad(p.rightThumbstick,right.x,right.y);
    for(unsigned slot=0;slot<2;slot++) {
        uint64_t bit=1ULL<<(16+slot);
        if((gZeldaLastBits&bit)!=(s.buttons&bit))PortRunShortcut(slot,(s.buttons&bit)!=0);
    }
    gZeldaLastBits=s.buttons;
}
@interface PortZeldaTouchPad : UIView {
    NSMapTable<UITouch *,NSValue *> *points,*leftOrigins,*rightOrigins;
    BOOL concealed;
    CGSize _lastBoundsSize;
}
- (void)buttons:(TouchButton *)b;
- (CGPoint)restingStick:(BOOL)right;
- (CGFloat)stickRadius;
- (void)updateState;
- (void)releaseAll;
- (void)releaseAllForReason:(NSString *)reason;
- (void)setPhysicalMode:(BOOL)physical;
@end
@implementation PortZeldaTouchPad
- (instancetype)initWithFrame:(CGRect)frame {
    if((self=[super initWithFrame:frame])) {
        self.multipleTouchEnabled=YES;self.backgroundColor=UIColor.clearColor;
        self.contentMode=UIViewContentModeRedraw;
        points=[NSMapTable strongToStrongObjectsMapTable];
        leftOrigins=[NSMapTable strongToStrongObjectsMapTable];rightOrigins=[NSMapTable strongToStrongObjectsMapTable];
    }return self;
}
- (void)setNeedsDisplay {[super setNeedsDisplay];gPortControlUIDirty=YES;}
- (CGRect)area { return UIEdgeInsetsInsetRect(self.bounds, self.safeAreaInsets); }
- (CGFloat)stickRadius { return MIN(70, self.area.size.height * .16); }
/* Distance between the centre of a button diamond (A/B/X/Y, the d-pad) and each of its buttons. */
- (CGFloat)diamond { return MIN(52, self.area.size.height * .14); }
- (CGFloat)hitRadius { return MIN(36, self.diamond * .68); }
/* How far the two columns sit inside the safe area: 14% of its width. On an iPhone they move halfway from there toward
   the screen edge, so the gap between the screen edge and the d-pad or A/B/X/Y is half as wide (the user's choice,
   2026-10-19; about 38 points on an iPhone 14, clear of the notch); the stick rings stay inside the safe area. */
- (CGFloat)columnInset {
    CGFloat full = self.area.size.width * .14;
    /* On an iPad both columns, and the sticks with them (restingStick), sit one stick radius further out (the user,
       2026-11-03: "往两边 靠一点 一个半径 距离"; 165 -> 95 points on an 11-inch iPad), keeping the outer button clear
       of the edge on a smaller one. */
    if (self.traitCollection.userInterfaceIdiom != UIUserInterfaceIdiomPhone)
        return MAX(full - self.stickRadius, self.diamond + kButtonSize / 2 + 12);
    CGFloat side = MAX(self.safeAreaInsets.left, self.safeAreaInsets.right), edgeGap = side + full - (self.diamond + kButtonSize / 2);
    return MAX(full - edgeGap / 2, self.stickRadius + 4);
}
/* The shoulder buttons move outward the same way on an iPhone: the gap between the screen edge and ZL / ZR is halved,
   and L / R move with them. */
- (CGFloat)shoulderShift {
    if (self.traitCollection.userInterfaceIdiom != UIUserInterfaceIdiomPhone) return 0;
    CGFloat side = MAX(self.safeAreaInsets.left, self.safeAreaInsets.right);
    return (side + self.area.size.width * .05 - kButtonSize / 2) / 2;
}
/* The top row (ZL L − frame-rate + R ZR): on an iPad the buttons' top edge one button (46 points) below the screen's
   top edge (the user, 2026-11-03: "最上面 所有按钮 往上", "46点"; their centre 96 -> 69 points on an 11-inch iPad),
   never above the safe area; on an iPhone 12% down the safe area, as before. */
- (CGFloat)topRow {
    CGRect a = self.area;
    if (self.traitCollection.userInterfaceIdiom == UIUserInterfaceIdiomPhone) return CGRectGetMinY(a) + a.size.height * .12;
    return MAX(CGRectGetMinY(self.bounds) + kButtonSize * 1.5, CGRectGetMinY(a) + kButtonSize / 2);
}
/* L3 / R3 in the bottom corners: on an iPad 46 points lower (the user, 2026-11-03: "l3 r3 往下 46点"), their bottom
   edge one button above the screen's bottom edge as the top row's top edge is below its top (centre 704 -> 751 on an
   11-inch iPad), never below the safe area; on an iPhone 88% down the safe area, as before. */
- (CGFloat)bottomRow {
    CGRect a = self.area;
    if (self.traitCollection.userInterfaceIdiom == UIUserInterfaceIdiomPhone) return CGRectGetMinY(a) + a.size.height * .88;
    return MIN(CGRectGetMaxY(self.bounds) - kButtonSize * 1.5, CGRectGetMaxY(a) - kButtonSize / 2);
}
- (CGPoint)restingStick:(BOOL)right {
    CGRect a = self.area; CGFloat inset = self.columnInset + self.stickRadius * 1.9;
    /* On an iPad the sticks rest another 90 points toward the edge (the user, 2026-11-03: "两个虚拟摇杆 靠近边缘 90点";
       centre 228 -> 138 points from the edge on an 11-inch iPad), the ring kept on the screen. */
    if (self.traitCollection.userInterfaceIdiom != UIUserInterfaceIdiomPhone) inset = MAX(inset - 90, self.stickRadius + 12);
    CGFloat lower = self.traitCollection.userInterfaceIdiom == UIUserInterfaceIdiomPhone ? 0 : 10;   // iPad: 10 points lower (the user, 2026-11-03)
    return CGPointMake(right ? CGRectGetMaxX(a) - inset : CGRectGetMinX(a) + inset, CGRectGetMinY(a) + a.size.height * .80 + lower);
}

- (void)buttons:(TouchButton *)b {
    CGRect a = self.area; CGFloat x = CGRectGetMinX(a), y = CGRectGetMinY(a), w = a.size.width, h = a.size.height, d = self.diamond;
    CGFloat top = self.topRow, bottom = self.bottomRow;
    CGFloat inset = self.columnInset, shift = self.shoulderShift;
    /* On an iPad the d-pad and A/B/X/Y sit 150 points lower (the user, 2026-11-03: "左边方向键、右边 ABXY ... 往下移 150 点";
       centre 336 -> 486 on an 11-inch iPad, the lowest button's edge still above the stick ring). */
    CGFloat mid = y + h * .42 + (self.traitCollection.userInterfaceIdiom == UIUserInterfaceIdiomPhone ? 0 : 150);
    CGPoint face = CGPointMake(x + w - inset, mid), pad = CGPointMake(x + inset, mid);
    TouchButton all[kButtonCount] = {
        {{face.x + d, face.y}, 1, @"A", kButtonSize}, {{face.x, face.y + d}, 2, @"B", kButtonSize}, {{face.x, face.y - d}, 4, @"X", kButtonSize}, {{face.x - d, face.y}, 8, @"Y", kButtonSize},
        {{x + w * .05 - shift, top}, 256, @"ZL", kButtonSize}, {{x + w * .16 - shift, top}, 64, @"L", kButtonSize}, {{x + w * .84 + shift, top}, 128, @"R", kButtonSize}, {{x + w * .95 + shift, top}, 512, @"ZR", kButtonSize},
        {{x + w * .40, top}, 2048, @"−", kButtonSize}, {{x + w * .60, top}, 1024, @"+", kButtonSize},
        {{pad.x - d, pad.y}, 4096, @"◀", kButtonSize}, {{pad.x, pad.y - d}, 8192, @"▲", kButtonSize}, {{pad.x + d, pad.y}, 16384, @"▶", kButtonSize}, {{pad.x, pad.y + d}, 32768, @"▼", kButtonSize},
        {{x + w * .05 - shift, bottom}, 16, @"L3", kButtonSize}, {{x + w * .95 + shift, bottom}, 32, @"R3", kButtonSize},
        {{x+w*.44,bottom},65536,@"K1",kButtonSize},
        {{x+w*.56,bottom},131072,@"K2",kButtonSize}};
    for (int i = 0; i < kButtonCount; i++) b[i] = all[i];
}

- (uint64_t)buttonAt:(CGPoint)p {
    TouchButton b[kButtonCount] = {0}; [self buttons:b]; CGFloat hit = self.hitRadius; uint64_t bits = 0;
    for (int i = 0; i < kButtonCount; i++) if (hypot(p.x - b[i].at.x, p.y - b[i].at.y) < MIN(hit, b[i].size * .75)) bits |= b[i].bit;
    return bits;
}


- (void)updateState {
    PortZeldaInputState s={0};CGFloat r=self.stickRadius;
    for(UITouch *t in points.keyEnumerator) {
        CGPoint p=[[points objectForKey:t] CGPointValue];
        NSValue *left=[leftOrigins objectForKey:t],*right=[rightOrigins objectForKey:t];
        NSValue *origin=left?:right;
        int role=left?PortZeldaLeft:right?PortZeldaRight:PortZeldaButtons;
        CGPoint from=origin?origin.CGPointValue:CGPointZero;
        PortZeldaAccumulate(&s,role,role==PortZeldaButtons?[self buttonAt:p]:0,from.x,from.y,p.x,p.y,r);
    }
    gPortRightTouchActive=rightOrigins.count!=0;
    gTraceState=s;gTraceSequence++;double started=NSProcessInfo.processInfo.systemUptime;
    PortPublishZelda(s);PortTracePublishFinished(started,points.count);[self setNeedsDisplay];
}
- (void)releaseAll {[self releaseAllForReason:@"explicit-reset"];}
- (void)releaseAllForReason:(NSString *)reason {
    PortTraceReset(reason,points.count);
    [points removeAllObjects];[leftOrigins removeAllObjects];[rightOrigins removeAllObjects];
    [self updateState];PortCancelShortcutKeys();
}
- (void)setPhysicalMode:(BOOL)physical {if(concealed==physical)return;[self releaseAllForReason:@"controller-change"];concealed=physical;[self setNeedsDisplay];}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    PortTouchTiming timing=PortTraceInputEntry(touches);
    if(concealed)return;
    for(UITouch *t in touches) {
        CGPoint p=[t locationInView:self];
        int role=PortZeldaRole([self buttonAt:p],p.x>=CGRectGetMidX(self.bounds),leftOrigins.count!=0,rightOrigins.count!=0);
        if(role==PortZeldaIgnored)continue;
        NSValue *v=[NSValue valueWithCGPoint:p];[points setObject:v forKey:t];
        if(role==PortZeldaLeft)[leftOrigins setObject:v forKey:t];
        if(role==PortZeldaRight)[rightOrigins setObject:v forKey:t];
    }
    [self updateState];PortTraceEvent(@"begin",touches,points.count,leftOrigins.count,rightOrigins.count,timing);
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    PortTouchTiming timing=PortTraceInputEntry(touches);
    if(concealed)return;
    for(UITouch *t in touches)if([points objectForKey:t])
        [points setObject:[NSValue valueWithCGPoint:[t locationInView:self]] forKey:t];
    [self updateState];PortTraceEvent(@"move",touches,points.count,leftOrigins.count,rightOrigins.count,timing);
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    PortTouchTiming timing=PortTraceInputEntry(touches);
    for(UITouch *t in touches){[points removeObjectForKey:t];[leftOrigins removeObjectForKey:t];[rightOrigins removeObjectForKey:t];}
    [self updateState];PortTraceEvent(@"end",touches,points.count,leftOrigins.count,rightOrigins.count,timing);
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    PortTraceReset(@"touch-cancelled",points.count);
    [self touchesEnded:touches withEvent:event];
}
- (void)safeAreaInsetsDidChange {
    [super safeAreaInsetsDidChange];[self setNeedsDisplay];
}
- (void)layoutSubviews {
    [super layoutSubviews];CGSize now=self.bounds.size;
    if(_lastBoundsSize.width>0 && _lastBoundsSize.height>0 &&
       (fabs(now.width-_lastBoundsSize.width)>1 || fabs(now.height-_lastBoundsSize.height)>1))
        [self releaseAllForReason:@"bounds-resize"];
    _lastBoundsSize=now;
}
- (void)drawRect:(CGRect)rect {} // The reused editor's drawPad supplies the shared Zelda artwork.
@end

// Include the adapted, otherwise preserved Zelda editor and settings next.
