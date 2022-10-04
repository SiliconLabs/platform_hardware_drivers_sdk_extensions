/***************************************************************************//**
 * @file drv_digital_out.h
 * @brief mikroSDK 2.0 Click Peripheral Drivers - Digital OUT
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

#ifndef _DRV_DIGITAL_OUT_H_
#define _DRV_DIGITAL_OUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "drv_name.h"
#include "hal_gpio.h"

typedef enum {
  DIGITAL_OUT_SUCCESS = 0, DIGITAL_OUT_UNSUPPORTED_PIN = (-1)
} digital_out_err_t;

typedef struct {
  hal_gpio_t pin;
} digital_out_t;

err_t digital_out_init(digital_out_t *out, pin_name_t name);
void digital_out_high(digital_out_t *out);
void digital_out_low(digital_out_t *out);
void digital_out_toggle(digital_out_t *out);
void digital_out_write(digital_out_t *out, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif // _DRV_DIGITAL_OUT_H_
// ------------------------------------------------------------------------- END
