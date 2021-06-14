#include <MPU6050_t3.h>
#include <i2c_t3.h>
#include <Arduino.h>

//#include "MPU6050_6Axis_MotionApps20.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"

enum
{
    MPUSM_INIT,
    MPUSM_GETFIFOCNT_WRITE,
    MPUSM_FIFORW_WRITE,
    MPUSM_GETFIFOCNT_WRITE_WAIT,
    MPUSM_FIFORW_WRITE_WAIT,
    MPUSM_GETFIFOCNT_READ,
    MPUSM_FIFORW_READ,
    MPUSM_GETFIFOCNT_READ_WAIT,
    MPUSM_FIFORW_READ_WAIT,
    MPUSM_WAIT,
    MPUSM_WAIT_SHORT,
    MPUSM_FAILED,
};

MPU6050     mpu;
float       mpu_ypr   [3];
float       mpu_euler [3];
Quaternion  mpu_quat;
VectorFloat mpu_gravity;
int16_t     mpu_gyros [3];
int16_t     mpu_accels[3];
uint8_t     mpu_fifo  [128];
bool        mpu_new          = false;
uint32_t    mpu_fifoCnt      = 0;
uint32_t    mpu_timestamp    = 0;
uint32_t    mpu_timeoutstamp = 0;

uint8_t mpu_statemachine = MPUSM_INIT;

