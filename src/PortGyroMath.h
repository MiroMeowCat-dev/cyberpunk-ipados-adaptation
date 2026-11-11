#ifndef PORT_GYRO_RESPONSE_H
#define PORT_GYRO_RESPONSE_H
#include "botw_motion_math.h"
#include "PortPadMath.h"

static inline double port_gyro_gain(double gain) {
    return isfinite(gain)?fmin(4,fmax(.25,gain)):1.5;
}
// Remove the component of gravity along screen X before selecting the yaw axis.
// Yaw and pitch are therefore orthogonal even when the tablet is rolled.
static inline int port_gyro_rates(int orientation,const double rate[3],const double gravity[3],
    const double bias[3],double out[2]) {
    double corrected[3],r[3],g[3];out[0]=out[1]=0;
    if(!bm_vectors_valid(gravity,rate))return 0;
    for(int i=0;i<3;i++) {
        if(!isfinite(bias[i])||fabs(bias[i])>.00300001)return 0;
        corrected[i]=rate[i]-bias[i];
    }
    if(!bm_landscape(orientation,corrected,r)||!bm_landscape(orientation,gravity,g))return 0;
    double n=sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]),yawNorm=hypot(g[1],g[2]);
    if(n<.5||n>1.5||yawNorm<.2)return 0;
    out[0]=(r[1]*g[1]+r[2]*g[2])/yawNorm;out[1]=r[0];return 1;
}
static inline double port_gyro_soft_noise(double v) {
    // Continuous, odd response instead of jumping from zero at |rate|=.015.
    return copysign(fmax(0,fabs(v)-.004),v);
}
static inline PortPadVector port_gyro_response(const double rate[2],double gainX,double gainY,int invertX,int invertY) {
    double x=port_gyro_soft_noise(rate[0])*port_gyro_gain(gainX)*(invertX?-1:1);
    double y=port_gyro_soft_noise(rate[1])*port_gyro_gain(gainY)*(invertY?-1:1);
    double length=hypot(x,y);if(length>1){x/=length;y/=length;}
    return (PortPadVector){(float)x,(float)y};
}
typedef struct {double last,rate[2];int ready;} PortGyroFilter;
static inline void port_gyro_filter_reset(PortGyroFilter *f) {*f=(PortGyroFilter){0};}
static inline int port_gyro_filter(PortGyroFilter *f,const double in[2],double timestamp,double out[2]) {
    if(!isfinite(timestamp)||timestamp<=0||!isfinite(in[0])||!isfinite(in[1])) {
        port_gyro_filter_reset(f);out[0]=out[1]=0;return 0;
    }
    if(f->ready&&timestamp<f->last) {port_gyro_filter_reset(f);out[0]=out[1]=0;return 0;}
    if(!f->ready||timestamp-f->last>.1) {
        f->ready=1;f->rate[0]=in[0];f->rate[1]=in[1];
    } else if(timestamp>f->last) {
        double dt=timestamp-f->last,alpha=dt/(.012+dt);
        for(int i=0;i<2;i++)f->rate[i]+=alpha*(in[i]-f->rate[i]);
    }
    f->last=timestamp;out[0]=f->rate[0];out[1]=f->rate[1];return 1;
}
static inline PortPadVector port_gyro_merge(PortPadVector manual,int manualOwned,PortPadVector gyro,int allowed) {
    return manualOwned?manual:allowed?gyro:manual;
}
typedef struct {double first,last,sum[3],sumSquare[3];unsigned count;} PortGyroCalibration;
static inline int port_gyro_calibrate(PortGyroCalibration *c,const double rate[3],double timestamp,double bias[3]) {
    if(!isfinite(timestamp)||timestamp<=0||timestamp<=c->last)return 0;
    double norm=0;
    for(int i=0;i<3;i++){if(!isfinite(rate[i]))return 0;norm+=rate[i]*rate[i];}
    if(norm>.012*.012){*c=(PortGyroCalibration){0};c->last=timestamp;return 0;}
    if(!c->count)c->first=timestamp;
    c->last=timestamp;c->count++;
    for(int i=0;i<3;i++){c->sum[i]+=rate[i];c->sumSquare[i]+=rate[i]*rate[i];}
    if(c->count<16||timestamp-c->first<.75)return 0;
    double mean[3],meanNorm=0,variance=0;
    for(int i=0;i<3;i++) {
        mean[i]=c->sum[i]/c->count;meanNorm+=mean[i]*mean[i];
        variance+=fmax(0,c->sumSquare[i]/c->count-mean[i]*mean[i]);
    }
    // Core Motion rotationRate is already bias corrected. Never learn a user's
    // slow turn as a large bias: accept only a tiny stable residual vector.
    if(meanNorm>.003*.003||variance>.003*.003) {
        *c=(PortGyroCalibration){0};c->last=timestamp;return 0;
    }
    for(int i=0;i<3;i++)bias[i]=mean[i];return 1;
}
#endif
