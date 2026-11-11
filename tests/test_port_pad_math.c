#include "PortPadMath.h"
#include <stdio.h>
static int near(float a,float b){return fabsf(a-b)<.0001f;}
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;} } while(0)
int main(void) {
    PortPadVector p=PortPadComputeStick(0,0,48);CHECK(p.x==0&&p.y==0);
    p=PortPadComputeStick(3,3,48);CHECK(p.x==0&&p.y==0);
    p=PortPadComputeStick(48,0,48);CHECK(near(p.x,1)&&near(p.y,0));
    p=PortPadComputeStick(0,-48,48);CHECK(near(p.x,0)&&near(p.y,1));
    p=PortPadComputeStick(96,96,48);CHECK(near(hypotf(p.x,p.y),1)&&p.x>0&&p.y<0);
    p=PortPadComputeStick(0,0,0);CHECK(p.x==0&&p.y==0);
    p=PortPadComputeStick(NAN,0,48);CHECK(p.x==0&&p.y==0);
    p=PortPadDirections(3);CHECK(p.x==0&&p.y==0);
    p=PortPadDirections(12);CHECK(p.x==0&&p.y==0);
    p=PortPadDirections(6);CHECK(p.x==1&&p.y==1);
    p=PortPadDirections(0);CHECK(p.x==0&&p.y==0);
    puts("11 joystick/direction behavioral cases passed");return 0;
}