void mpu_task()
{
    static uint32_t dmpFifoSize = 0;
    uint32_t avail;
    switch (mpu_statemachine)
    {
        case MPUSM_INIT:
            Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
            Wire.resetBus();
            Wire.setDefaultTimeout(5000);
            Wire.setOpMode(I2C_OP_MODE_IMM);
            mpu.initialize();
            if (mpu.dmpInitialize() == 0)
            {
                //mpu.CalibrateAccel(6);
                //mpu.CalibrateGyro(6);
                mpu.setDMPEnabled(true);
                dmpFifoSize = mpu.dmpGetFIFOPacketSize();
                mpu_statemachine = MPUSM_WAIT;
                mpu_timestamp = millis();
            }
            else
            {
                mpu_statemachine = MPUSM_FAILED;
            }
            break;
        case MPUSM_GETFIFOCNT_WRITE:
        case MPUSM_FIFORW_WRITE:
            Wire.setOpMode(I2C_OP_MODE_ISR);
            Wire.beginTransmission(MPU6050_DEFAULT_ADDRESS);
            Wire.write((mpu_statemachine == MPUSM_GETFIFOCNT_WRITE) ? MPU6050_RA_FIFO_COUNTH : MPU6050_RA_FIFO_R_W);
            Wire.sendTransmission(I2C_NOSTOP);
            mpu_statemachine = (mpu_statemachine == MPUSM_GETFIFOCNT_WRITE) ? MPUSM_GETFIFOCNT_WRITE_WAIT : MPUSM_FIFORW_WRITE_WAIT;
            mpu_timeoutstamp = mpu_timestamp;
            break;
        case MPUSM_GETFIFOCNT_WRITE_WAIT:
        case MPUSM_FIFORW_WRITE_WAIT:
            if (Wire.getError() != 0)
            {
                Wire.finish();
                mpu_statemachine = MPUSM_GETFIFOCNT_WRITE;
            }
            else if (Wire.done())
            {
                mpu_statemachine = (mpu_statemachine == MPUSM_GETFIFOCNT_WRITE_WAIT) ? MPUSM_GETFIFOCNT_READ : MPUSM_FIFORW_READ;
            }
            else
            {
                if ((millis() - mpu_timeoutstamp) > 5)
                {
                    Wire.resetBus();
                    mpu_statemachine = MPUSM_GETFIFOCNT_WRITE;
                }
            }
            break;
        case MPUSM_GETFIFOCNT_READ:
        case MPUSM_FIFORW_READ:
            Wire.sendRequest(MPU6050_DEFAULT_ADDRESS,
                    (mpu_statemachine == MPUSM_GETFIFOCNT_READ) ? 2 : dmpFifoSize
                , I2C_STOP);
            mpu_statemachine = (mpu_statemachine == MPUSM_GETFIFOCNT_READ) ? MPUSM_GETFIFOCNT_READ_WAIT : MPUSM_FIFORW_READ_WAIT;
            mpu_timeoutstamp = millis();
            if (mpu_statemachine == MPUSM_FIFORW_READ_WAIT)
            {
                mpu_timestamp = mpu_timeoutstamp;
            }
            break;
        case MPUSM_GETFIFOCNT_READ_WAIT:
        case MPUSM_FIFORW_READ_WAIT:
            if (Wire.getError() != 0)
            {
                Wire.finish();
                mpu_statemachine = MPUSM_GETFIFOCNT_WRITE;
            }
            else
            {
                avail = Wire.available();
                if ((avail >= 2 && mpu_statemachine == MPUSM_GETFIFOCNT_READ_WAIT) || (avail >= dmpFifoSize && mpu_statemachine == MPUSM_FIFORW_READ_WAIT) || Wire.done())
                {
                    if (mpu_statemachine == MPUSM_GETFIFOCNT_READ_WAIT)
                    {
                        uint8_t hi, lo;
                        uint16_t x;
                        int16_t* xp = (int16_t*)&x;
                        hi = Wire.readByte();
                        lo = Wire.readByte();
                        x = (hi << 8) | lo;
                        mpu_fifoCnt = *xp;
                        if (mpu_fifoCnt >= dmpFifoSize)
                        {
                            mpu_statemachine = MPUSM_FIFORW_WRITE;
                        }
                        else
                        {
                            mpu_timestamp = millis();
                            mpu_statemachine = MPUSM_WAIT_SHORT;
                        }
                    }
                    else // read from FIFO itself
                    {
                        Wire.read(mpu_fifo, dmpFifoSize);
                        mpu_fifoCnt -= dmpFifoSize;
                        if (mpu_fifoCnt >= dmpFifoSize)
                        {
                            mpu_statemachine = MPUSM_FIFORW_WRITE; // too many bytes, read again
                        }
                        else
                        {
                            mpu_timestamp = millis();
                            mpu_statemachine = MPUSM_WAIT;
                            mpu_new = true;
                        }
                    }
                }
                else
                {
                    if ((millis() - mpu_timeoutstamp) > 5)
                    {
                        Wire.resetBus();
                        mpu_statemachine = MPUSM_GETFIFOCNT_WRITE;
                    }
                }
            }
            break;
        case MPUSM_WAIT:
            if ((millis() - mpu_timestamp) >= 5)
            {
                mpu_statemachine = MPUSM_GETFIFOCNT_WRITE;
            }
            break;
        case MPUSM_WAIT_SHORT:
            if ((millis() - mpu_timestamp) >= 2)
            {
                mpu_statemachine = MPUSM_GETFIFOCNT_WRITE;
            }
            break;
        case MPUSM_FAILED:
            if ((millis() - mpu_timestamp) >= 1000)
            {
                mpu_timestamp = millis();
            }
            break;
    }
}

void mpu_doMath()
{
    mpu.dmpGetQuaternion  (&mpu_quat           , (const uint8_t*)mpu_fifo);
    mpu.dmpGetEuler       ((float*)mpu_euler   , &mpu_quat);
    mpu.dmpGetGravity     (&mpu_gravity        , &mpu_quat);
    mpu.dmpGetYawPitchRoll((float*)mpu_ypr     , &mpu_quat, &mpu_gravity);
    mpu.dmpGetGyro        ((int16_t*)mpu_gyros , (const uint8_t*)mpu_fifo);
    mpu.dmpGetAccel       ((int16_t*)mpu_accels, (const uint8_t*)mpu_fifo);
}
