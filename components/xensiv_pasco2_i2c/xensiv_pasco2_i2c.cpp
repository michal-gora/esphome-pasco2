#include "esphome/core/log.h"
#include "xensiv_pasco2_i2c.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {
        static const char *const TAG = "xensiv_pasco2_i2c.component";

        void XensivPasCO2I2C::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up XensivPasCO2I2C component");
            // Initialize I2C communication with the sensor
            // TODO: Add sensor initialization code here
            this->co2_ppm_ = 42.0;
        }

        void XensivPasCO2I2C::update()
        {
            ESP_LOGD(TAG, "Updating XensivPasCO2I2C component");

            // Read CO2 value from I2C sensor
            //     TODO : Replace with actual I2C read operations
            //                Example placeholder : uint16_t co2_ppm = 0;
            this->read_co2_ppm();
            this->publish_state(this->co2_ppm_);
            ESP_LOGD(TAG, "Published CO2 value: %.2f ppm", this->co2_ppm_);

            // For now, publish a dummy value if sensor is configured
            // if (this->co2_sensor_ != nullptr) {
            //     // Placeholder: publish a test value (400-500 ppm range)
            //     float dummy_co2 = 400.0f + (test_value_ % 100);
            //     this->co2_sensor_->publish_state(dummy_co2);
            //     ESP_LOGD(TAG, "Published dummy CO2 value: %.2f ppm", dummy_co2);
            // } else {
            //     ESP_LOGW(TAG, "CO2 sensor not configured");
            // }
        }

        void XensivPasCO2I2C::read_co2_ppm()
        {
            // Placeholder for reading CO2 ppm from the sensor
            // Read 2 bytes (MSB and LSB) from registers 0x5 and 0x6 in a single I2C transaction
            uint8_t data[2] = {0};
            // Try reading 8 bytes starting from register 0x0 for debugging
            const size_t debug_bytes_to_read = 17; // Set this variable to change how many bytes to read
            uint8_t debug_data[debug_bytes_to_read] = {0};
            if (this->read_bytes(0x0, debug_data, debug_bytes_to_read)) {
                ESP_LOGD(TAG, "I2C raw data:");
                for (size_t i = 0; i < debug_bytes_to_read; ++i) {
                    ESP_LOGD(TAG, "  Byte %zu: 0x%02X", i, debug_data[i]);
                }
                // Optionally, still try to read CO2 value as before
                if (this->read_bytes(0x5, data, 2)) {
                    int16_t co2_raw = (static_cast<int16_t>(data[0]) << 8) | data[1];
                    this->co2_ppm_ = static_cast<float>(co2_raw);
                } else {
                    ESP_LOGW(TAG, "Failed to read CO2 value from sensor");
                    this->co2_ppm_ = NAN;
                }
            } else {
                ESP_LOGW(TAG, "Failed to read I2C debug data from sensor");
                this->co2_ppm_ = NAN;
            }
        }

        void XensivPasCO2I2C::dump_config()
        {
            ESP_LOGCONFIG(TAG, "XensivPasCO2I2C Component:");
            ESP_LOGCONFIG(TAG, " Firmware Version: 0x%04X", this->version_);
            LOG_I2C_DEVICE(this);
            LOG_UPDATE_INTERVAL(this);
            ESP_LOGCONFIG(TAG, "  Last CO2 Value: %.2f ppm", this->co2_ppm_);
        }

    }
}