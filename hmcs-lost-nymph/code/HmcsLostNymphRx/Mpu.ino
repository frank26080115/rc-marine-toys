#include <MPU6050_t3.h>

void mpu_test()
{
    uint32_t samples = 0;
    uint32_t ts = 0;
    uint32_t now;
    while (1)
    {
        mpu_task();
        if (mpu_new != false)
        {
            mpu_new = false;
            samples++;
            now = millis();
            if ((now - ts) > 100)
            {
                ts = now;
                mpu_doMath();
                uint8_t di;
                Serial.printf("%7d: ", samples);
                for (di = 0; di < 3; di++)
                {
                    Serial.printf("%7d ", mpu_accels[di]);
                }
                Serial.println();
            }
        }
    }
}
