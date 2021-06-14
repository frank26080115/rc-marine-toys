#include <i2c_t3.h>
#include "mpu_defs.h"

void mpu_init()
{
    mpu_busFreeCheck();
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
    Wire.setDefaultTimeout(250000); // 250ms default timeout
    mpu_writeReg(MPU6050_RA_PWR_MGMT_1, 0x80);
    delay(1);
    mpu_writeReg(MPU6050_RA_PWR_MGMT_1, 0x00);
    mpu_writeBits(MPU6050_RA_GYRO_CONFIG , MPU6050_GCONFIG_FS_SEL_BIT , MPU6050_GCONFIG_FS_SEL_LENGTH , MPU6050_GYRO_FS_250);
    mpu_writeBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);
    mpu_writeReg(MPU6050_RA_CONFIG, 0);
    mpu_writeReg(MPU6050_RA_SMPLRT_DIV, 4);
}

enum
{
    MPUSM_IDLE,
    MPUSM_WRITE,
    MPUSM_WRITE_WAIT,
    MPUSM_READ,
    MPUSM_READ_WAIT,
};

uint8_t  mpu_statemachine = MPUSM_IDLE;
int16_t  mpu_dataRaw[7];
bool     mpu_new = false;

void mpu_task()
{
    switch (mpu_statemachine)
    {
        case MPUSM_IDLE:
            mpu_statemachine = MPUSM_WRITE;
        case MPUSM_WRITE:
            Wire.setOpMode(I2C_OP_MODE_ISR);
            Wire.beginTransmission(MPU6050_DEFAULT_ADDRESS);
            Wire.write(MPU6050_RA_ACCEL_XOUT_H);
            Wire.sendTransmission(I2C_NOSTOP);
            mpu_statemachine = MPUSM_WRITE_WAIT;
            break;
        case MPUSM_WRITE_WAIT:
            if (Wire.done())
            {
                mpu_statemachine = MPUSM_READ;
            }
            break;
        case MPUSM_READ:
            Wire.sendRequest(MPU6050_DEFAULT_ADDRESS, 14, I2C_STOP);
            mpu_statemachine = MPUSM_READ_WAIT;
            break;
        case MPUSM_READ_WAIT:
            if (Wire.available() >= 14 || Wire.done())
            {
                uint8_t i;
                for (i = 0; i < 7; i++)
                {
                    uint8_t hi, lo;
                    uint16_t x;
                    int16_t* xp = (int16_t*)&x;
                    hi = Wire.readByte();
                    lo = Wire.readByte();
                    x = (hi << 8) | lo;
                    mpu_dataRaw[i] = *xp;
                }
                mpu_new = true;
                mpu_statemachine = MPUSM_WRITE;
            }
            break;
    }
}

void mpu_test()
{
    uint32_t ts = 0;
    uint32_t now;
    while (1)
    {
        mpu_task();
        if (mpu_new != false)
        {
            mpu_new = false;
            now = millis();
            if ((now - ts) > 100)
            {
                ts = now;
                Serial.println(mpu_dataRaw[0], DEC);
            }
        }
    }
}

void mpu_writeReg(uint8_t regAdr, uint8_t val)
{
    Wire.setOpMode(I2C_OP_MODE_IMM);
    Wire.beginTransmission(MPU6050_DEFAULT_ADDRESS);
    Wire.write(regAdr);
    Wire.write(val);
    Wire.endTransmission(I2C_STOP);
}

uint8_t mpu_readReg(uint8_t regAdr)
{
    Wire.setOpMode(I2C_OP_MODE_IMM);
    Wire.beginTransmission(MPU6050_DEFAULT_ADDRESS);
    Wire.write(regAdr);
    Wire.endTransmission(I2C_NOSTOP);
    Wire.requestFrom(MPU6050_DEFAULT_ADDRESS, 1, I2C_STOP);
    while (Wire.available() <= 0)
    {
        // wait
    }
    return Wire.readByte();
}

void mpu_writeBits(uint8_t regAdr, uint8_t bitpos, uint8_t len, uint8_t data)
{
    uint8_t x = mpu_readReg(regAdr);

    uint8_t mask = ((1 << len) - 1) << (bitpos - len + 1);
    data <<= (bitpos - len + 1);      // shift data into correct position
    data &= mask;                     // zero all non-important bits in data
    x    &= ~(mask);                  // zero all important bits in existing byte
    x    |= data;                     // combine data with existing byte

    mpu_writeReg(regAdr, x);
}

void mpu_writeBit(uint8_t regAdr, uint8_t bitpos, bool set)
{
    uint8_t x = mpu_readReg(regAdr);

    if (set)
    {
        x |=  (1 << bitpos);
    }
    else
    {
        x &= ~(1 << bitpos);
    }

    mpu_writeReg(regAdr, x);
}

#define PIN_SDA 18
#define PIN_SCL 19

void mpu_busFreeCheck()
{
    pinMode(PIN_SDA, INPUT);
    pinMode(PIN_SCL, INPUT);
    if (digitalRead(PIN_SCL) == LOW || digitalRead(PIN_SDA) == LOW)
    {
        uint8_t tgl;
        pinMode(PIN_SCL, OUTPUT);
        for (tgl = 0; tgl < 9 && digitalRead(PIN_SDA) == LOW; tgl++)
        {
            digitalWrite(PIN_SCL, LOW);
            delayMicroseconds(5);
            digitalWrite(PIN_SCL, HIGH);
            delayMicroseconds(5);
        }
        pinMode(PIN_SDA, OUTPUT);
        pinMode(PIN_SCL, OUTPUT);
        digitalWrite(PIN_SDA, HIGH);
        digitalWrite(PIN_SCL, HIGH);
        delayMicroseconds(100);
    }
}
