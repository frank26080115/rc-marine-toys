#define MOTOR_PWM_FREQ      19999
#define MOTOR_BRAKE_TIME_MS 500
#define PIN_MOTOR_LEFT_A
#define PIN_MOTOR_LEFT_B
#define PIN_MOTOR_LEFT_PWM
#define PIN_MOTOR_RIGHT_A
#define PIN_MOTOR_RIGHT_B
#define PIN_MOTOR_RIGHT_PWM
#define PIN_PUMP_A
#define PIN_PUMP_B
#define PIN_PUMP_PWM

void motors_init()
{
    pinMode(PIN_MOTOR_LEFT_A     , OUTPUT);
    pinMode(PIN_MOTOR_LEFT_B     , OUTPUT);
    pinMode(PIN_MOTOR_LEFT_PWM   , OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_A    , OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_B    , OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_PWM  , OUTPUT);
    pinMode(PIN_PUMP_A           , OUTPUT);
    pinMode(PIN_PUMP_B           , OUTPUT);
    pinMode(PIN_PUMP_PWM         , OUTPUT);
    analogWriteFrequency(PIN_MOTOR_LEFT_PWM , MOTOR_PWM_FREQ);
    analogWriteFrequency(PIN_MOTOR_RIGHT_PWM, MOTOR_PWM_FREQ);
    analogWriteFrequency(PIN_PUMP_PWM       , MOTOR_PWM_FREQ);
    failsafe();
    analogWrite(PIN_MOTOR_LEFT_PWM , 0);
    analogWrite(PIN_MOTOR_RIGHT_PWM, 0);
    analogWrite(PIN_PUMP_PWM       , 0);
    motor_buzzAll();
}

void motor_task()
{
    static uint32_t braketime[3] = {0, 0, 0,};
    uint32_t now = millis();

    motor_handle(0, PIN_MOTOR_LEFT_A , PIN_MOTOR_LEFT_B , PIN_MOTOR_LEFT_PWM , (uint32_t*)braketime, now);
    motor_handle(1, PIN_MOTOR_RIGHT_A, PIN_MOTOR_RIGHT_B, PIN_MOTOR_RIGHT_PWM, (uint32_t*)braketime, now);
    motor_handle(2, PIN_PUMP_A       , PIN_PUMP_B       , PIN_PUMP_PWM       , (uint32_t*)braketime, now);
}

void motor_handle(int idx, int pin_a, int pin_b, int pin_pwm, uint32_t* braketimes, uint32_t nowtime)
{
    int x = channel[idx];
    int d;
    if (x >= (PULSE_CENTER_US + RC_DEADZONE_US))
    {
        d = x - PULSE_CENTER_US - RC_DEADZONE_US;
        d /= 2;
        d = d > 255 ? 255 : d;
        digitalWrite(pin_a, HIGH);
        digitalWrite(pin_b, LOW);
        analogWrite(pin_pwm, d);
        braketimes[idx] = 0;
    }
    else if (x <= (PULSE_CENTER_US - RC_DEADZONE_US))
    {
        d = (PULSE_CENTER_US - RC_DEADZONE_US) - x;
        d /= 2;
        d = d > 255 ? 255 : d;
        digitalWrite(pin_a, LOW);
        digitalWrite(pin_b, HIGH);
        analogWrite(pin_pwm, d);
        braketimes[idx] = 0;
    }
    else
    {
        if (braketimes[idx] == 0)
        {
            braketimes[idx] = nowtime;
            digitalWrite(pin_a, LOW);
            digitalWrite(pin_b, LOW);
            analogWrite(pin_pwm, 255);
        }
        else if ((nowtime - braketimes[idx]) >= MOTOR_BRAKE_TIME_MS)
        {
            digitalWrite(pin_a, LOW);
            digitalWrite(pin_b, LOW);
            analogWrite(pin_pwm, 0);
        }
    }
}

void failsafe()
{
    channel[0] = PULSE_CENTER_US;
    channel[1] = PULSE_CENTER_US;
    channel[2] = PULSE_CENTER_US + (PULSE_MAXDIFF_US / 2); // surface
    motor_task();
}

void motor_buzz(int pin_a, int pin_b, int pin_pwm, uint32_t freq, uint32_t duration)
{
    uint32_t dly = 500000 / freq;
    uint32_t now = millis();
    analogWrite(pin_pwm, 255);
    while ((millis - now) < duration)
    {
        digitalWrite(pin_a, LOW);
        digitalWrite(pin_b, HIGH);
        delayMicroseconds(dly);
        digitalWrite(pin_b, LOW);
        digitalWrite(pin_a, HIGH);
        delayMicroseconds(dly);
    }
    digitalWrite(pin_a, LOW);
    digitalWrite(pin_b, LOW);
    analogWrite(pin_pwm, 0);
}

void motor_buzzAll()
{
    motor_buzz(PIN_MOTOR_LEFT_A , PIN_MOTOR_LEFT_B , PIN_MOTOR_LEFT_PWM , 440, 800);
    motor_buzz(PIN_MOTOR_RIGHT_A, PIN_MOTOR_RIGHT_B, PIN_MOTOR_RIGHT_PWM, 660, 800);
    motor_buzz(PIN_PUMP_A       , PIN_PUMP_B       , PIN_PUMP_PWM       , 880, 800);
}
