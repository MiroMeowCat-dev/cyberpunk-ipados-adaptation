#ifndef PORT_PAD_MATH_H
#define PORT_PAD_MATH_H
#include <math.h>
typedef struct { float x, y; } PortPadVector;
static inline PortPadVector PortPadComputeStick(float dx, float dy, float radius) {
    if (!(radius > 0) || !isfinite(dx) || !isfinite(dy)) return (PortPadVector){0,0};
    float x=dx/radius, y=-dy/radius, length=hypotf(x,y);
    if (length<=0.12f) return (PortPadVector){0,0};
    float magnitude=fminf(length,1.f);
    magnitude=(magnitude-0.12f)/0.88f;
    return (PortPadVector){x/length*magnitude,y/length*magnitude};
}
static inline PortPadVector PortPadDirections(unsigned held) {
    return (PortPadVector){((held&2)?1.f:0)-((held&1)?1.f:0),
                          ((held&4)?1.f:0)-((held&8)?1.f:0)};
}
#endif
