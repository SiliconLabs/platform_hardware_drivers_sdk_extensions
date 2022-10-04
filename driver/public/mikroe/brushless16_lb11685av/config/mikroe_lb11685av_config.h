/***************************************************************************//**
 * @file mikroe_lb11685av_config.h
 * @brief SCL lb11685av config file
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
#ifndef MIKROE_LB11685AV_CONFIG_H_
#define MIKROE_LB11685AV_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// <<< sl:start pin_tool >>>

// <gpio optional=true> BRUSHLESS_16_RD
// $[GPIO_BRUSHLESS_16_RD]
#define BRUSHLESS_16_RD_PORT                   gpioPortB
#define BRUSHLESS_16_RD_PIN                    3
// [GPIO_BRUSHLESS_16_RD]$

// <gpio optional=true> BRUSHLESS_16_FG
// $[GPIO_BRUSHLESS_16_FG]
#define BRUSHLESS_16_FG_PORT                   gpioPortB
#define BRUSHLESS_16_FG_PIN                    4
// [GPIO_BRUSHLESS_16_FG]$

// <gpio optional=true> BRUSHLESS_16_EN
// $[GPIO_BRUSHLESS_16_EN]
#define BRUSHLESS_16_EN_PORT                   gpioPortC
#define BRUSHLESS_16_EN_PIN                    3
// [GPIO_BRUSHLESS_16_EN]$

// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif /* MIKROE_LB11685AV_CONFIG_H_ */
