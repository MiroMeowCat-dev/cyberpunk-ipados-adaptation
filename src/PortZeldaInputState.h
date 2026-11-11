#ifndef PORT_ZELDA_INPUT_STATE_H
#define PORT_ZELDA_INPUT_STATE_H
#include "PortFloatingStick.h"
typedef struct {uint64_t buttons;PortPadVector left,right;} PortZeldaInputState;
enum {PortZeldaIgnored,PortZeldaButtons,PortZeldaLeft,PortZeldaRight};
static inline int PortZeldaRole(uint64_t buttons,int rightHalf,int leftTaken,int rightTaken) {
    if(buttons)return PortZeldaButtons;
    if(rightHalf)return rightTaken?PortZeldaIgnored:PortZeldaRight;
    return leftTaken?PortZeldaIgnored:PortZeldaLeft;
}
static inline void PortZeldaAccumulate(PortZeldaInputState *s,int role,uint64_t buttons,
    double fromX,double fromY,double x,double y,double radius) {
    if(role==PortZeldaButtons)s->buttons|=buttons;
    else if(role==PortZeldaLeft)s->left=PortZeldaStickAxes(x-fromX,y-fromY,radius);
    else if(role==PortZeldaRight)s->right=PortZeldaStickAxes(x-fromX,y-fromY,radius);
}
#endif
