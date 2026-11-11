#include "PortFloatingStick.h"
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
static int near(float a,float b){return fabsf(a-b)<.00004f;}
// Independent reference preserves the pinned Zelda stick_axes operation order.
static PortPadVector reference(double fromX,double fromY,double toX,double toY,double radius) {
    double dx=toX-fromX,dy=fromY-toY,dist=hypot(dx,dy);
    if(dist>radius){dx*=radius/dist;dy*=radius/dist;}
    int32_t x=(int32_t)(dx/radius*32767),y=(int32_t)(dy/radius*32767);
    return (PortPadVector){x/32767.f,y/32767.f};
}
int main(void) {
    PortFloatingStick left={0},right={0};
    CHECK(PortFloatingStickBegin(&left,1,245,600,100,650,70,1));
    CHECK(left.value.x==0 && left.value.y==0); // off-center touch does not jump
    CHECK(!PortFloatingStickBegin(&left,2,270,610,100,650,70,1));
    CHECK(!PortFloatingStickMove(&left,2,300,600)); // another finger cannot steal
    CHECK(PortFloatingStickMove(&left,1,248,600));
    CHECK(left.value.x>0 && left.value.x<.05f); // no extra 12% dead-zone cliff
    CHECK(PortFloatingStickMove(&left,1,315,600));
    CHECK(near(left.value.x,1) && left.value.y==0);
    CHECK(PortFloatingStickBegin(&right,2,900,610,100,650,70,1));
    CHECK(PortFloatingStickMove(&right,2,900,575));
    CHECK(near(right.value.y,.5f) && near(left.value.x,1)); // independent two thumbs
    CHECK(PortFloatingStickMove(&left,1,1000,700)); // crossing midpoint keeps ownership
    CHECK(left.owner==1 && right.owner==2 && hypotf(left.value.x,left.value.y)<=1.00001f);
    CHECK(!PortFloatingStickEnd(&left,2));
    CHECK(left.owner==1);
    CHECK(PortFloatingStickEnd(&left,1));
    CHECK(left.owner==0 && left.value.x==0 && left.value.y==0 && right.owner==2);
    CHECK(!PortFloatingStickMove(&left,1,300,600));
    CHECK(PortFloatingStickEnd(&right,0)); // background/hide/settings cancellation
    CHECK(right.owner==0 && right.value.x==0 && right.value.y==0);
    CHECK(PortFloatingStickBegin(&left,3,135,650,100,650,70,0));
    CHECK(near(left.value.x,.5f)); // fixed mode keeps its resting center
    CHECK(PortFloatingStickEnd(&left,0));
    CHECK(!PortFloatingStickBegin(&left,1,NAN,0,0,0,70,1));
    CHECK(!PortFloatingStickBegin(&left,0,1,1,0,0,70,1));
    CHECK(!PortFloatingStickBegin(&left,1,1,1,0,0,0,1));
    CHECK(!PortFloatingStickHit(300,400,100,650,70,0));
    CHECK(PortFloatingStickHit(300,400,100,650,70,1));
    CHECK(PortFloatingStickHit(150,650,100,650,70,0));
    CHECK(!PortFloatingStickHit(NAN,650,100,650,70,1));
    unsigned comparisons=0;
    for(int x=-10;x<=10;x++)for(int y=-10;y<=10;y++) {
        PortPadVector actual=PortZeldaStickAxes(x*13.25,y*9.75,70);
        PortPadVector expected=reference(200,300,200+x*13.25,300+y*9.75,70);
        CHECK(near(actual.x,expected.x) && near(actual.y,expected.y));comparisons++;
    }
    puts("30 floating/fixed, ownership, cancellation and capture assertions passed");
    printf("%u comparisons against pinned Zelda axes passed\n",comparisons);return 0;
}
