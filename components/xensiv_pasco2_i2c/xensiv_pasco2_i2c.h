#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/gpio.h"

namespace esphome
{
  namespace xensiv_pasco2_i2c
  {

    class XensivPasCO2I2C : public PollingComponent, public i2c::I2CDevice, public sensor::Sensor
    {
    public:
      void setup() override;
      void update() override;
      void dump_config() override;
      void read_co2_ppm();

      void set_interrupt_pin(InternalGPIOPin *pin) { interrupt_pin_ = pin; }
      void set_sensor_rate_value(int16_t rate) { sensor_rate = rate; }

    protected:
      static void gpio_intr_(XensivPasCO2I2C *arg);
      static void setup_sensor_(XensivPasCO2I2C *arg);

      float co2_ppm_{0.0f};
      uint16_t version_{2};
      int16_t sensor_rate_{10}; // Default rate in seconds
      bool set_continuous_operation_mode_with_interrupt_();
      bool single_shot_measure_co2_ppm_();
      bool set_sensor_rate_(int16_t);

      InternalGPIOPin *interrupt_pin_{nullptr};
      volatile bool data_ready_{false};
    };

  } // namespace xensiv_pasco2_i2c
} // namespace esphome