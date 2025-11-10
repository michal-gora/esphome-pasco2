#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {

        class XensivPasCO2I2C : public sensor::Sensor, public i2c::I2CDevice, public PollingComponent
        {
        public:
            void setup() override;
            void update() override;
            void dump_config() override;

            void set_co2_sensor(sensor::Sensor *co2_sensor) { co2_sensor_ = co2_sensor; }

        protected:
            uint32_t test_value_;
            sensor::Sensor *co2_sensor_{nullptr};
            uint16_t version_{0};
        };
    }
}