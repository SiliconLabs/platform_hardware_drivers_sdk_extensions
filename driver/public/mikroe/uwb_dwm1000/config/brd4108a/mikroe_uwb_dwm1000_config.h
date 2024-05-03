/***************************************************************************//**
 * @file  dwm1000_config.h
 * @brief DWM1000 UWB config.
 * @version 0.0.1
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef DWM1000_CONFIG_H_
#define DWM1000_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio optional=true> DWM1000_RESET
// $[GPIO_DWM1000_RESET]
#define DWM1000_RESET_PORT                 gpioPortC
#define DWM1000_RESET_PIN                  8
// [GPIO_DWM1000_RESET]$

// <gpio optional=true> DWM1000_INT
// $[GPIO_DWM1000_INT]
#define DWM1000_INT_PORT                   gpioPortB
#define DWM1000_INT_PIN                    1
// [GPIO_DWM1000_INT]$

// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif
#endif /* DWM1000_CONFIG_H_ */
