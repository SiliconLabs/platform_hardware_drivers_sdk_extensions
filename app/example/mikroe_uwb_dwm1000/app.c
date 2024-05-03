/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
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
 * # Experimental Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/

// ------------------------------------------------------------------- INCLUDES

#include "mikroe_uwb_dwm1000.h"
#include "sl_spidrv_instances.h"
#include "app_log.h"
#include "app_assert.h"

#include "sl_simple_button_instances.h"

#include "sl_udelay.h"

// ------------------------------------------------------------------- DEFINES

#ifndef BUTTON_INSTANCE_0
#define BUTTON_INSTANCE_0   sl_button_btn0
#endif

// ------------------------------------------------------------------ VARIABLES

// Device mode setter - selects the module working mode
//   RX(receiver)/TX(transmitter)
static uint8_t dev_mode = MIKROE_DWM1000_MODE_TX;

// Transmit buffers
static uint8_t data_tx[7] = "Silabs";
static uint8_t transmit_cnt = 0;

static volatile bool tx_requested = false;

// Transmit length read var
static uint16_t temp_len = 0;

// Received data buffer
static uint8_t received_data[256] = { 0 };

// Dev_status var
static uint8_t dev_status = { 0 };

void app_init(void)
{
  sl_status_t sc;

  app_log("---- Application Init ----\r\n\n");

  if (mikroe_dwm1000_init(sl_spidrv_mikroe_handle) == SL_STATUS_OK) {
    app_log("---- UWB DWM1000 Click initialized successfully ----\r\n\n");
  } else {
    app_log("****** Error initializing the uwb device ******\r\n\n");
  }

  sl_udelay_wait(100000);

  mikroe_dwm1000_enable();

  sl_udelay_wait(100000);

  uint8_t id_raw[4] = { 0 };

  app_log("----- Reading Device TAG ----\r\n\n");
  mikroe_dwm1000_generic_read(MIKROE_DWM1000_REG_DEV_ID, &id_raw[0], 4);

  uint16_t tag_data = (( uint16_t ) id_raw[3] << 8) | id_raw[2];

  if (MIKROE_DWM1000_TAG != tag_data) {
    app_log(" ***** TAG ERROR ***** \r\n\n");
    sc = SL_STATUS_FAIL;
  } else {
    app_log("---- TAG OK ----\r\n\n");
    sc = SL_STATUS_OK;
  }

  app_assert_status(sc);

  mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_IDLE);

  // -----------------------------------------------------

  // Setting device mode and interrupt for that mode as well as clearing
  //   dev_status reg.
  mikroe_dwm1000_set_mode(dev_mode);
  mikroe_dwm1000_int_mask_set();
  mikroe_dwm1000_clear_status();

  // Setting device address and network ID
  app_log(" ------------------ \r\n");
  if (MIKROE_DWM1000_MODE_RX == dev_mode) {
    mikroe_dwm1000_set_dev_adr_n_network_id(6, 10);
    app_log(" -----RECEIVER----- \r\n");
  } else if (MIKROE_DWM1000_MODE_TX == dev_mode) {
    mikroe_dwm1000_set_dev_adr_n_network_id(5, 10);
    app_log(" ---TRANSMITTER--- \r\n");
  }
  app_log(" ------------------ \r\n\n");

  sl_udelay_wait(100000);

  // Setting default configuartion and tuning device for that configuration
  mikroe_dwm1000_use_smart_power(MIKROE_DWM1000_LOW);
  mikroe_dwm1000_frame_check(MIKROE_DWM1000_LOW);
  mikroe_dwm1000_frame_filter(MIKROE_DWM1000_LOW);
  mikroe_dwm1000_set_transmit_type(
    &MIKROE_DWM1000_TMODE_LONGDATA_RANGE_LOWPOWER[0]);
  mikroe_dwm1000_set_channel(MIKROE_DWM1000_CHANNEL_5);
  mikroe_dwm1000_tune_config();

  sl_udelay_wait(100000);

  if (MIKROE_DWM1000_MODE_RX == dev_mode) {
    // Setup for first receive
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_IDLE);
    mikroe_dwm1000_set_bit(MIKROE_DWM1000_REG_SYS_CFG, 29, MIKROE_DWM1000_HIGH);
    mikroe_dwm1000_set_bit(MIKROE_DWM1000_REG_SYS_CFG, 30, MIKROE_DWM1000_HIGH);
    mikroe_dwm1000_set_bit(MIKROE_DWM1000_REG_SYS_CFG, 31, MIKROE_DWM1000_HIGH);
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_RX);
    mikroe_dwm1000_clear_status();
    mikroe_dwm1000_start_transceiver();
  } else if (MIKROE_DWM1000_MODE_TX == dev_mode) {
    // Setup for first transmit
    app_log("Setting up first transmit. \r\n\n");
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_IDLE);
    mikroe_dwm1000_clear_status();
    mikroe_dwm1000_set_transmit(&data_tx[0], 7);
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_TX);
    mikroe_dwm1000_start_transceiver();
    transmit_cnt++;

    app_log(" - Transmit %u done - \r\n", transmit_cnt);
  }

  sl_udelay_wait(2000000);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  dev_status = mikroe_dwm1000_get_qint_pin_status();

  if (dev_status && (MIKROE_DWM1000_MODE_RX == dev_mode)) {
    // Reading transmitted data, logs it and resetting to receive mode
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_IDLE);
    mikroe_dwm1000_clear_status();
    temp_len = mikroe_dwm1000_get_transmit_len();
    mikroe_dwm1000_get_transmit(&received_data[0], temp_len);
    app_log("Received data: %s\r\n", received_data);
    app_log(" - Receive done - \r\n\n");
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_RX);
    mikroe_dwm1000_start_transceiver();
    mikroe_dwm1000_int_mask_set();
    mikroe_dwm1000_clear_status();
  }

  if (tx_requested && (MIKROE_DWM1000_MODE_TX == dev_mode)) {
    // Transmits data, resetting to receive mode
    tx_requested = false;
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_IDLE);
    mikroe_dwm1000_set_dev_adr_n_network_id(5, 10);
    mikroe_dwm1000_clear_status();
    mikroe_dwm1000_set_transmit(&data_tx[0], 7);
    mikroe_dwm1000_set_mode(MIKROE_DWM1000_MODE_TX);
    mikroe_dwm1000_start_transceiver();
    transmit_cnt++;

    app_log(" - Transmit %u done - \r\n", transmit_cnt);
  }
}

/******************************************************************************
 * Button callback, called if any button is pressed or released.
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&BUTTON_INSTANCE_0 == handle) {
      tx_requested = true;
    }
  }
}
