// Used only by the Mac polling-loop bridge. UIKit stays on the main thread.
#include "PortReadyPump.h"
extern void PortCommitTouchUI(void);
static int PortPumpStepReady(void *context) {
    return (int)CFRunLoopRunInMode(kCFRunLoopDefaultMode,0,true);
}
static double PortPumpUptime(void *context) {return NSProcessInfo.processInfo.systemUptime;}
static void PortServiceUIKit(NSTimeInterval wait) {
    static BOOL pumping;
    if(pumping)return;
    pumping=YES;
    @try {
        // Preserve the caller's existing wait behavior; all subsequent steps poll.
        if(wait>0)CFRunLoopRunInMode(kCFRunLoopDefaultMode,MIN(wait,0.25),true);
        double begin=PortPumpUptime(NULL);
        PortPumpResult result=PortDrainReady(NULL,PortPumpStepReady,PortPumpUptime,16,.002,kCFRunLoopRunHandledSource);
        PortCommitTouchUI();
        static unsigned calls,steps,caps,logs;static double last,maxCost;
        calls++;steps+=result.steps;caps+=result.capped;
        double now=PortPumpUptime(NULL);maxCost=MAX(maxCost,now-begin);
        if(now-last>=5 && logs++<60) {
            NSLog(@"PORTPAD pump calls=%u ready-steps=%u caps=%u max-service-ms=%.2f",calls,steps,caps,maxCost*1000);
            calls=steps=caps=0;maxCost=0;last=now;
        }
    }@finally {pumping=NO;}
}
