/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"
#include "sl_status.h"
#include "particulate_matter_sensor_sps30_i2c_config.h"

// Variable to store the i2cspm instance
static i2c_master_t s_i2c_handle;

/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked as IMPLEMENT.
 * Follow the function specification in the comments.
 */

/**
 * Select the current i2c bus by index.
 * All following i2c operations will be directed at that bus.
 *
 * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
 * bus)
 *
 * @param bus_idx   Bus index to select
 * @returns         SL_STATUS_OK on success, an error code otherwise
 */
sl_status_t sensirion_i2c_hal_select_bus(uint8_t bus_idx)
{
  /* TODO:IMPLEMENT or leave empty if all sensors are located on one single
   * bus
   */
  (void)bus_idx;
  return SL_STATUS_NOT_AVAILABLE;
}

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
sl_status_t sensirion_i2c_hal_init(mikroe_i2c_handle_t i2c_handle)
{
  uint32_t stt = SL_STATUS_INVALID_PARAMETER;

  if (NULL != i2c_handle) {
    s_i2c_handle.handle = i2c_handle;

    i2c_master_config_t i2c_cfg;
    i2c_master_configure_default(&i2c_cfg);
    i2c_cfg.speed = I2C_MASTER_SPEED_STANDARD;
    i2c_cfg.scl = HAL_PIN_NC;
    i2c_cfg.sda = HAL_PIN_NC;

#if (PM_SENSOR_SPS30_I2C_UC == 1)
    i2c_cfg.speed = PM_SENSOR_SPS30_I2C_SPEED_MODE;
#endif

    if (i2c_master_open(&s_i2c_handle, &i2c_cfg) == I2C_MASTER_ERROR) {
      return SL_STATUS_INITIALIZATION;
    }
    stt = SL_STATUS_OK;
  }

  return stt;
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
 */
void sensirion_i2c_hal_free(void)
{
  /* TODO:IMPLEMENT or leave empty if no resources need to be freed */
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns SL_STATUS_OK on success, error code otherwise
 */
sl_status_t sensirion_i2c_hal_read(uint8_t address, uint8_t *data,
                                   uint8_t count)
{
  if ((NULL != data) && (0 != count)) {
    i2c_master_set_slave_address(&s_i2c_handle, address);

    if (I2C_MASTER_SUCCESS != i2c_master_read(&s_i2c_handle,
                                              data,
                                              count)) {
      return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns SL_STATUS_OK on success, error code otherwise
 */
sl_status_t sensirion_i2c_hal_write(uint8_t address, const uint8_t *data,
                                    uint8_t count)
{
  if (NULL != data) {
    i2c_master_set_slave_address(&s_i2c_handle, address);

    if (I2C_MASTER_SUCCESS != i2c_master_write(&s_i2c_handle,
                                               data,
                                               count)) {
      return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_i2c_hal_sleep_usec(uint32_t useconds)
{
  uint32_t delay_ms = 1;

  if (useconds > 1000) {
    delay_ms = (useconds / 1000) + 1;
  }

  sl_sleeptimer_delay_millisecond(delay_ms);
}
