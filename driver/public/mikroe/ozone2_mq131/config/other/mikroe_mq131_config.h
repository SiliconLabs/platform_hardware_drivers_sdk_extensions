/***************************************************************************//**
 * @file mikroe_mq131_config.h
 * @brief Mikroe MQ131 Configuration
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the
 * specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/
#ifndef MIKROE_MQ131_CONFIG_H
#define MIKROE_MQ131_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SLI_SI917
#include "em_gpio.h"
#endif

// <<< Use Configuration Wizard in Context Menu >>>
// <h> Ozone 2 Click ADC Select

// <o MIKROE_OZONE2_ADC_SEL> Select ADC module
// <i>
// <0=> Internal ADC
// <1=> External ADC
#define MIKROE_OZONE2_ADC_SEL                      1

// </h>

// <h> Ozone 2 Click SPI Configuration

// <e> Ozone 2 Click SPI UC Configuration
// <i> Enable: Peripheral configuration is taken straight from the configuration set in the universal configuration (UC).
// <i> Disable: If the application demands it to be modified during runtime, use the default API to modify the peripheral configuration.
// <i> Default: 0
#define MIKROE_OZONE2_CLICK_SPI_UC                 0

// <o MIKROE_OZONE2_CLICK_SPI_UC_BITRATE> Bit Rate (Bits/Second) <1-116000000>
// <i> Default: 10000000
#define MIKROE_OZONE2_CLICK_SPI_UC_BITRATE         10000000

// </e>
// </h>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio optional=true> MIKROE_OZONE2_AN
// $[GPIO_MIKROE_OZONE2_AN]
#warning "MIKROE_OZONE2_AN is not configured"
// #define MIKROE_OZONE2_AN_PORT                    gpioPortB
// #define MIKROE_OZONE2_AN_PIN                     0
// [GPIO_MIKROE_OZONE2_AN]$

// <gpio optional=true> MIKROE_OZONE2_CS
// $[GPIO_MIKROE_OZONE2_CS]
#warning "MIKROE_OZONE2_CS is not configured"
// #define MIKROE_OZONE2_CS_PORT                    gpioPortC
// #define MIKROE_OZONE2_CS_PIN                     0
// [GPIO_MIKROE_OZONE2_CS]$

// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif // MIKROE_MQ131_CONFIG_H
