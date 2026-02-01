#include "BLDCMotor.h"
#include "mbed.h"
#include <cmath>

static const int32_t STOP_WINDOW_PULSES = 1;

BLDCMotor::BLDCMotor(Pin pwm, Pin dir, Pin fg, uint8_t ppr)
    : pwm_pin(pwm),
      dir_pin(dir),
      fg_pin(fg),
      pulses_per_rev(ppr),
      pulse_count(0),
      direction_forward(true),
      moving(false),
      target_pulses(0)
{
}

void BLDCMotor::init(uint32_t pwm_freq)
{
    pwm_pin.as_pwm();
    pwm_pin.set_pwm_frequency(pwm_freq);
    pwm_pin.set(0.0f);

    dir_pin.as_output();
    dir_pin.set(true);

    fg_pin.as_input();
    fg_pin.pull_up();
    fg_pin.attach_interrupt(&BLDCMotor::fg_isr, this, Pin::RISING);
}

void BLDCMotor::set_speed(float duty)
{
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;
    pwm_pin.set(duty);
}

void BLDCMotor::set_direction(bool forward)
{
    direction_forward = forward;
    dir_pin.set(forward);
}

void BLDCMotor::move_to_pulses(int32_t target, float duty)
{
    target_pulses = target;
    moving = true;

    int32_t delta = target_pulses - pulse_count;
    set_direction(delta >= 0);
    set_speed(duty);
}

void BLDCMotor::update()
{
    if (!moving) return;

    int32_t delta = target_pulses - pulse_count;

    if (std::abs(delta) <= STOP_WINDOW_PULSES) {
        stop();
    }
}

void BLDCMotor::stop()
{
    set_speed(0.0f);
    moving = false;
}

bool BLDCMotor::is_busy() const
{
    return moving;
}

int32_t BLDCMotor::get_position_pulses() const
{
    return pulse_count;
}

float BLDCMotor::get_position_revolutions() const
{
    return (float)pulse_count / (float)pulses_per_rev;
}

void BLDCMotor::reset_position()
{
    pulse_count = 0;
}

void BLDCMotor::fg_isr(void *arg)
{
    static_cast<BLDCMotor *>(arg)->handle_fg_pulse();
}

void BLDCMotor::handle_fg_pulse()
{
    if (direction_forward) {
        pulse_count++;
    } else {
        pulse_count--;
    }
}
