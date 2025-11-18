---
title: "Infineon Xensiv PAS CO2 Sensor"
description: "Instructions for setting up Infineon Xensiv PAS CO2 sensors with ESPHome"
---

The `xensiv_pasco2_i2c` component allows you to use Infineon's Xensiv PAS CO2 sensor with ESPHome. 
This photoacoustic spectroscopy-based CO2 sensor provides accurate carbon dioxide concentration 
measurements from 0 to 32,000 ppm. The [I²C Bus](/components/i2c) is required to be set up in your 
configuration for this sensor to work.

**Compatible Hardware:**
- [EVAL-CO2-5V-MINIBOARD](https://www.infineon.com/evaluation-board/EVAL-CO2-5V-MINIBOARD) (5V version)
- [EVAL-PASCO2-MINIBOARD](https://www.infineon.com/evaluation-board/EVAL-PASCO2-MINIBOARD) (12V version)

**Documentation:**
- [PAS CO2 5V Datasheet](https://www.infineon.com/document-promo/infineon-pasco2v15-datasheet-en_fcad9bf5-ca1e-4c2b-bf84-6041fe0219d8)
- [PAS CO2 12V Datasheet](https://www.infineon.com/document-promo/infineon-pasco2v01-datasheet-en_4fb77315-f724-408e-b491-a1702031c2c1)
- [5V Miniboard User Manual](https://www.infineon.com/document-promo/infineon-user-manual-eval-co2-5v-miniboard-usermanual-en_4635183f-758d-41ab-a6cb-88ff8d72f0bf)

```yaml
# Example configuration entry
sensor:
  - platform: xensiv_pasco2_i2c
    name: "CO2"
```

## Configuration Variables

- **name** (**Required**, string): The name for the CO2 sensor. All options from 
  [Sensor](/components/sensor).
- **address** (*Optional*, int): The I²C address of the sensor. Defaults to `0x28`. The sensor has a 
  fixed hardware address that cannot be changed.
- **interrupt_pin** (*Optional*, [Pin Schema](/guides/configuration-types#pin-schema)): The GPIO pin 
  connected to the sensor's INT pin. Required for continuous operation mode. The sensor uses an 
  active-low interrupt signal.
- **sensor_rate** (*Optional*, [Time](/guides/configuration-types#time)): The measurement interval 
  for continuous mode. Valid range: 5 seconds to 4905 seconds. Defaults to `10s`. Accepts formats 
  like `10s`, `1min`, `60s`.
- **operation_mode** (*Optional*, string): Sensor operation mode. One of `continuous`, `single_shot`. 
  Defaults to `continuous`.
- **id** (*Optional*, [ID](/guides/configuration-types#id)): Manually specify the ID used for code 
  generation.

## Operation Modes

### Continuous Mode (Default)

In continuous mode, the sensor automatically takes measurements at the specified `sensor_rate` and 
triggers an interrupt when new data is ready. This is the recommended mode for most applications.

Requirements:
- `interrupt_pin` must be configured
- `operation_mode: continuous`

### Single-Shot Mode

In single-shot mode, measurements are only taken when explicitly triggered. This mode is useful for 
battery-powered applications or when measurements are needed on-demand.

> [!NOTE]
> Each measurement may take up to about 2 seconds to complete.

## Hardware Differences

### 5V Version (EVAL-CO2-5V-MINIBOARD)

- Operating voltage: 5V
- Lower power consumption
- Recommended for embedded applications

### 12V Version (EVAL-PASCO2-MINIBOARD)

- Operating voltage: 12V
- Higher sensitivity
- Suitable for industrial applications

Both versions use the same I²C interface and are fully compatible with this component.

## Triggering Single-Shot Measurements

You can trigger a single-shot CO2 measurement using a lambda action:

```yaml
button:
  - platform: template
    name: "Measure CO2 Now"
    on_press:
      - lambda: |-
          id(co2_sensor).measure_now();
```

This is useful in `single_shot` operation mode or for triggering additional measurements in 
continuous mode.

## Advanced Example

```yaml
i2c:
  sda: GPIOXX
  scl: GPIOXX

sensor:
  - platform: xensiv_pasco2_i2c
    id: co2_sensor
    name: "CO2"
    address: 0x28
    interrupt_pin: GPIOXX
    sensor_rate: 60s
    operation_mode: continuous
    filters:
      - sliding_window_moving_average:
          window_size: 5
          send_every: 1

button:
  - platform: template
    name: "Measure CO2 Now"
    on_press:
      - lambda: |-
          id(co2_sensor).single_shot_measure_co2_ppm();
```

## Troubleshooting

### Sensor Not Responding

- Verify I²C address (default: `0x28`)
- Check I²C wiring (SDA, SCL, GND, VCC)
- Ensure correct voltage (5V or 12V depending on module)

### No Interrupt Triggering

- Verify interrupt pin is correctly connected
- Check that `operation_mode` is set to `continuous`
- Ensure `interrupt_pin` is configured in the YAML

### Inaccurate Readings

- Allow 2-3 minutes warm-up time after power-on
- Ensure adequate ventilation around the sensor
- Check that `sensor_rate` is appropriate for your application (minimum 5s, maximum 4905s)

## See Also

- [Sensor Filters](/components/sensor#sensor-filters)
- [I²C Bus Component](/components/i2c)
- [Infineon Xensiv PAS CO2 Product Page (5V)](https://www.infineon.com/evaluation-board/EVAL-CO2-5V-MINIBOARD)
- [Infineon Xensiv PAS CO2 Product Page (12V)](https://www.infineon.com/evaluation-board/EVAL-PASCO2-MINIBOARD)
- {{< apiref "xensiv_pasco2_i2c/xensiv_pasco2_i2c.h" "xensiv_pasco2_i2c.h" >}}

The `xensiv_pasco2_i2c` component allows you to use Infineon's Xensiv PAS CO2 sensor with ESPHome. This photoacoustic spectroscopy-based CO2 sensor provides accurate carbon dioxide concentration measurements from 0 to 32,000 ppm. The [I²C Bus](/components/i2c) is required to be set up in your configuration for this sensor to work.

**Compatible Hardware:**
- [EVAL-CO2-5V-MINIBOARD](https://www.infineon.com/evaluation-board/EVAL-CO2-5V-MINIBOARD) (5V version)
- [EVAL-PASCO2-MINIBOARD](https://www.infineon.com/evaluation-board/EVAL-PASCO2-MINIBOARD) (12V version)

**Documentation:**
- [PAS CO2 5V Datasheet](https://www.infineon.com/document-promo/infineon-pasco2v15-datasheet-en_fcad9bf5-ca1e-4c2b-bf84-6041fe0219d8)
- [PAS CO2 12V Datasheet] (https://www.infineon.com/document-promo/infineon-pasco2v01-datasheet-en_4fb77315-f724-408e-b491-a1702031c2c1)
- [5V Miniboard User Manual](https://www.infineon.com/document-promo/infineon-user-manual-eval-co2-5v-miniboard-usermanual-en_4635183f-758d-41ab-a6cb-88ff8d72f0bf)

```yaml
# Example configuration entry
i2c:
  sda: GPIO20
  scl: GPIO21
  scan: false

sensor:
  - platform: xensiv_pasco2_i2c
    id: co2_sensor
    name: "CO2"
    address: 0x28
    interrupt_pin: GPIO19
    sensor_rate: 60s
    operation_mode: continuous
```

## Configuration variables

- **name** (**Required**, string): The name for the CO2 sensor.
  All options from [Sensor](/components/sensor).

- **address** (*Optional*, int): Manually specify the I²C address of the sensor. Defaults to `0x28`.

- **interrupt_pin** (*Optional*, [Pin Schema](/guides/configuration-types#pin-schema)): The GPIO pin connected to the sensor's INT pin. Required for continuous operation mode. The sensor uses a high-active interrupt signal.

- **sensor_rate** (*Optional*, [Time](/guides/configuration-types#time)): The measurement interval for continuous mode. Valid range: 5 seconds to 4905 seconds. Defaults to `10s`. Accepts formats like `10s`, `1min`, `60s`.

- **operation_mode** (*Optional*, enum): Sensor operation mode. One of:
  - `continuous` (default) - Continuous measurements with interrupt notification (requires `interrupt_pin`)
  - `single_shot` - Manual trigger mode, measurements only when requested

- **id** (*Optional*, [ID](/guides/configuration-types#id)): Manually specify the ID used for code generation.

## Operation Modes

### Continuous Mode (Default)
In continuous mode, the sensor automatically takes measurements at the specified `sensor_rate` and triggers an interrupt when new data is ready. This is the recommended mode for most applications.

Requirements:
- `interrupt_pin` must be configured
- `operation_mode: continuous`

### Single-Shot Mode
In single-shot mode, measurements are only taken when explicitly triggered. This mode is useful for battery-powered applications or when measurements are needed on-demand.

See the **Actions** section below for triggering single-shot measurements.

> [!NOTE]
> Each measurement may take up to about 2 seconds to complete.

## Hardware Differences

### 5V Version (EVAL-CO2-5V-MINIBOARD)
- Operating voltage: 5V
- Lower power consumption
- Recommended for embedded applications

### 12V Version (EVAL-PASCO2-MINIBOARD)
- Operating voltage: 12V
- Higher sensitivity
- Suitable for industrial applications

Both versions use the same I²C interface and are fully compatible with this component.

## Actions

### Trigger Single-Shot Measurement

You can trigger a single-shot CO2 measurement using a lambda action:

```yaml
button:
  - platform: template
    name: "Measure CO2 Now"
    on_press:
      - lambda: |-
          id(co2_sensor).single_shot_measure_co2_ppm();
```

This is useful in `single_shot` operation mode or for triggering additional measurements in continuous mode.

## Complete Example

```yaml
# Example configuration for RP2040 MCU
i2c:
  sda: GPIO20
  scl: GPIO21
  scan: false

sensor:
  - platform: xensiv_pasco2_i2c
    id: co2_sensor
    name: "CO2"
    address: 0x28
    interrupt_pin: GPIO19
    sensor_rate: 60s
    operation_mode: continuous
    unit_of_measurement: "ppm"
    accuracy_decimals: 0
    device_class: carbon_dioxide
    state_class: measurement
    filters:
      - sliding_window_moving_average:
          window_size: 5
          send_every: 1

# Optional: Button for manual measurements
button:
  - platform: template
    name: "Measure CO2 Now"
    on_press:
      - lambda: |-
          id(co2_sensor).single_shot_measure_co2_ppm();
```

## Troubleshooting

### Sensor Not Responding
- Verify I²C address (default: `0x28`)
- Check I²C wiring (SDA, SCL, GND, VCC)
- Ensure correct voltage (5V or 12V depending on module)

### No Interrupt Triggering
- Verify interrupt pin is correctly connected
- Check that `operation_mode` is set to `continuous`
- Ensure correct `interrupt_pin` is configured in the YAML

### Inaccurate Readings
- Allow 2-3 minutes warm-up time after power-on
- Ensure adequate ventilation around the sensor
- Check that `sensor_rate` is appropriate for your application (minimum 5s, maximum 4905s)

## See Also

- [Sensor Filters](/components/sensor#sensor-filters)
- [I²C Bus Component](/components/i2c)
- [Infineon Xensiv PAS CO2 Product Page (5V)](https://www.infineon.com/evaluation-board/EVAL-CO2-5V-MINIBOARD)
- [Infineon Xensiv PAS CO2 Product Page (12V)](https://www.infineon.com/evaluation-board/EVAL-PASCO2-MINIBOARD)
- {{< apiref "xensiv_pasco2_i2c/xensiv_pasco2_i2c.h" "xensiv_pasco2_i2c.h" >}}