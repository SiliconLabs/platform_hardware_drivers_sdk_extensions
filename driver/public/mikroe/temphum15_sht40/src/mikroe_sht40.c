/***************************************************************************//**
 * @file mikroe_sht40.c
 * @brief mikroe_sht40.c Source File
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
#include "mikroe_sht40.h"
#include "temphum15.h"

static temphum15_t temhum15;
static temphum15_cfg_t temhum15_cfg;

#define CHECK_PRECISION_MODE(x) ((mikroe_sht40_precision_mode_high == x)      \
                                 || (mikroe_sht40_precision_mode_medium == x) \
                                 || (mikroe_sht40_precision_mode_low == x))

sl_status_t mikroe_sht40_init(sl_i2cspm_t *i2cspm_instance)
{
  sl_status_t stt = SL_STATUS_INVALID_PARAMETER;

  if (NULL != i2cspm_instance) {
    temhum15.i2c.handle = i2cspm_instance;
    temphum15_cfg_setup(&temhum15_cfg);

    if (I2C_MASTER_SUCCESS == temphum15_init(&temhum15, &temhum15_cfg)) {
      stt = SL_STATUS_OK;
    } else {
      stt = SL_STATUS_FAIL;
    }
  }
  return stt;
}

sl_status_t mikroe_sht40_set_i2csmp_instance(sl_i2cspm_t *i2cspm_instance)
{
  sl_status_t stt = SL_STATUS_INVALID_PARAMETER;

  if (NULL != i2cspm_instance) {
    temhum15.i2c.handle = i2cspm_instance;
    stt = SL_STATUS_OK;
  }
  return stt;
}

sl_status_t mikroe_sht40_generic_write(uint8_t reg, uint8_t *tx_buf,
                                       uint8_t tx_len)
{
  sl_status_t stt = SL_STATUS_INVALID_PARAMETER;

  if ((NULL != tx_buf)) {
    if (I2C_MASTER_SUCCESS == temphum15_generic_write(&temhum15, reg,
                                                      tx_buf, tx_len)) {
      stt = SL_STATUS_OK;
    } else {
      stt = SL_STATUS_FAIL;
    }
  }
  return stt;
}

sl_status_t mikroe_sht40_generic_read(uint8_t reg, uint8_t *rx_buf,
                                      uint8_t rx_len)
{
  sl_status_t stt = SL_STATUS_INVALID_PARAMETER;

  if ((NULL != rx_buf)) {
    if (I2C_MASTER_SUCCESS == temphum15_generic_read(&temhum15, reg,
                                                     rx_buf, rx_len)) {
      stt = SL_STATUS_OK;
    } else {
      stt = SL_STATUS_FAIL;
    }
  }
  return stt;
}

sl_status_t mikroe_sht40_get_temp_and_hum(
  mikroe_sht40_precision_mode_e precision_mode,
  mikroe_sht40_measurement_data_t *measurement_data)
{
  sl_status_t stt = SL_STATUS_INVALID_PARAMETER;

  if ((NULL != measurement_data) && (CHECK_PRECISION_MODE(precision_mode))) {
    float temp_val, hum_val;
    if (I2C_MASTER_SUCCESS == temphum15_get_temp_and_hum(&temhum15,
                                                         precision_mode,
                                                         &temp_val,
                                                         &hum_val)) {
      measurement_data->temperature = temp_val;
      measurement_data->humidity = hum_val;

      stt = SL_STATUS_OK;
    } else {
      stt = SL_STATUS_FAIL;
    }
  }
  return stt;
}

sl_status_t mikroe_sht40_soft_reset(void)
{
  sl_status_t stt = SL_STATUS_FAIL;

  if (I2C_MASTER_SUCCESS == temphum15_soft_reset(&temhum15)) {
    stt = SL_STATUS_OK;
  }
  return stt;
}

sl_status_t mikroe_sht40_read_serial(uint32_t *serial_number)
{
  sl_status_t stt = SL_STATUS_INVALID_PARAMETER;

  if (NULL != serial_number) {
    if (I2C_MASTER_SUCCESS == temphum15_read_serial(&temhum15, serial_number)) {
      stt = SL_STATUS_OK;
    } else {
      stt = SL_STATUS_FAIL;
    }
  }
  return stt;
}
