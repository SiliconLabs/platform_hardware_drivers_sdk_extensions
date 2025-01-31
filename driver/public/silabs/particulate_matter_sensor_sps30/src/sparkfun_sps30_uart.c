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

#include "sparkfun_sps30_uart.h"
#include "sensirion_shdlc.h"
#include "sensirion_uart_hal.h"

#define SPS30_ADDR                       (0x00)
#define SPS30_CMD_START_MEASUREMENT      (0x00)
#define SPS30_CMD_STOP_MEASUREMENT       (0x01)
#define SPS30_SUBCMD_MEASUREMENT_START \
  { 0x01, 0x03 }
#define SPS30_CMD_READ_MEASUREMENT       (0x03)
#define SPS30_CMD_SLEEP                  (0x10)
#define SPS30_CMD_WAKE_UP                (0x11)
#define SPS30_CMD_FAN_CLEAN_INTV         (0x80)
#define SPS30_CMD_FAN_CLEAN_INTV_LEN     (5)
#define SPS30_SUBCMD_READ_FAN_CLEAN_INTV (0x00)
#define SPS30_CMD_START_FAN_CLEANING     (0x56)
#define SPS30_CMD_DEV_INFO               (0xd0)
#define SPS30_CMD_DEV_INFO_SUBCMD_GET_SERIAL \
  { 0x03 }
#define SPS30_CMD_READ_VERSION           (0xd1)
#define SPS30_CMD_RESET                  (0xd3)
#define SPS30_ERR_STATE(state) (SPS30_ERR_STATE_MASK | (state))

uint32_t sps30_init(mikroe_uart_handle_t uart_handle)
{
  return sensirion_uart_init(uart_handle);
}

sl_status_t sps30_probe(void)
{
  char serial[SPS30_MAX_SERIAL_LEN];
  // Try to wake up, but ignore failure if it is not in sleep mode
  (void)sps30_wake_up();
  sl_status_t ret = sps30_get_serial(serial);

  return ret;
}

