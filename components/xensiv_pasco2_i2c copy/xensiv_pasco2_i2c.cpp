#include "esphome/core/log.h"
#include "xensiv_pasco2_i2c.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {
        static const char *const TAG = "xensiv_pasco2_i2c.component";

        void XensivPasCO2I2C::dump_config()
        {
            ESP_LOGCONFIG(TAG, "XENSIV PASCO2 CO2 Sensor:");
            LOG_I2C_DEVICE(this);

            if (this->is_failed())
            {
                ESP_LOGE(TAG, "Communication with PASCO2 failed!");
            }

            if (this->co2_sensor_ != nullptr)
            {
                LOG_SENSOR("  ", "CO2 Sensor", this->co2_sensor_);
            }

            if (this->interrupt_pin_ != nullptr)
            {
                LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
            }
            else
            {
                ESP_LOGCONFIG(TAG, "  Interrupt Pin: Not configured");
            }

            ESP_LOGCONFIG(TAG, "  Operation Mode: %s",
                          this->continuous_operation_mode_ ? "Continuous" : "Single-shot");

            ESP_LOGCONFIG(TAG, "  Measurement Rate: %d seconds", this->sensor_rate_);

            if (this->pressure_ref_ > 0)
            {
                ESP_LOGCONFIG(TAG, "  Pressure Compensation: %d Pa (%.2f hPa)",
                              this->pressure_ref_, this->pressure_ref_ / 100.0f);
            }
            else
            {
                ESP_LOGCONFIG(TAG, "  Pressure Compensation: Using sensor default : 1015 hPa");
            }
        }

        bool XensivPasCO2I2C::read_byte(uint8_t reg, uint8_t &value)
        {
            return I2CDevice::read_byte(reg, &value);
        }

        bool XensivPasCO2I2C::read_bytes(uint8_t reg, uint8_t *data, size_t len)
        {
            return I2CDevice::read_bytes(reg, data, len);
        }

        bool XensivPasCO2I2C::write_byte(uint8_t reg, uint8_t value)
        {
            return I2CDevice::write_byte(reg, value);
        }
    }
}