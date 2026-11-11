// Main-thread pull of the latest Core Motion sample. No per-sample dispatch queue,
// networking, raw sample history, or touch-to-mouse input is used.
#import <CoreMotion/CoreMotion.h>
static CMMotionManager *gPortMotion;
static BOOL gPortMotionRunning,gPortCalibrating;
static int gPortMotionOrientation;
static double gPortBias[3],gPortCalibrationStarted,gPortLastMotionTimestamp;
static PortGyroCalibration gPortCalibration;
static PortGyroFilter gPortGyroFilter;
static uint64_t gPortGyroStale,gPortGyroInvalid,gPortGyroCalibrations;
static unsigned gPortGyroReports;
static double gPortGyroLastReport;
static double gPortGyroMaxSampleGap,gPortGyroMaxAge;
static NSString *gPortCalibrationLabel=@"握稳 iPad 后点此校准";
static uint64_t gPortMotionSamples,gPortMotionNonzero;
static BOOL PortGyroEnabled(void) {
    id value=[NSUserDefaults.standardUserDefaults objectForKey:@"PortGyroEnabled"];return value?[value boolValue]:YES;
}
static int PortGyroMode(void) {return [NSUserDefaults.standardUserDefaults integerForKey:@"PortGyroMode"]==1?1:0;}
static double PortGyroGain(void) {
    id value=[NSUserDefaults.standardUserDefaults objectForKey:@"PortGyroGain"];
    return port_gyro_gain(value?[value doubleValue]:1.5);
}
static double PortGyroAxisGain(BOOL vertical) {
    id value=[NSUserDefaults.standardUserDefaults objectForKey:vertical?@"PortGyroVerticalGain":@"PortGyroHorizontalGain"];
    double gain=value?[value doubleValue]:1;return isfinite(gain)?fmin(2.5,fmax(.5,gain)):1;
}
static void PortGyroCalibrationDone(NSString *message) {
    gPortCalibrating=NO;gPortCalibrationLabel=message;
    [NSNotificationCenter.defaultCenter postNotificationName:@"PortGyroCalibrationFinished" object:nil];
}
static void PortGyroSuspend(void) {
    if(gPortMotionRunning)[gPortMotion stopDeviceMotionUpdates];
    port_gyro_filter_reset(&gPortGyroFilter);
    gPortMotionRunning=NO;gPortGyroAllowed=NO;gPortGyroAxis=(PortPadVector){0,0};
    if(!gPortRightTouchActive)PortSetPad(gTouchController.extendedGamepad.rightThumbstick,0,0);
    if(gPortCalibrating)PortGyroCalibrationDone(@"校准已取消，可重新开始");
}
static void PortGyroRecenter(void) {
    if(!PortGyroEnabled()){gPortCalibrationLabel=@"请先开启体感";return;}
    port_gyro_filter_reset(&gPortGyroFilter);
    gPortCalibration=(PortGyroCalibration){0};gPortCalibrationStarted=NSProcessInfo.processInfo.systemUptime;
    gPortCalibrating=YES;gPortCalibrationLabel=@"保持静止，正在采样…";
    gPortGyroAllowed=NO;gPortGyroAxis=(PortPadVector){0,0};
    if(!gPortRightTouchActive)PortSetPad(gTouchController.extendedGamepad.rightThumbstick,0,0);
}
static NSString *PortGyroStatus(void) {return gPortCalibrationLabel;}
static void PortGyroTick(void) {
    @autoreleasepool {
        BOOL appActive=UIApplication.sharedApplication.applicationState==UIApplicationStateActive;
        int orientation=(int)gTouchWindow.windowScene.effectiveGeometry.interfaceOrientation;
        BOOL landscape=orientation==BM_LANDSCAPE_LEFT||orientation==BM_LANDSCAPE_RIGHT;
        BOOL blocked=state(gZeldaSurface).settingsPresented||state(gZeldaSurface).editor!=nil;
        BOOL canUse=appActive&&PortGyroEnabled()&&!gPhysicalMode&&gPublished&&landscape;
        BOOL run=canUse&&(!blocked||gPortCalibrating);
        PortPadVector axis={0,0};BOOL output=NO;
        if(!run) {
            if(gPortMotionRunning || gPortCalibrating)PortGyroSuspend();
        } else {
            if(!gPortMotion)gPortMotion=[CMMotionManager new];
            if(!gPortMotion.deviceMotionAvailable || !([CMMotionManager availableAttitudeReferenceFrames]&CMAttitudeReferenceFrameXArbitraryZVertical)) {
                if(gPortMotionRunning){[gPortMotion stopDeviceMotionUpdates];gPortMotionRunning=NO;}
                if(gPortCalibrating)PortGyroCalibrationDone(@"此时无法获取传感器");
            } else {
                if(gPortMotionRunning && gPortMotionOrientation!=orientation) {
                    port_gyro_filter_reset(&gPortGyroFilter);
                    [gPortMotion stopDeviceMotionUpdates];gPortMotionRunning=NO;
                    if(gPortCalibrating)PortGyroCalibrationDone(@"方向已改变，请重新校准");
                }
                if(!gPortMotionRunning) {
                    gPortGyroReports=0;gPortGyroLastReport=0;
                    gPortMotion.deviceMotionUpdateInterval=1.0/120.0;
                    [gPortMotion startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryZVertical];
                    gPortMotionRunning=YES;gPortMotionOrientation=orientation;gPortLastMotionTimestamp=0;
                }
                CMDeviceMotion *sample=gPortMotion.deviceMotion;
                double now=NSProcessInfo.processInfo.systemUptime;
                if(sample&&bm_sample_fresh(sample.timestamp,(uint64_t)(now*1e9))) {
                    CMRotationRate w=sample.rotationRate;CMAcceleration g=sample.gravity;
                    double rate[3]={w.x,w.y,w.z},gravity[3]={g.x,g.y,g.z};
                    gPortGyroMaxAge=MAX(gPortGyroMaxAge,now-sample.timestamp);
                    if(sample.timestamp!=gPortLastMotionTimestamp){
                        if(gPortLastMotionTimestamp>0)gPortGyroMaxSampleGap=MAX(gPortGyroMaxSampleGap,sample.timestamp-gPortLastMotionTimestamp);
                        gPortMotionSamples++;gPortLastMotionTimestamp=sample.timestamp;
                    }
                    if(gPortCalibrating) {
                        if(port_gyro_calibrate(&gPortCalibration,rate,sample.timestamp,gPortBias)) {
                            gPortGyroCalibrations++;port_gyro_filter_reset(&gPortGyroFilter);PortGyroCalibrationDone(@"校准完成");
                        }
                    }
                    output=canUse&&!blocked&&!gPortCalibrating&&(PortGyroMode()==0||gTouchController.extendedGamepad.leftTrigger.value>.2f);
                    if(output&&!gPortRightTouchActive) {
                        double projected[2],filtered[2];
                        if(port_gyro_rates(orientation,rate,gravity,gPortBias,projected)&&port_gyro_filter(&gPortGyroFilter,projected,sample.timestamp,filtered))
                            axis=port_gyro_response(filtered,PortGyroGain()*PortGyroAxisGain(NO),PortGyroGain()*PortGyroAxisGain(YES),
                                [NSUserDefaults.standardUserDefaults boolForKey:@"PortGyroInvertX"],
                                [NSUserDefaults.standardUserDefaults boolForKey:@"PortGyroInvertY"]);
                        else {gPortGyroInvalid++;port_gyro_filter_reset(&gPortGyroFilter);output=NO;}
                    } else port_gyro_filter_reset(&gPortGyroFilter);
                } else {gPortGyroStale++;port_gyro_filter_reset(&gPortGyroFilter);}
            }
        }
        if(gPortCalibrating&&NSProcessInfo.processInfo.systemUptime-gPortCalibrationStarted>3)
            PortGyroCalibrationDone(@"未取得稳定数据，请重试");
        gPortGyroAllowed=output;gPortGyroAxis=axis;
        if(!gPortRightTouchActive)PortSetPad(gTouchController.extendedGamepad.rightThumbstick,axis.x,axis.y);
        if(axis.x!=0||axis.y!=0)gPortMotionNonzero++;
        double now=NSProcessInfo.processInfo.systemUptime;
        if(gPortGyroReports<120 && now-gPortGyroLastReport>=5) {
            gPortGyroReports++;
            NSLog(@"PORTGYRO enabled=%d mode=%d available=%d active=%d output=%d manual=%d samples=%llu nonzero=%llu calibrating=%d stale=%llu invalid=%llu bias-norm=%.5f calibration-count=%llu max-gap-ms=%.1f max-age-ms=%.1f gain-x=%.2f gain-y=%.2f",
                PortGyroEnabled(),PortGyroMode(),gPortMotion.deviceMotionAvailable,gPortMotion.deviceMotionActive,
                output,gPortRightTouchActive,(unsigned long long)gPortMotionSamples,(unsigned long long)gPortMotionNonzero,gPortCalibrating,
                (unsigned long long)gPortGyroStale,(unsigned long long)gPortGyroInvalid,
                sqrt(gPortBias[0]*gPortBias[0]+gPortBias[1]*gPortBias[1]+gPortBias[2]*gPortBias[2]),
                (unsigned long long)gPortGyroCalibrations,gPortGyroMaxSampleGap*1000,gPortGyroMaxAge*1000,
                PortGyroAxisGain(NO),PortGyroAxisGain(YES));
            gPortGyroLastReport=now;gPortGyroMaxSampleGap=gPortGyroMaxAge=0;
        }
    }
}
