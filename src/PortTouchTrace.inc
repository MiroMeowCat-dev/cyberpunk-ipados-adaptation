// Bounded diagnostics for this game's control panel, not keyboard text or accounts.
static unsigned gTraceSequence,gTraceDrawSequence,gTraceEvents,gTraceDraws,gTraceResets;
static NSTimeInterval gTraceLastMove,gTraceLastDraw;
static PortZeldaInputState gTraceState;
typedef struct {double started,entryAge;} PortTouchTiming;
static double gTracePublishMs;
static PortTouchTiming PortTraceInputEntry(NSSet<UITouch *> *touches) {
    PortTouchTiming timing={NSProcessInfo.processInfo.systemUptime,0};
    for(UITouch *t in touches)timing.entryAge=MAX(timing.entryAge,timing.started-t.timestamp);
    return timing;
}
static void PortTracePublishFinished(double started,NSUInteger active) {
    gTracePublishMs=(NSProcessInfo.processInfo.systemUptime-started)*1000;
    static unsigned slowLogs;
    if(gTracePublishMs>10 && slowLogs++<60)
        NSLog(@"PORTPAD publish seq=%u active=%lu elapsed-ms=%.2f",gTraceSequence,(unsigned long)active,gTracePublishMs);
}
static void PortTraceReset(NSString *reason,NSUInteger active) {
    if(active && gTraceResets++<32)NSLog(@"PORTPAD reset reason=%@ active=%lu seq=%u",reason,(unsigned long)active,gTraceSequence);
}
static void PortTraceEvent(NSString *phase,NSSet<UITouch *> *touches,NSUInteger active,NSUInteger left,NSUInteger right,PortTouchTiming timing) {
    NSTimeInterval now=NSProcessInfo.processInfo.systemUptime;
    if([phase isEqual:@"move"] && now-gTraceLastMove<1)return;
    if([phase isEqual:@"move"])gTraceLastMove=now;
    if(gTraceEvents++>=2000)return;
    NSTimeInterval age=0;for(UITouch *t in touches)age=MAX(age,now-t.timestamp);
    NSLog(@"PORTPAD touch phase=%@ incoming=%lu active=%lu left=%lu right=%lu seq=%u entry-age-ms=%.1f callback-ms=%.1f publish-ms=%.1f age-ms=%.1f axes=%.2f,%.2f,%.2f,%.2f",phase,
        (unsigned long)touches.count,(unsigned long)active,(unsigned long)left,(unsigned long)right,gTraceSequence,
        timing.entryAge*1000,(now-timing.started)*1000,gTracePublishMs,age*1000,gTraceState.left.x,gTraceState.left.y,gTraceState.right.x,gTraceState.right.y);
}
static void PortTraceDraw(NSUInteger active) {
    NSTimeInterval now=NSProcessInfo.processInfo.systemUptime;
    gTraceDrawSequence=gTraceSequence;
    if(now-gTraceLastDraw<1 || gTraceDraws++>=3600)return;gTraceLastDraw=now;
    NSLog(@"PORTPAD draw seq=%u active=%lu",gTraceDrawSequence,(unsigned long)active);
}
