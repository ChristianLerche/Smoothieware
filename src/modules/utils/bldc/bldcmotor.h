#pragma once

#include "libs/Pin.h"
#include <cstdint>

class BLDCMotor {
public:
    BLDCMotor(Pin pwm, Pin dir, Pin fg, uint8_t pulses_per_rev);

    void init(uint32_t pwm_freq);

    // Direct control
    void set_speed(float duty);          // 0.0 – 1.0
    void set_direction(bool forward);

    // Position / movement
    void move_to_pulses(int32_t target, float duty);
    void update();                       // call periodically
    void stop();

    // Status
    bool is_busy() const;
    int32_t get_position_pulses() const;
    float get_position_revolutions() const;
    void reset_position();

private:
    static void fg_isr(void *arg);
    void handle_fg_pulse();

    Pin pwm_pin;
    Pin dir_pin;
    Pin fg_pin;

    uint8_t pulses_per_rev;

    volatile int32_t pulse_count;
    volatile bool direction_forward;

    // Move-to state
    volatile bool moving;
    volatile int32_t target_pulses;
};
