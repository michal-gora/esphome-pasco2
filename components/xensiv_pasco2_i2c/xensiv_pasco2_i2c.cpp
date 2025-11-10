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
            // Write 0x25 to MEAS_CFG register (0x04) to enable continuous measurement mode
            uint8_t meas_cfg_value = 0x26;
            if (this->write_byte(0x04, meas_cfg_value)) {
            ESP_LOGCONFIG(TAG, "Sensor set to continuous measurement mode (MEAS_CFG=0x26)");
            } else {
            ESP_LOGW(TAG, "Failed to set sensor to continuous measurement mode");
            }
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
            // TODO DIENSTAG: properly implement reading CO2 ppm from the sensor, remove reading drdy before reading for checking if data is ready

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
                // Check DRDY flag in MEAS_STS (address 0x07, bit 4)
                uint8_t meas_sts = debug_data[7];
                bool drdy = (meas_sts & (1 << 4)) != 0;
                ESP_LOGD(TAG, "MEAS_STS (0x07): 0x%02X, DRDY: %s", meas_sts, drdy ? "SET" : "NOT SET");

                // Log SENS_STS (address 0x08)
                uint8_t sens_sts = debug_data[8];
                ESP_LOGD(TAG, "SENS_STS (0x08): 0x%02X", sens_sts);
                // SENS_STS is a status register that provides sensor status flags, such as error conditions, calibration status, and other sensor-specific states. Refer to the sensor's datasheet for detailed bit definitions.

                if (drdy) {
                    // Read CO2PPM_H (0x05) and CO2PPM_L (0x06)
                    uint8_t co2ppm_h = debug_data[5];
                    uint8_t co2ppm_l = debug_data[6];
                    ESP_LOGD(TAG, "CO2PPM_H (0x05): 0x%02X", co2ppm_h);
                    ESP_LOGD(TAG, "CO2PPM_L (0x06): 0x%02X", co2ppm_l);

                    int16_t co2_raw = (static_cast<int16_t>(co2ppm_h) << 8) | co2ppm_l;
                    this->co2_ppm_ = static_cast<float>(co2_raw);
                } else {
                    ESP_LOGW(TAG, "DRDY not set, CO2 value not ready");
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