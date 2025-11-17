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

            // Perform full sensor reset (reset sticky bits, set to idle state)

            // Soft reset - use XENSIV_PASCO2_CMD_SOFT_RESET command
            if (this->write_byte(XENSIV_PASCO2_REG_SENS_RST, XENSIV_PASCO2_CMD_SOFT_RESET))
            {
                ESP_LOGCONFIG(TAG, "Sensor soft reset");
            }
            else
            {
                ESP_LOGW(TAG, "Failed to perform sensor soft reset");
            }
            
            // Run sensor initialization after a delay to avoid blocking the main thread
            this->set_timeout(3000, [this]() { XensivPasCO2I2C::setup_sensor_(this); });
        }

        void XensivPasCO2I2C::setup_sensor_(XensivPasCO2I2C *arg)
        {
            ESP_LOGCONFIG(TAG, "Starting sensor configuration...");
            
            if (!arg->select_sensor_rate_()) {
                ESP_LOGE(TAG, "Failed to set sensor rate");
            }
            if (!arg->setup_interrupt_()) {
                ESP_LOGE(TAG, "Failed to setup interrupt");
            }
            if (!arg->set_operation_mode_()) {
                ESP_LOGE(TAG, "Failed to set operation mode");
            }

            // Set up interrupt pin if configured
            if (arg->interrupt_pin_ != nullptr)
            {
                arg->interrupt_pin_->setup();
                // Input only - sensor has push-pull output (active low)
                arg->interrupt_pin_->pin_mode(gpio::FLAG_INPUT);
                arg->interrupt_pin_->attach_interrupt(
                    XensivPasCO2I2C::gpio_intr_,
                    arg,
                    gpio::INTERRUPT_FALLING_EDGE // Active low interrupt
                );
                ESP_LOGCONFIG(TAG, "  Interrupt pin configured (active low)");
            }
            
        }

        void XensivPasCO2I2C::gpio_intr_(XensivPasCO2I2C *arg)
        {
            // ISR - keep this minimal, no logging in ISR!
            arg->read_co2_ppm();
            ESP_LOGW(TAG, "Interrupt triggered - data ready");
            // Clear MEAS_STS INT_STS_CLR bit
            arg->write_byte(XENSIV_PASCO2_REG_MEAS_STS, XENSIV_PASCO2_REG_MEAS_STS_INT_STS_CLR_MSK);
        }

        bool XensivPasCO2I2C::setup_interrupt_()
        {
            // Configure interrupt: DRDY function (data ready), active low
            xensiv_pasco2_interrupt_config_t int_cfg;
            int_cfg.u = 0;
            int_cfg.b.int_func = XENSIV_PASCO2_INTERRUPT_FUNCTION_DRDY;
            int_cfg.b.int_typ = XENSIV_PASCO2_INTERRUPT_TYPE_LOW_ACTIVE;
            int_cfg.b.alarm_typ = XENSIV_PASCO2_ALARM_TYPE_LOW_TO_HIGH;

            if (this->write_byte(XENSIV_PASCO2_REG_INT_CFG, int_cfg.u))
            {
                ESP_LOGCONFIG(TAG, "Interrupt configured (active low, data ready)");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to configure interrupt");
                return false;
            }
        }

        bool XensivPasCO2I2C::set_operation_mode_()
        {
            if (this->operation_mode_ == XENSIV_PASCO2_OP_MODE_SINGLE)
            {
                return true; // Single-shot mode setup done using single_shot_measure_co2_ppm()
            }
            
            // Set to continuous measurement mode with automatic baseline offset compensation
            xensiv_pasco2_measurement_config_t meas_cfg;
            meas_cfg.u = 0;
            meas_cfg.b.op_mode = XENSIV_PASCO2_OP_MODE_CONTINUOUS;
            meas_cfg.b.boc_cfg = XENSIV_PASCO2_BOC_CFG_AUTOMATIC;
            meas_cfg.b.pwm_mode = XENSIV_PASCO2_PWM_MODE_SINGLE_PULSE;
            meas_cfg.b.pwm_outen = 0; // PWM output disabled

            bool success = this->write_byte(XENSIV_PASCO2_REG_MEAS_CFG, meas_cfg.u);

            if (success)
            {
                ESP_LOGCONFIG(TAG, "Sensor set to continuous measurement mode");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to continuous measurement mode");
                return false;
            }
        }

        bool XensivPasCO2I2C::select_sensor_rate_()
        {
            // Rate validation is done in sensor.py (5-4095 seconds)
            // Rate is stored as 12-bit value across MEAS_RATE_H and MEAS_RATE_L registers
            uint16_t rate = this->sensor_rate_;
            uint8_t rate_h = (rate >> 8) & 0xFF; // Upper byte
            uint8_t rate_l = rate & 0xFF;         // Lower byte

            ESP_LOGD(TAG, "Setting sensor rate to %d seconds", rate);

            if (!this->write_byte(XENSIV_PASCO2_REG_MEAS_RATE_H, rate_h)) {
                ESP_LOGE(TAG, "Failed to write MEAS_RATE_H");
                return false;
            }
            if (!this->write_byte(XENSIV_PASCO2_REG_MEAS_RATE_L, rate_l)) {
                ESP_LOGE(TAG, "Failed to write MEAS_RATE_L");
                return false;
            }

            ESP_LOGCONFIG(TAG, "Sensor rate set to %d seconds", rate);
            return true;
        }

        bool XensivPasCO2I2C::single_shot_measure_co2_ppm()
        {
            // Start single-shot measurement with automatic baseline offset compensation
            xensiv_pasco2_measurement_config_t meas_cfg;
            meas_cfg.u = 0;
            meas_cfg.b.op_mode = XENSIV_PASCO2_OP_MODE_SINGLE;
            meas_cfg.b.boc_cfg = XENSIV_PASCO2_BOC_CFG_AUTOMATIC;
            meas_cfg.b.pwm_mode = XENSIV_PASCO2_PWM_MODE_SINGLE_PULSE;
            meas_cfg.b.pwm_outen = 0; // PWM output disabled

            if (this->write_byte(XENSIV_PASCO2_REG_MEAS_CFG, meas_cfg.u))
            {
                ESP_LOGCONFIG(TAG, "Sensor set to single-shot measurement mode");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to single-shot measurement mode");
                return false;
            }
        }

        void XensivPasCO2I2C::read_co2_ppm()
        {
            ESP_LOGD(TAG, "Reading CO2 data...");

            uint8_t co2_ppm_val[2] = {0};
            xensiv_pasco2_meas_status_t meas_sts;

            // Only set to continuous mode if requested and not already in continuous mode
            if (this->operation_mode_ == XENSIV_PASCO2_OP_MODE_CONTINUOUS)
            {
                xensiv_pasco2_measurement_config_t current_meas_cfg;
                if (this->read_bytes(XENSIV_PASCO2_REG_MEAS_CFG, &current_meas_cfg.u, 1))
                {
                    // Check if we're in continuous mode with auto BOC
                    if (current_meas_cfg.b.op_mode != XENSIV_PASCO2_OP_MODE_CONTINUOUS ||
                        current_meas_cfg.b.boc_cfg != XENSIV_PASCO2_BOC_CFG_AUTOMATIC)
                    {
                        ESP_LOGW(TAG, "MEAS_CFG incorrect (mode: %d, boc: %d), reconfiguring", 
                                 current_meas_cfg.b.op_mode, current_meas_cfg.b.boc_cfg);
                        this->set_operation_mode_();
                    }
                }
            }

            // Check DRDY flag
            if (this->read_bytes(XENSIV_PASCO2_REG_MEAS_STS, &meas_sts.u, 1))
            {
                ESP_LOGD(TAG, "MEAS_STS: 0x%02X, DRDY: %s, INT_STS: %s, ALARM: %s", 
                         meas_sts.u, 
                         meas_sts.b.drdy ? "SET" : "NOT SET",
                         meas_sts.b.int_sts ? "SET" : "NOT SET",
                         meas_sts.b.alarm ? "SET" : "NOT SET");

                if (meas_sts.b.drdy || true)
                {
                    if (this->read_bytes(XENSIV_PASCO2_REG_CO2PPM_H, co2_ppm_val, 2))
                    {
                        // Read CO2PPM_H and CO2PPM_L
                        uint8_t co2ppm_h = co2_ppm_val[0];
                        uint8_t co2ppm_l = co2_ppm_val[1];
                        int16_t co2_raw = (static_cast<int16_t>(co2ppm_h) << 8) | co2ppm_l;
                        this->co2_ppm_ = static_cast<float>(co2_raw);
                        this->publish_state(this->co2_ppm_);
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Failed to read CO2 concentration registers");
                    }
                }
                else
                {
                    ESP_LOGD(TAG, "DRDY not set, CO2 value not ready");
                }
            }
            else
            {
                ESP_LOGW(TAG, "Failed to read MEAS_STS register for DRDY check");
            }
        }

        void XensivPasCO2I2C::dump_config()
        {
            ESP_LOGCONFIG(TAG, "XensivPasCO2I2C Component:");
            ESP_LOGCONFIG(TAG, "  Firmware Version: 0x%04X", this->version_);
            LOG_I2C_DEVICE(this);
            ESP_LOGCONFIG(TAG, "  Sensor Rate: %d seconds", this->sensor_rate_);
            if (this->interrupt_pin_ != nullptr) {
                LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
            }
            ESP_LOGCONFIG(TAG, "  Last CO2 Value: %.2f ppm", this->co2_ppm_);
        }

    }
}