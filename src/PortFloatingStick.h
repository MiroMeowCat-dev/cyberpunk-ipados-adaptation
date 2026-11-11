// Stick axes adapted from MacNativeGames games/totk/ios/Sources/touch_pad.m
// commit edb17ab98543c6cfcbc473928f6783741d9452d5 (MIT).
// Copyright (c) 2026 MacNativeGames contributors.
// Full license: upstream/botw-touch-reference/LICENSE (also shipped in Info.plist).
#ifndef PORT_FLOATING_STICK_H
#define PORT_FLOATING_STICK_H
#include <stdint.h>
#include "PortPadMath.h"

typedef struct {
    uintptr_t owner;
    double originX, originY, radius;
    PortPadVector value;
} PortFloatingStick;

// Preserve Zelda's linear radial clamp and signed-16-bit quantization. No
// extra 12% dead zone: the receiving game's controller settings own that.
static inline PortPadVector PortZeldaStickAxes(double dx, double dy, double radius) {
    if (!(radius>0) || !isfinite(radius) || !isfinite(dx) || !isfinite(dy))
        return (PortPadVector){0,0};
    double dist=hypot(dx,dy);
    if (!isfinite(dist))return (PortPadVector){0,0};
    if (dist>radius) {dx*=radius/dist;dy*=radius/dist;}
    int32_t x=(int32_t)(dx/radius*32767), y=(int32_t)(-dy/radius*32767);
    return (PortPadVector){x/32767.f,y/32767.f};
}
static inline int PortFloatingStickMove(PortFloatingStick *s,uintptr_t owner,double x,double y) {
    if (!owner || s->owner!=owner || !isfinite(x) || !isfinite(y))return 0;
    s->value=PortZeldaStickAxes(x-s->originX,y-s->originY,s->radius);return 1;
}
static inline int PortFloatingStickBegin(PortFloatingStick *s,uintptr_t owner,double x,double y,
                                         double homeX,double homeY,double radius,int floating) {
    if (s->owner || !owner || !(radius>0) || !isfinite(radius) || !isfinite(x) || !isfinite(y)
        || !isfinite(homeX) || !isfinite(homeY))return 0;
    s->owner=owner;s->originX=floating?x:homeX;s->originY=floating?y:homeY;s->radius=radius;
    return PortFloatingStickMove(s,owner,x,y);
}
static inline int PortFloatingStickEnd(PortFloatingStick *s,uintptr_t owner) {
    if (!s->owner || (owner && s->owner!=owner))return 0;
    s->owner=0;s->value=(PortPadVector){0,0};return 1;
}
static inline int PortFloatingStickHit(double x,double y,double homeX,double homeY,double radius,int priority) {
    if (!isfinite(x) || !isfinite(y) || !(radius>0) || !isfinite(radius))return 0;
    return priority || hypot(x-homeX,y-homeY)<=radius*1.6;
}
#endif
