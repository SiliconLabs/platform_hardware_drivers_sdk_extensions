/***************************************************************************//**
 * @file drv_digital_in.h
 * @brief mikroSDK 2.0 Click Peripheral Drivers - Digital IN
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

#ifndef _DRV_DIGITAL_IN_H_
#define _DRV_DIGITAL_IN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "drv_name.h"
#include "hal_gpio.h"

typedef enum {
  DIGITAL_IN_SUCCESS = 0, DIGITAL_IN_UNSUPPORTED_PIN = (-1)
} digital_in_err_t;

typedef struct {
  hal_gpio_t pin;
} digital_in_t;

err_t digital_in_init(digital_in_t *in, pin_name_t name);
err_t digital_in_pullup_init(digital_in_t *in, pin_name_t name);
err_t digital_in_pulldown_init(digital_in_t *in, pin_name_t name);
uint8_t digital_in_read(digital_in_t *in);

#ifdef __cplusplus
}
#endif

#endif // _DRV_DIGITAL_IN_H_
// ------------------------------------------------------------------------- END
