#include "PortReadyPump.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned checks;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);exit(1);}}while(0)
typedef struct {int remaining,terminal,calls;double now,cost;} Queue;
static int step(void *context) {
    Queue *q=context;q->calls++;q->now+=q->cost;
    if(q->remaining<0)return 4;
    if(q->remaining){q->remaining--;return 4;}
    return q->terminal;
}
static double clock_now(void *context){return ((Queue *)context)->now;}
int main(void) {
    Queue q={5,3,0,0,.00005};PortPumpResult r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.remaining==0&&q.calls==6&&r.steps==6&&!r.capped); // old single pass left 4 ready
    q=(Queue){-1,3,0,0,0};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==16&&r.capped); // permanently ready producer cannot spin forever
    q=(Queue){-1,3,0,0,.0011};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==2&&r.capped); // callback budget stops between steps
    q=(Queue){-1,3,0,0,.02};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==1&&r.capped); // no claim that long callbacks are preempted
    q=(Queue){0,3,0,0,0};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==1&&r.lastResult==3&&!r.capped); // idle: no busy polling
    q=(Queue){0,2,0,0,0};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==1&&r.lastResult==2&&!r.capped); // run-loop stopped
    q=(Queue){0,1,0,0,0};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==1&&r.lastResult==1&&!r.capped); // run-loop finished
    q=(Queue){5,3,0,0,0};r=PortDrainReady(&q,step,clock_now,0,.002,4);
    CHECK(q.calls==0&&r.steps==0);
    r=PortDrainReady(&q,step,clock_now,16,0,4);CHECK(q.calls==0);
    q=(Queue){-1,3,0,0,-.001};r=PortDrainReady(&q,step,clock_now,16,.002,4);
    CHECK(q.calls==16&&r.capped); // even an invalid backward test clock stays bounded
    printf("%u ready-pump scheduling checks passed\n",checks);return 0;
}
