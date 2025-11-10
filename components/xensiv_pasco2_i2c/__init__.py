import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_CARBON_DIOXIDE,
    UNIT_PARTS_PER_MILLION,
)

CODEOWNERS = ["@goram"]
CONF_TEST_VALUE = "test_value"
# CONF_CO2 = "co2"
DEPENDENCIES = ["sensor"]

xensiv_pasco2_i2c_ns = cg.esphome_ns.namespace("xensiv_pasco2_i2c")
XensivPasCO2I2C = xensiv_pasco2_i2c_ns.class_(
    "XensivPasCO2I2C", cg.PollingComponent #, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(XensivPasCO2I2C),
        cv.Optional(CONF_TEST_VALUE): sensor.sensor_schema(
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        # cv.Optional(CONF_CO2): sensor.sensor_schema(
        #     state_class=STATE_CLASS_MEASUREMENT,
        # ),
    })
    .extend(cv.polling_component_schema("60s"))
    # .extend(i2c.i2c_device_schema(0x28))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # await i2c.register_i2c_device(var, config)
    
    if test_value_config := config.get(CONF_TEST_VALUE):
        sens = await sensor.new_sensor(test_value_config)
        cg.add(var.set_test_sensor(sens))
    
    # if co2_config := config.get(CONF_CO2):
    #     sens = await sensor.new_sensor(co2_config)
    #     cg.add(var.set_co2_sensor(sens))