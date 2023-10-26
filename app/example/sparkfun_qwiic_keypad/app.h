/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef APP_H
#define APP_H

#include "sl_i2cspm_instances.h"
#include "printf.h"
#include "sparkfun_qwiic_keypad.h"

#ifdef SAPRKFUN_KEYPAD_INT_PIN_EN

#include "gpiointerrupt.h"

/***************************************************************************//**
 * Callback for the intertrupt.
 ******************************************************************************/
void app_sparkfun_buttonEvent_callback(const uint8_t pin);

#else

#define BUTTON_EVENT_CHECK_FREQUENCY_MS (uint32_t)300
#define BUTTON_READ_HANDLER_INIT        app_init_sleeptimer

/***************************************************************************//**
 * Initialize the sleep timer.
 ******************************************************************************/
void app_init_sleeptimer(void);

#endif //SAPRKFUN_KEYPAD_INT_PIN_EN

/***************************************************************************//**
 * Initialize the keypad.
 ******************************************************************************/
void app_init_keypad(void);

/***************************************************************************//**
 * This function handles the interfacing with the keypad.
 ******************************************************************************/
void app_handle_new_button(void);

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void);

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void);

#endif // APP_H