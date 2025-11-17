
/***********************************************************************************************//**
 * \file xensiv_pas_gas_co2_regs.h
 *
 * Description: This file contains the register definitions
 *              for interacting with the XENSIV™ PAS CO2 sensor.
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2025-2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************************************/

#ifndef XENSIV_PAS_GAS_CO2_REGS_H_
#define XENSIV_PAS_GAS_CO2_REGS_H_

#include "xensiv_pas_gas_regs.h"

/**
 * \addtogroup group_board_libs XENSIV™ PAS GAS CO2 sensor
 * \{
 */

// --------------------------SENS_STS Bitfields CO2 sensor only Register---------------------------//
#define XENSIV_PAS_GAS_CO2_REG_SENS_STS_PWM_DIS_ST_POS       (6U)                                                           /*!< SENS_STS: PWM_DIS_ST position (CO2 only) */
#define XENSIV_PAS_GAS_CO2_REG_SENS_STS_PWM_DIS_ST_MSK       (0x01U << XENSIV_PAS_GAS_CO2_REG_SENS_STS_PWM_DIS_ST_POS)      /*!< SENS_STS: PWM_DIS_ST mask (CO2 only) */

// --------------------------MEAS_CFG Bitfields CO2 sensor only Register---------------------------//
#define XENSIV_PAS_GAS_CO2_REG_MEAS_CFG_PWM_MODE_POS         (4U)                                                           /*!< MEAS_CFG: PWM_MODE position (CO2 only) */
#define XENSIV_PAS_GAS_CO2_REG_MEAS_CFG_PWM_MODE_MSK         (0x01U << XENSIV_PAS_GAS_CO2_REG_MEAS_CFG_PWM_MODE_POS)        /*!< MEAS_CFG: PWM_MODE mask (CO2 only) */
#define XENSIV_PAS_GAS_CO2_REG_MEAS_CFG_PWM_OUTEN_POS        (5U)                                                           /*!< MEAS_CFG: PWM_OUTEN position (CO2 only) */
#define XENSIV_PAS_GAS_CO2_REG_MEAS_CFG_PWM_OUTEN_MSK        (0x01U << XENSIV_PAS_GAS_CO2_REG_MEAS_CFG_PWM_OUTEN_POS)       /*!< MEAS_CFG: PWM_OUTEN mask (CO2 only) */

/** \} group_board_libs */

#endif /* XENSIV_PAS_GAS_CO2_REGS_H_ */