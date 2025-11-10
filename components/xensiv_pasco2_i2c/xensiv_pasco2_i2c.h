#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
// #include "esphome/components/i2c/i2c.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {

        class XensivPasCO2I2C : public PollingComponent//, public i2c::I2CDevice
        {
        public:
            // Constructor
            XensivPasCO2I2C();

            void setup() override;
            void update() override;
            void dump_config() override;

            void set_test_sensor(sensor::Sensor *test_sensor) { test_sensor_ = test_sensor; }
            void set_co2_sensor(sensor::Sensor *co2_sensor) { co2_sensor_ = co2_sensor; }

        protected:
            uint32_t test_value_;
            sensor::Sensor *test_sensor_{nullptr};
            sensor::Sensor *co2_sensor_{nullptr};
        };
    }
}