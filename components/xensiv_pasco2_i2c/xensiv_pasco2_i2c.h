#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/gpio.h"
#include "xensiv_pas_gas_regs.h"
#include "xensiv_pas_gas_co2_regs.h"

namespace esphome
{
  namespace xensiv_pasco2_i2c
  {

    class XensivPasCO2I2C : public Component, public i2c::I2CDevice, public sensor::Sensor
    {
    public:
      void setup() override;
      void dump_config() override;
      void read_co2_ppm();

      void set_interrupt_pin(InternalGPIOPin *pin) { interrupt_pin_ = pin; }
      void set_sensor_rate_value(int16_t rate) { sensor_rate_ = rate; }
      void set_operation_mode(int mode) { operation_mode_ = static_cast<int8_t>(mode); ESP_LOGD("xensiv_pasco2_i2c", "Measurement mode set to %d", operation_mode_); }
      bool single_shot_measure_co2_ppm();
      
      protected:
      static void gpio_intr_(XensivPasCO2I2C *arg);
      static void setup_sensor_(XensivPasCO2I2C *arg);
      
      float co2_ppm_{0.0f};
      uint16_t version_{2};
      int16_t sensor_rate_{10}; // Default rate in seconds
      int8_t operation_mode_{0}; // (Default) 0 for continuous mode, 1 for single-shot
      bool set_operation_mode_();
      bool select_sensor_rate_();
      bool setup_interrupt_();

      InternalGPIOPin *interrupt_pin_{nullptr};
      volatile bool data_ready_{false};
    };

  } // namespace xensiv_pasco2_i2c
} // namespace esphome