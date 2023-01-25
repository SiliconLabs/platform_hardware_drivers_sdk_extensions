/***************************************************************************//**
 * @file
 * @brief Top level application functions
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
#include "mikroe_max6969.h"
#include "sl_spidrv_instances.h"
#include "sl_pwm_instances.h"
#include "sl_simple_timer.h"
#include "app_log.h"

static sl_simple_timer_t display_timer;

static void app_led_timer_handle(
  sl_simple_timer_t *timer, void *data);

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  if (mikroe_max6969_init(sl_spidrv_mikroe_handle,
                          &sl_pwm_mikroe) == SL_STATUS_OK) {
    app_log("led init ok\n");
  }

  sl_simple_timer_start(&display_timer, 1000, app_led_timer_handle, NULL, true);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

static void app_led_timer_handle(sl_simple_timer_t *timer, void *data)
{
  (void)timer;
  (void)data;

  static uint8_t cnt = 0;
  mikroe_max6969_display_number(cnt, MIKROE_UTM7SEGR_DOT_LEFT);
  cnt++;
  if (cnt >= 100) {
    cnt = 0;
  }
}