sl_status_t sps30_get_serial(char *serial)
{
  if (NULL != serial) {
    struct sensirion_shdlc_rx_header header;
    uint8_t param_buf[] = SPS30_CMD_DEV_INFO_SUBCMD_GET_SERIAL;
    sensirion_shdlc_return_t ret;

    ret = sensirion_shdlc_xcv(SPS30_ADDR,
                              SPS30_CMD_DEV_INFO,
                              sizeof(param_buf),
                              param_buf,
                              SPS30_MAX_SERIAL_LEN,
                              &header,
                              (uint8_t *)serial);

    if (ret != SENSIRION_SHDLC_OK) {
      return SL_STATUS_FAIL;
    }

    if (header.state) {
      return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

sl_status_t sps30_start_measurement(void)
{
  struct sensirion_shdlc_rx_header header;
  uint8_t param_buf[] = SPS30_SUBCMD_MEASUREMENT_START;

  sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                     SPS30_CMD_START_MEASUREMENT,
                                                     sizeof(param_buf),
                                                     param_buf,
                                                     0,
                                                     &header,
                                                     (uint8_t *)NULL);
  if (stt == SENSIRION_SHDLC_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t sps30_stop_measurement(void)
{
  struct sensirion_shdlc_rx_header header;

  sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                     SPS30_CMD_STOP_MEASUREMENT,
                                                     0,
                                                     (uint8_t *)NULL,
                                                     0,
                                                     &header,
                                                     (uint8_t *)NULL);

  if (stt == SENSIRION_SHDLC_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t sps30_read_measurement(sps30_measurement_t *measurement)
{
  if (NULL != measurement) {
    struct sensirion_shdlc_rx_header header;
    uint8_t data[10][4];

    sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                       SPS30_CMD_READ_MEASUREMENT,
                                                       0,
                                                       (uint8_t *)NULL,
                                                       sizeof(data),
                                                       &header,
                                                       (uint8_t *)data);
    if (stt != SENSIRION_SHDLC_OK) {
      return SL_STATUS_FAIL;
    }

    if (header.data_len != sizeof(data)) {
      return SL_STATUS_FAIL;
    }

    measurement->mc_1p0 = sensirion_common_bytes_to_float(data[0]);
    measurement->mc_2p5 = sensirion_common_bytes_to_float(data[1]);
    measurement->mc_4p0 = sensirion_common_bytes_to_float(data[2]);
    measurement->mc_10p0 = sensirion_common_bytes_to_float(data[3]);
    measurement->nc_0p5 = sensirion_common_bytes_to_float(data[4]);
    measurement->nc_1p0 = sensirion_common_bytes_to_float(data[5]);
    measurement->nc_2p5 = sensirion_common_bytes_to_float(data[6]);
    measurement->nc_4p0 = sensirion_common_bytes_to_float(data[7]);
    measurement->nc_10p0 = sensirion_common_bytes_to_float(data[8]);
    measurement->typical_particle_size =
      sensirion_common_bytes_to_float(data[9]);

    if (header.state) {
      return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

sl_status_t sps30_sleep(void)
{
  struct sensirion_shdlc_rx_header header;
  sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                     SPS30_CMD_SLEEP,
                                                     0,
                                                     (uint8_t *)NULL,
                                                     0,
                                                     &header,
                                                     (uint8_t *)NULL);

  if (stt == SENSIRION_SHDLC_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t sps30_wake_up(void)
{
  struct sensirion_shdlc_rx_header header;
  sl_status_t ret;
  const uint8_t data = 0xFF;

  ret = sensirion_uart_tx(1, &data, NULL);
  if (ret) {
    return ret;
  }
  sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                     SPS30_CMD_WAKE_UP,
                                                     0,
                                                     (uint8_t *)NULL,
                                                     0,
                                                     &header,
                                                     (uint8_t *)NULL);
  if (stt == SENSIRION_SHDLC_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t sps30_get_fan_auto_cleaning_interval(uint32_t *interval_seconds)
{
  if (NULL != interval_seconds) {
    struct sensirion_shdlc_rx_header header;
    uint8_t tx_data[] = { SPS30_SUBCMD_READ_FAN_CLEAN_INTV };
    uint8_t data[4];

    sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(
      SPS30_ADDR, SPS30_CMD_FAN_CLEAN_INTV, sizeof(tx_data), tx_data,
      sizeof(*interval_seconds), &header, (uint8_t *)data);

    if (stt != SENSIRION_SHDLC_OK) {
      return SL_STATUS_FAIL;
    }

    *interval_seconds = sensirion_common_bytes_to_uint32_t(data);

    if (header.state) {
      return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

sl_status_t sps30_set_fan_auto_cleaning_interval(uint32_t interval_seconds)
{
  struct sensirion_shdlc_rx_header header;
  uint8_t cleaning_command[SPS30_CMD_FAN_CLEAN_INTV_LEN];

  cleaning_command[0] = SPS30_SUBCMD_READ_FAN_CLEAN_INTV;
  sensirion_common_uint32_t_to_bytes(interval_seconds, &cleaning_command[1]);

  sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                     SPS30_CMD_FAN_CLEAN_INTV,
                                                     sizeof(cleaning_command),
                                                     cleaning_command,
                                                     0,
                                                     &header,
                                                     (uint8_t *)NULL);
  if (stt == SENSIRION_SHDLC_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t sps30_get_fan_auto_cleaning_interval_days(uint8_t *interval_days)
{
  if (NULL != interval_days) {
    sl_status_t ret;
    uint32_t interval_seconds;

    ret = sps30_get_fan_auto_cleaning_interval(&interval_seconds);
    if (ret) {
      return ret;
    }

    *interval_days = interval_seconds / (24 * 60 * 60);
    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

sl_status_t sps30_set_fan_auto_cleaning_interval_days(uint8_t interval_days)
{
  return sps30_set_fan_auto_cleaning_interval((uint32_t)interval_days * 24
                                              * 60 * 60);
}

sl_status_t sps30_start_manual_fan_cleaning(void)
{
  struct sensirion_shdlc_rx_header header;

  sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                     SPS30_CMD_START_FAN_CLEANING,
                                                     0,
                                                     (uint8_t *)NULL,
                                                     0,
                                                     &header,
                                                     (uint8_t *)NULL);
  if (stt == SENSIRION_SHDLC_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t
sps30_read_version(sps30_version_information_t *version_information)
{
  if (NULL != version_information) {
    struct sensirion_shdlc_rx_header header;
    uint8_t data[7];

    sensirion_shdlc_return_t stt = sensirion_shdlc_xcv(SPS30_ADDR,
                                                       SPS30_CMD_READ_VERSION,
                                                       0,
                                                       (uint8_t *)NULL,
                                                       sizeof(data),
                                                       &header,
                                                       data);

    if (stt != SENSIRION_SHDLC_OK) {
      return SL_STATUS_FAIL;
    }

    if (header.data_len != sizeof(data)) {
      return SL_STATUS_FAIL;
    }

    if (header.state) {
      return SL_STATUS_FAIL;
    }

    version_information->firmware_major = data[0];
    version_information->firmware_minor = data[1];
    version_information->hardware_revision = data[3];
    version_information->shdlc_major = data[5];
    version_information->shdlc_minor = data[6];

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

sl_status_t sps30_reset(void)
{
  return sensirion_shdlc_tx(SPS30_ADDR, SPS30_CMD_RESET, 0, (uint8_t *)NULL);
}
