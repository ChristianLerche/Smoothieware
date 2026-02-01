#pragma once

#include "libs/Module.h"
#include "gcode/Gcode.h"
#include "BLDCMotor.h"

class BLDCModule : public Module {
public:
    BLDCModule();

    void on_module_loaded() override;
    void on_gcode_received(void *argument) override;
    void on_slow_tick(void *argument) override;

private:
    BLDCMotor *motor;

    bool enabled;
    float max_rpm;
};