import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
)

CODEOWNERS = ["@goram"]
CONF_TEST_VALUE = "test_value"

example_component_ns = cg.esphome_ns.namespace("xensiv_pasco2_i2c")
ExampleComponent = example_component_ns.class_("XensivPasCO2I2C", cg.PollingComponent)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ExampleComponent),
    cv.Optional(CONF_TEST_VALUE): sensor.sensor_schema(
        state_class=STATE_CLASS_MEASUREMENT,
    ),
}).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    if test_value_config := config.get(CONF_TEST_VALUE):
        sens = await sensor.new_sensor(test_value_config)
        cg.add(var.set_test_sensor(sens))