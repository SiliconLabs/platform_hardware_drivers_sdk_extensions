// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.2
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "ui.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_button_instances.h"
#include "sl_si91x_led_config.h"
#include "sl_si91x_led_instances.h"
#include "sl_si91x_led.h"

#define ON                  1
#define OFF                 0

static volatile bool led0_status = OFF;
static volatile bool led1_status = OFF;

void btn0_pressed_event_cb(lv_event_t *e)
{
  (void)e;
  if (led0_status == ON) {
    sl_si91x_led_clear(led_led0.pin);
    led0_status = OFF;
  } else {
    sl_si91x_led_set(led_led0.pin);
    led0_status = ON;
  }
}

void btn1_pressed_event_cb(lv_event_t *e)
{
  (void)e;
  if (led1_status == ON) {
    sl_si91x_led_clear(led_led1.pin);
    led1_status = OFF;
  } else {
    sl_si91x_led_set(led_led1.pin);
    led1_status = ON;
  }
}

/***************************************************************************//**
 * Callback on button change.
 ******************************************************************************/
void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  if (state == BUTTON_PRESSED) {
    if (pin == button_btn0.pin) {
      if (lv_obj_has_state(ui_BTN0, LV_STATE_CHECKED)) {
        lv_obj_clear_state(ui_BTN0, LV_STATE_CHECKED);
        sl_si91x_led_clear(led_led0.pin);
        led0_status = OFF;
      } else {
        lv_obj_add_state(ui_BTN0, LV_STATE_CHECKED);
        sl_si91x_led_set(led_led0.pin);
        led0_status = ON;
      }
    } else if (pin == button_btn1.pin) {
      if (lv_obj_has_state(ui_BTN1, LV_STATE_CHECKED)) {
        lv_obj_clear_state(ui_BTN1, LV_STATE_CHECKED);
        sl_si91x_led_clear(led_led1.pin);
        led1_status = OFF;
      } else {
        lv_obj_add_state(ui_BTN1, LV_STATE_CHECKED);
        sl_si91x_led_set(led_led1.pin);
        led1_status = ON;
      }
    }
  }
}
