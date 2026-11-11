#ifndef PORT_READY_PUMP_H
#define PORT_READY_PUMP_H
typedef struct {unsigned steps;int lastResult,capped;} PortPumpResult;
typedef int (*PortPumpStep)(void *);
typedef double (*PortPumpClock)(void *);
// Drain ready sources only. Step must be nonblocking; the cap is checked between
// callbacks and cannot preempt a slow callback. No timer, sleeps or spin-wait.
static inline PortPumpResult PortDrainReady(void *context,PortPumpStep step,PortPumpClock clock,
    unsigned maxSteps,double budgetSeconds,int handledSource) {
    PortPumpResult out={0,0,0};
    if(!maxSteps || !(budgetSeconds>0))return out;
    double start=clock(context);
    do {
        out.lastResult=step(context);out.steps++;
        if(out.lastResult!=handledSource)break;
        if(out.steps>=maxSteps || clock(context)-start>=budgetSeconds){out.capped=1;break;}
    }while(1);
    return out;
}
#endif
