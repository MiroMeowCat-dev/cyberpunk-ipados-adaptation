#include "PortZeldaInputState.h"
#include "PortInputSources.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned checks;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);exit(1);}}while(0)
static int near(float a,float b){return fabsf(a-b)<.00004f;}
int main(void) {
    // Touch-down classification: buttons win; each stick has one stable owner.
    CHECK(PortZeldaRole(1,0,0,0)==PortZeldaButtons);
    CHECK(PortZeldaRole(256,1,1,1)==PortZeldaButtons);
    CHECK(PortZeldaRole(0,0,0,0)==PortZeldaLeft);
    CHECK(PortZeldaRole(0,1,0,0)==PortZeldaRight);
    CHECK(PortZeldaRole(0,0,1,0)==PortZeldaIgnored);
    CHECK(PortZeldaRole(0,1,0,1)==PortZeldaIgnored);
    // A single aggregate publishes left movement, right camera and several keys.
    PortZeldaInputState s={0};
    PortZeldaAccumulate(&s,PortZeldaLeft,0,100,600,135,600,70);
    PortZeldaAccumulate(&s,PortZeldaRight,0,950,600,950,565,70);
    PortZeldaAccumulate(&s,PortZeldaButtons,256,0,0,0,0,70);
    PortZeldaAccumulate(&s,PortZeldaButtons,1,0,0,0,0,70);
    PortZeldaAccumulate(&s,PortZeldaButtons,65536,0,0,0,0,70);
    CHECK(near(s.left.x,.5f)&&s.left.y==0);
    CHECK(near(s.right.y,.5f)&&s.right.x==0);
    CHECK(s.buttons==(256|1|65536));
    PortZeldaAccumulate(&s,PortZeldaIgnored,~0ULL,0,0,20,20,70);
    CHECK(s.buttons==(256|1|65536));
    // Next snapshot excludes a released finger, preserving other held fingers.
    s=(PortZeldaInputState){0};
    PortZeldaAccumulate(&s,PortZeldaRight,0,950,600,950,565,70);
    PortZeldaAccumulate(&s,PortZeldaButtons,256,0,0,0,0,70);
    CHECK(s.left.x==0&&s.left.y==0&&near(s.right.y,.5f)&&s.buttons==256);
    // Zelda button fingers slide between keys without becoming sticks.
    s=(PortZeldaInputState){0};PortZeldaAccumulate(&s,PortZeldaButtons,2,0,0,0,0,70);
    CHECK(s.buttons==2&&s.left.x==0&&s.right.x==0);
    s=(PortZeldaInputState){0};PortZeldaAccumulate(&s,PortZeldaButtons,0,0,0,0,0,70);
    CHECK(s.buttons==0&&s.left.x==0&&s.right.x==0);
    // Stick owner keeps its original role even when crossing the midpoint.
    PortZeldaAccumulate(&s,PortZeldaLeft,1,100,600,1000,600,70);
    CHECK(near(s.left.x,1)&&s.buttons==0&&s.right.x==0);
    s=(PortZeldaInputState){0};
    CHECK(s.buttons==0&&s.left.x==0&&s.left.y==0&&s.right.x==0&&s.right.y==0);
    // Fixed origin and floating origin both use the same Zelda axis mapping.
    PortZeldaAccumulate(&s,PortZeldaLeft,0,100,600,100,600,70);CHECK(s.left.x==0);
    PortZeldaAccumulate(&s,PortZeldaLeft,0,100,600,103,600,70);CHECK(s.left.x>0&&s.left.x<.05f);
    PortZeldaAccumulate(&s,PortZeldaLeft,0,100,600,100,530,70);CHECK(near(s.left.y,1));
    PortZeldaAccumulate(&s,PortZeldaLeft,0,100,600,NAN,0,70);CHECK(s.left.x==0&&s.left.y==0);
    uint32_t held=PortInputSources(0,20,1);held=PortInputSources(held,21,1);
    CHECK(held==((1u<<20)|(1u<<21)));
    held=PortInputSources(held,20,0);CHECK(held==(1u<<21));
    CHECK(PortInputSources(held,32,0)==held);
    CHECK(PortInputSources(held,21,0)==0);
    printf("%u unified Zelda input assertions passed\n",checks);return 0;
}
