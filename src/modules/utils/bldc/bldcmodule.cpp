#include "BLDCModule.h"
#include "Kernel.h"
#include "libs/Config.h"

/*
Config: 
bldc.enable              true
bldc.pwm_pin             2.1
bldc.dir_pin             1.23
bldc.fg_pin              1.24
bldc.pwm_frequency       20000
bldc.pulses_per_rev      6
bldc.max_rpm             6000

G-code use-case:
M3 P1000 S3000     ; move to +1000 pulses
M3 P0 S1500        ; move back to zero
M3 P-500 S2000     ; move backwards
M5                 ; stop
M114               ; report position
*/



#define bldc_enable_checksum           CHECKSUM("bldc.enable")
#define bldc_pwm_pin_checksum          CHECKSUM("bldc.pwm_pin")
#define bldc_dir_pin_checksum          CHECKSUM("bldc.dir_pin")
#define bldc_fg_pin_checksum           CHECKSUM("bldc.fg_pin")
#define bldc_pwm_freq_checksum         CHECKSUM("bldc.pwm_frequency")
#define bldc_ppr_checksum              CHECKSUM("bldc.pulses_per_rev")
#define bldc_max_rpm_checksum          CHECKSUM("bldc.max_rpm")

BLDCModule::BLDCModule()
    : motor(nullptr),
      enabled(false),
      max_rpm(6000.0f)
{
}

void BLDCModule::on_module_loaded()
{
    Kernel *kernel = Kernel::getInstance();

    enabled = kernel->config->value(bldc_enable_checksum)
                  ->by_default(false)->as_bool();
    if (!enabled) return;

    Pin pwm_pin = kernel->config->value(bldc_pwm_pin_checksum)->as_pin();
    Pin dir_pin = kernel->config->value(bldc_dir_pin_checksum)->as_pin();
    Pin fg_pin  = kernel->config->value(bldc_fg_pin_checksum)->as_pin();

    uint32_t pwm_freq =
        kernel->config->value(bldc_pwm_freq_checksum)
        ->by_default(20000)->as_number();

    uint8_t ppr =
        kernel->config->value(bldc_ppr_checksum)
        ->by_default(6)->as_number();

    max_rpm =
        kernel->config->value(bldc_max_rpm_checksum)
        ->by_default(6000)->as_float();

    motor = new BLDCMotor(pwm_pin, dir_pin, fg_pin, ppr);
    motor->init(pwm_freq);

    kernel->register_for_event(ON_GCODE_RECEIVED, this);
    kernel->slow_ticker->attach(50, this, &BLDCModule::on_slow_tick);
}

void BLDCModule::on_slow_tick(void *)
{
    if (motor) motor->update();
}

void BLDCModule::on_gcode_received(void *argument)
{
    if (!enabled || motor == nullptr) return;

    Gcode *gcode = static_cast<Gcode *>(argument);

    // M3 P<pos> S<rpm>  ? move to position
    if (gcode->has_m && gcode->m == 3 && gcode->has_p) {

        int32_t target = (int32_t)gcode->p;
        float rpm = gcode->has_s ? gcode->s : max_rpm;

        float duty = rpm / max_rpm;
        if (duty > 1.0f) duty = 1.0f;

        motor->move_to_pulses(target, duty);
        gcode->mark_as_taken();
    }

    // M5 – stop immediately
    else if (gcode->has_m && gcode->m == 5) {
        motor->stop();
        gcode->mark_as_taken();
    }

    // M114 – report BLDC position
    else if (gcode->has_m && gcode->m == 114) {
        Kernel::getInstance()->serial->printf(
            "BLDC pulses: %ld  revs: %.4f\n",
            motor->get_position_pulses(),
            motor->get_position_revolutions()
        );
        gcode->mark_as_taken();
    }
}
