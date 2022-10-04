/***************************************************************************//**
 * @file sparkfun_type5.c
 * @brief Dosimeter source file.
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
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                               Includes
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include "sl_sleeptimer.h"
#include "sparkfun_type5_config.h"
#include "gpiointerrupt.h"
#include "sparkfun_type5.h"

// -----------------------------------------------------------------------------
//                               Macros
// -----------------------------------------------------------------------------

// Enable/Disable Interrupt
#define IntEnable()       (GPIO_IntEnable(SPARKFUN_TYPE5_SIG_PIN \
                                          | SPARKFUN_TYPE5_NS_PIN))
#define IntDisable()      (GPIO_IntDisable(SPARKFUN_TYPE5_SIG_PIN \
                                           | SPARKFUN_TYPE5_NS_PIN))

// Convert tick to millisecond
#define TICK_TO_MS(x)     (sl_sleeptimer_tick_to_ms(x))

// -----------------------------------------------------------------------------
//                       Local Variables
// -----------------------------------------------------------------------------

// Callback function for noise and radiation detection
static sparkfun_type5_callback_t sparkfun_type5_noise_callback = NULL;
static sparkfun_type5_callback_t sparkfun_type5_radiation_callback = NULL;

static bool initialized = false;

// Radiation and noise count
static uint32_t radiation_count = 0;
static uint32_t noise_count = 0;

static uint8_t history_length = 0;
static uint8_t history_index = 0;

static uint32_t count;
static uint32_t count_history[SPARKFUN_TYPE5_HISTORY_LENGTH];

static uint32_t prev_tick = 0;
static uint32_t previous_history_tick = 0;

static const double k_alpha = 53.032;
// -----------------------------------------------------------------------------
//                       Local Function
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *  Callback function for radiation interrupt.
 *
 * @details
 *  This function will be executed when there is an interrupt event on SIG_PIN.
 *
 ******************************************************************************/
static void sparkfun_type5_on_radiation_handler(uint8_t sig_pin)
{
  (void)sig_pin;
  radiation_count++;
}

/***************************************************************************//**
 * @brief
 *  Callback function for radiation interrupt.
 *
 * @details
 *  This function will be executed when there is an interrupt event on NS_PIN.
 *
 ******************************************************************************/
static void sparkfun_type5_on_noise_handler(uint8_t ns_pin)
{
  (void)ns_pin;
  noise_count++;
}

// -----------------------------------------------------------------------------
//                       Public Function
// -----------------------------------------------------------------------------

/**************************************************************************//**
 *  Initialize the Sparkfun dosimeter type5 sensor.
 *****************************************************************************/
void sparkfun_type5_init(void)
{
  if (initialized) {
    return;
  }

  initialized = true;
  GPIO_PinModeSet(SPARKFUN_TYPE5_SIG_PORT,
                  SPARKFUN_TYPE5_SIG_PIN,
                  gpioModeInputPull,
                  1);

  GPIO_PinModeSet(SPARKFUN_TYPE5_NS_PORT,
                  SPARKFUN_TYPE5_NS_PIN,
                  gpioModeInputPull,
                  1);

  GPIO_ExtIntConfig(SPARKFUN_TYPE5_SIG_PORT,
                    SPARKFUN_TYPE5_SIG_PIN,
                    SPARKFUN_TYPE5_SIG_PIN,
                    false,
                    true,
                    true);

  GPIO_ExtIntConfig(SPARKFUN_TYPE5_NS_PORT,
                    SPARKFUN_TYPE5_NS_PIN,
                    SPARKFUN_TYPE5_NS_PIN,
                    false,
                    true,
                    true);

  GPIOINT_CallbackRegister(SPARKFUN_TYPE5_SIG_PIN,
                           sparkfun_type5_on_radiation_handler);

  GPIOINT_CallbackRegister(SPARKFUN_TYPE5_NS_PIN,
                           sparkfun_type5_on_noise_handler);
}

/**************************************************************************//**
 *  Register callback function for noise detection.
 *****************************************************************************/
void sparkfun_type5_register_noise_callback(sparkfun_type5_callback_t callback)
{
  sparkfun_type5_noise_callback = callback;
}

/**************************************************************************//**
 *  Register callback function for radiation detection.
 *****************************************************************************/
void sparkfun_type5_register_radiation_callback(
  sparkfun_type5_callback_t callback)
{
  sparkfun_type5_radiation_callback = callback;
}

/**************************************************************************//**
 *  Process data.
 *****************************************************************************/
void sparkfun_type5_process(void)
{
  uint32_t curr_tick = sl_sleeptimer_get_tick_count();
  uint32_t curr_noise = 0;
  uint32_t curr_radiation = 0;

  if (TICK_TO_MS(curr_tick - prev_tick) >= SPARKFUN_TYPE5_PROCESS_PERIOD) {
    // Disable Interrupt to store the current radiation count.
    IntDisable();

    // Store radiation and noise count.
    curr_noise = noise_count;
    curr_radiation = radiation_count;

    // Reset radiation and noise count.
    radiation_count = 0;
    noise_count = 0;

    // Enable Interrupt to continue receiving signal.
    IntEnable();

    if (noise_count == 0) {
      // Store count into count history array.
      count_history[history_index] = curr_radiation;

      // Add current radiation count to total count.
      count += curr_radiation;
    }

    // Shift the value of history count array to the left for each 6 seconds.
    if (TICK_TO_MS(curr_tick - previous_history_tick)
        >= SPARKFUN_TYPE5_HISTORY_UNIT * 1000) {
      previous_history_tick += sl_sleeptimer_ms_to_tick(
        SPARKFUN_TYPE5_HISTORY_UNIT * 1000);
      history_index = (history_index + 1) % SPARKFUN_TYPE5_HISTORY_LENGTH;
      if (history_length < (SPARKFUN_TYPE5_HISTORY_LENGTH - 1)) {
        history_length++;
      }
      count -= count_history[history_index];
      count_history[history_index] = 0;
    }

    prev_tick = curr_tick;

    if ((curr_noise > 0) && sparkfun_type5_noise_callback) {
      sparkfun_type5_noise_callback();
    }
    if ((curr_radiation > 0) && sparkfun_type5_radiation_callback) {
      sparkfun_type5_radiation_callback();
    }
  }
}

/**************************************************************************//**
 *  Get radiation count during integration time.
 *****************************************************************************/
uint32_t sparkfun_type5_get_radiation_count(void)
{
  return count;
}

/**************************************************************************//**
 *  Get integration time.
 *****************************************************************************/
uint32_t sparkfun_type5_get_integration_time(void)
{
  return TICK_TO_MS(history_length * SPARKFUN_TYPE5_HISTORY_UNIT * 1000
                    + prev_tick - previous_history_tick);
}

/**************************************************************************//**
 *  Get the number of radiation count by minute.
 *****************************************************************************/
double sparkfun_type5_get_cpm(void)
{
  // cpm = uSv x alpha
  double min = sparkfun_type5_get_integration_time() / 60000.0;
  return (min > 0) ? sparkfun_type5_get_radiation_count() / min : 0;
}

/**************************************************************************//**
 *  Get the radiation dose.
 *****************************************************************************/
double sparkfun_type5_get_usvh(void)
{
  return sparkfun_type5_get_cpm() / k_alpha;
}

/**************************************************************************//**
 *  Get the radiation dose error.
 *****************************************************************************/
double sparkfun_type5_get_usvh_error(void)
{
  double min = sparkfun_type5_get_integration_time() / 60000.0;
  return (min > 0)
         ? sqrt(sparkfun_type5_get_radiation_count()) / min / k_alpha : 0;
}
