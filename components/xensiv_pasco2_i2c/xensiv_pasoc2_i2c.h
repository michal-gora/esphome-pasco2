#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {

        class XensivPasCO2I2C : public PollingComponent
        {
        public:
            // Constructor
            XensivPasCO2I2C(uint32_t update_interval = 60000);

            void setup() override;
            void update() override;
            void dump_config() override;

            void set_test_sensor(sensor::Sensor *test_sensor) { test_sensor_ = test_sensor; }

        protected:
            uint32_t test_value_;
            sensor::Sensor *test_sensor_{nullptr};
        };
    }