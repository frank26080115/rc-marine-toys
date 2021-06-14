#ifndef _MPU6050_T3_H_
#define _MPU6050_T3_H_

#include <stdint.h>
//#include <MPU6050.h>
#include "helper_3dmath.h"

extern float       mpu_ypr   [3];
extern float       mpu_euler [3];
extern Quaternion  mpu_quat;
extern VectorFloat mpu_gravity;
extern int16_t     mpu_gyros [3];
extern int16_t     mpu_accels[3];
extern uint8_t     mpu_fifo  [128];
extern bool        mpu_new;

void mpu_task(void);
void mpu_doMath(void);

#endif