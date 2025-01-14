/***************************************************************************//**
 * @file sparkfun_mlx90640.c
 * @brief SPARKFUN MLX90640 IR Array sensor source file.
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
//                                   Includes
// -----------------------------------------------------------------------------

#include <math.h>
#include "sl_sleeptimer.h"
#include "app_assert.h"

#include "sparkfun_mlx90640.h"
#include "sparkfun_mlx90640_config.h"

#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
#define app_printf(...) printf(__VA_ARGS__)
#endif

static paramsMLX90640 mlx90640;
static i2c_master_t mlx90640_i2c;

// -----------------------------------------------------------------------------
//                    Static Local function declarations
// -----------------------------------------------------------------------------
static void extract_VDD_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_PTAT_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_gain_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_Tgc_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_resolution_parameters(uint16_t *eeData,
                                          paramsMLX90640 *mlx90640);
static void extract_KsTa_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_KsTo_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_Alpha_parameters(uint16_t *eeData,
                                     paramsMLX90640 *mlx90640);
static void extract_offset_parameters(uint16_t *eeData,
                                      paramsMLX90640 *mlx90640);
static void extract_KtaPixel_parameters(uint16_t *eeData,
                                        paramsMLX90640 *mlx90640);
static void extract_KvPixel_parameters(uint16_t *eeData,
                                       paramsMLX90640 *mlx90640);
static void extract_CP_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static void extract_CILC_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
static int extract_deviating_pixels(uint16_t *eeData, paramsMLX90640 *mlx90640);
static int check_adjacent_pixels(uint16_t pix1, uint16_t pix2);
static float get_median(float *values, int n);
static int is_pixel_bad(uint16_t pixel, paramsMLX90640 *params);
static int validate_frame_data(uint16_t *frame_data);
static int validate_aux_data(uint16_t *aux_data);
static sl_status_t sparkfun_mlx90640_i2c_read(uint16_t startAddress,
                                              uint16_t nMemAddressRead,
                                              uint16_t *data);
static sl_status_t sparkfun_mlx90640_i2c_write(uint16_t writeAddress,
                                               uint16_t data);
static sl_status_t sparkfun_mlx90640_i2c_general_reset(void);

// -----------------------------------------------------------------------------
//                           Function definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Initializes the MLX90640 driver
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_init(mikroe_i2c_handle_t i2c_instance,
                                   uint8_t i2c_addr)
{
  i2c_master_config_t mlx90640_cfg;
  int status;
  uint8_t bad_pixel_count = 0;
  uint16_t eeMLX90640[832];

  // Configure default i2csmp instance
  mlx90640_i2c.handle = i2c_instance;

  i2c_master_configure_default(&mlx90640_cfg);
  mlx90640_cfg.addr = i2c_addr;

#if (SPARKFUN_MLX90640_I2C_UC == 1)
  mlx90640_cfg.speed = SPARKFUN_MLX90640_I2C_SPEED_MODE;
#endif

  if (i2c_master_open(&mlx90640_i2c, &mlx90640_cfg) == I2C_MASTER_ERROR) {
    return SL_STATUS_INITIALIZATION;
  }
  i2c_master_set_speed(&mlx90640_i2c, mlx90640_cfg.speed);
  i2c_master_set_timeout(&mlx90640_i2c, 0);

  status = sparkfun_mlx90640_dump_ee(eeMLX90640);

  if (status != SL_STATUS_OK) {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
    app_printf("\nFailed to load system parameters of MLX90640\n");
#endif
    return SL_STATUS_NOT_INITIALIZED;
  }

  bad_pixel_count = sparkfun_mlx90640_extract_parameters(eeMLX90640, &mlx90640);

  if (bad_pixel_count != 0) {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
    app_printf("\nNumber of pixel errors: %d\n", bad_pixel_count);
#endif
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Requests and provides an array of temperatures for all 768 pixels as float
 * values.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_image_array(float *pixel_array)
{
  if (pixel_array == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  float Ta;
  // Read both sub-pages
  for (uint8_t x = 0 ; x < 2 ; x++) {
    uint16_t mlx90640Frame[834] = { 0 };
    int status = sparkfun_mlx90640_get_frame_data(mlx90640Frame);
    if (status < 0) {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
      app_printf("GetFrame Error: %d", status);
#endif
      return SL_STATUS_FAIL;
    }

    sparkfun_mlx90640_get_ta(mlx90640Frame, &mlx90640, &Ta);
    // Reflected temperature based on the sensor ambient temperature
    float tr = Ta - SPARKFUN_TA_SHIFT;
    float emissivity = SPARKFUN_MLX90640_CONFIG_EMISSIVITY;

    sparkfun_mlx90640_calculate_to(mlx90640Frame,
                                   &mlx90640,
                                   emissivity,
                                   tr,
                                   pixel_array);
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Changes which I2C bus and slave address does the driver use
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_change_devices(mikroe_i2c_handle_t i2c_instance,
                                             uint8_t new_addr)
{
  if ((i2c_instance == NULL) || (new_addr == 0x00)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  mlx90640_i2c.handle = i2c_instance;
  mlx90640_i2c.config.addr = new_addr;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Change slave address to the value of "new_addr"
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_set_slave_addr(mikroe_i2c_handle_t i2c_instance,
                                             uint8_t new_addr)
{
  if ((i2c_instance == NULL) || (new_addr == 0x00)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = SL_STATUS_FAIL;
  uint16_t temp_address = 0x0000;

  // If currently used i2c address is the same as the new one, then there's no
  // need to change anything
  if (mlx90640_i2c.config.addr == new_addr) {
    status = SL_STATUS_OK;
  } else {
    status = sparkfun_mlx90640_i2c_read(0x240F, 1, &temp_address);
    if (status == SL_STATUS_TRANSMIT) {
      mlx90640_i2c.config.addr = new_addr;
      status = sparkfun_mlx90640_i2c_read(0x240F, 1, &temp_address);
      if (status == SL_STATUS_OK) {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
        app_printf("\nCurrent address is 0x%x\n", temp_address & 0xFF);
#endif
        return status;
      } else {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
        app_printf(
          "\nUnknown I2C address!\nPlease find out the current address of the device, then change the SPARKFUN_MLX90640_DEFAULT_I2C_ADDR macro to that address!\n");
#endif
        status = SL_STATUS_FAIL;
      }
    } else if (status == SL_STATUS_OK) {
      if (new_addr != 0x00) {
        // Erase the EEPROM entry used for the i2c address
        status = sparkfun_mlx90640_i2c_write(0x240F, 0x0000);
        app_assert_status(status);

        // 10ms delay makes sure the write cycle finished before writing the new
        // address
        sl_sleeptimer_delay_millisecond(10);

        // 0xBE is a reserved value in the EEPROM. 0xBExx shall be written,
        // where xx is the i2c address
        status = sparkfun_mlx90640_i2c_write(0x240F, (0xBE << 8) | new_addr);
        app_assert_status(status);

        // 10ms delay makes sure the write cycle finished before writing the new
        // address
        sl_sleeptimer_delay_millisecond(10);

        status = sparkfun_mlx90640_i2c_read(0x240F, 1, &temp_address);
        app_assert_status(status);

        if ((temp_address & 0xFF) == new_addr) {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
          app_printf("\nAddress change succesful\nPower-reset the device!\n");
#endif
        } else {
#if SPARKFUN_MLX90640_CONFIG_ENABLE_LOG
          app_printf("\nAddress change failed\n");
#endif
        }
        status = SL_STATUS_NOT_INITIALIZED;
      }
    }
  }
  return status;
}

/***************************************************************************//**
 * Requests and provides the contents of the EEPROM.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_dump_ee(uint16_t *eeData)
{
  if (eeData == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return sparkfun_mlx90640_i2c_read(0x2400, 832, eeData);
}

/***************************************************************************//**
 * Waits for a new data to be available with a given slave address
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_synch_frame(void)
{
  uint16_t dataReady = 0;
  uint16_t statusRegister;
  int error = SL_STATUS_FAIL;

  error = sparkfun_mlx90640_i2c_write(0x8000, 0x0030);
  if (error == SL_STATUS_FAIL) {
    return SL_STATUS_FAIL;
  }

  while (dataReady == 0) {
    error = sparkfun_mlx90640_i2c_read(0x8000, 1, &statusRegister);
    if (error != 0) {
      return error;
    }
    dataReady = statusRegister & 0x0008;
  }

  return 0;
}

/***************************************************************************//**
 * Uses the global reset command described in the I2C standard to trigger a
 * measurement.
 * !! Note that this function will reset all devices on the same I2C bus that
 * support this command. !!
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_trigger_measurement(void)
{
  int error = 1;
  uint16_t ctrlReg;

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &ctrlReg);

  if (error != 0) {
    return error;
  }

  ctrlReg |= 0x8000;
  error = sparkfun_mlx90640_i2c_write(0x800D, ctrlReg);

  if (error != 0) {
    return error;
  }

  error = sparkfun_mlx90640_i2c_general_reset();

  if (error != 0) {
    return error;
  }

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &ctrlReg);

  if (error != 0) {
    return error;
  }

  if ((ctrlReg & 0x8000) != 0) {
    return -9;
  }

  return 0;
}

/***************************************************************************//**
 * Reads all the necessary frame data from the device.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_frame_data(uint16_t *frameData)
{
  uint16_t dataReady = 0;
  uint16_t controlRegister1;
  uint16_t statusRegister;
  int error = 1;
  uint16_t data[64];
  uint8_t cnt = 0;

  if (frameData == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  while (dataReady == 0) {
    error = sparkfun_mlx90640_i2c_read(0x8000, 1, &statusRegister);
    if (error != 0) {
      return error;
    }
    dataReady = statusRegister & 0x0008;
  }

  error = sparkfun_mlx90640_i2c_write(0x8000, 0x0030);
  if (error == -1) {
    return error;
  }

  error = sparkfun_mlx90640_i2c_read(0x0400, 768, frameData);
  if (error != 0) {
    return error;
  }

  error = sparkfun_mlx90640_i2c_read(0x0700, 64, data);
  if (error != 0) {
    return error;
  }

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);
  frameData[832] = controlRegister1;
  frameData[833] = statusRegister & 0x0001;

  if (error != 0) {
    return error;
  }

  error = validate_aux_data(data);
  if (error == 0) {
    for (cnt = 0; cnt < 64; cnt++)
    {
      frameData[cnt + 768] = data[cnt];
    }
  }

  error = validate_frame_data(frameData);
  if (error != 0) {
    return error;
  }

  return frameData[833];
}

/***************************************************************************//**
 * Validates frame data
 ******************************************************************************/
static int validate_frame_data(uint16_t *frameData)
{
  uint8_t line = 0;

  if (frameData == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (int i = 0; i < 768; i += 32) {
    if ((frameData[i] == 0x7FFF) && (line % 2 == frameData[833])) {
      return -8;
    }
    line = line + 1;
  }

  return 0;
}

/***************************************************************************//**
 * Validates auxiliary data
 ******************************************************************************/
static int validate_aux_data(uint16_t *auxData)
{
  if (auxData == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (auxData[0] == 0x7FFF) {
    return -8;
  }

  for (int i = 8; i < 19; i++) {
    if (auxData[i] == 0x7FFF) {
      return -8;
    }
  }

  for (int i = 20; i < 23; i++) {
    if (auxData[i] == 0x7FFF) {
      return -8;
    }
  }

  for (int i = 24; i < 33; i++) {
    if (auxData[i] == 0x7FFF) {
      return -8;
    }
  }

  for (int i = 40; i < 51; i++) {
    if (auxData[i] == 0x7FFF) {
      return -8;
    }
  }

  for (int i = 52; i < 55; i++) {
    if (auxData[i] == 0x7FFF) {
      return -8;
    }
  }

  for (int i = 56; i < 64; i++) {
    if (auxData[i] == 0x7FFF) {
      return -8;
    }
  }

  return 0;
}

/***************************************************************************//**
 * Extracts the parameters from a given EEPROM data array and stores values
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_extract_parameters(uint16_t *eeData,
                                                 paramsMLX90640 *mlx90640)
{
  int error = 0;

  if ((eeData == NULL) || (mlx90640 == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  extract_VDD_parameters(eeData, mlx90640);
  extract_PTAT_parameters(eeData, mlx90640);
  extract_gain_parameters(eeData, mlx90640);
  extract_Tgc_parameters(eeData, mlx90640);
  extract_resolution_parameters(eeData, mlx90640);
  extract_KsTa_parameters(eeData, mlx90640);
  extract_KsTo_parameters(eeData, mlx90640);
  extract_CP_parameters(eeData, mlx90640);
  extract_Alpha_parameters(eeData, mlx90640);
  extract_offset_parameters(eeData, mlx90640);
  extract_KtaPixel_parameters(eeData, mlx90640);
  extract_KvPixel_parameters(eeData, mlx90640);
  extract_CILC_parameters(eeData, mlx90640);
  error = extract_deviating_pixels(eeData, mlx90640);

  return error;
}

/***************************************************************************//**
 * Writes the desired resolution value in order to change the current resolution
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_set_resolution(uint8_t resolution)
{
  uint16_t controlRegister1;
  int value;
  int error;

  value = (resolution & 0x03) << 10;

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);

  if (error == 0) {
    value = (controlRegister1 & 0xF3FF) | value;
    error = sparkfun_mlx90640_i2c_write(0x800D, value);
  }

  return error;
}

/***************************************************************************//**
 * Provides the current resolution.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_cur_resolution(uint16_t *currentResolution)
{
  uint16_t controlRegister1;
  int resolutionRAM;
  int error;

  if (currentResolution == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);
  if (error != 0) {
    return SL_STATUS_FAIL;
  }
  resolutionRAM = (controlRegister1 & 0x0C00) >> 10;

  *currentResolution = resolutionRAM;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Writes the desired refresh rate value in order to change the current refresh
 * rate
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_set_refresh_rate(uint8_t refreshRate)
{
  uint16_t controlRegister1;
  int value;
  int error;

  value = (refreshRate & 0x07) << 7;

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);
  if (error == 0) {
    value = (controlRegister1 & 0xFC7F) | value;
    error = sparkfun_mlx90640_i2c_write(0x800D, value);
  }

  return error;
}

/***************************************************************************//**
 * Provides the current refresh rate.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_refresh_rate(uint16_t *current_rate)
{
  uint16_t controlRegister1;
  int refreshRate;
  int error;

  if (current_rate == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);
  if (error != 0) {
    return SL_STATUS_FAIL;
  }
  refreshRate = (controlRegister1 & 0x0380) >> 7;

  *current_rate = refreshRate;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets device to interleaved mode
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_set_interleaved_mode(void)
{
  uint16_t controlRegister1;
  int value;
  int error;

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);

  if (error == 0) {
    value = (controlRegister1 & 0xEFFF);
    error = sparkfun_mlx90640_i2c_write(0x800D, value);
  }

  return error;
}

/***************************************************************************//**
 * Sets device to chess mode
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_set_chess_mode(void)
{
  uint16_t controlRegister1;
  int value;
  int error;

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);

  if (error == 0) {
    value = (controlRegister1 | 0x1000);
    error = sparkfun_mlx90640_i2c_write(0x800D, value);
  }

  return error;
}

/***************************************************************************//**
 * Provides the current working mode.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_cur_mode(uint8_t *cur_mode)
{
  uint16_t controlRegister1;
  int modeRAM;
  int error;

  if (cur_mode == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  error = sparkfun_mlx90640_i2c_read(0x800D, 1, &controlRegister1);
  if (error != 0) {
    return error;
  }
  modeRAM = (controlRegister1 & 0x1000) >> 12;

  *cur_mode = modeRAM;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Calculates the object temperatures for all 768 pixel.
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_calculate_to(uint16_t *frameData,
                                           const paramsMLX90640 *params,
                                           float emissivity,
                                           float tr,
                                           float *result)
{
  float vdd;
  float ta;
  float ta4;
  float tr4;
  float taTr;
  float gain;
  float irDataCP[2];
  float irData;
  float alphaCompensated;
  uint8_t mode;
  int8_t ilPattern;
  int8_t chessPattern;
  int8_t pattern;
  int8_t conversionPattern;
  float Sx;
  float To;
  float alphaCorrR[4];
  int8_t range;
  uint16_t subPage;
  float ktaScale;
  float kvScale;
  float alphaScale;
  float kta;
  float kv;

  if ((frameData == NULL) || (params == NULL) || (result == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  subPage = frameData[833];
  sparkfun_mlx90640_get_vdd(frameData, params, &vdd);
  sparkfun_mlx90640_get_ta(frameData, params, &ta);

  ta4 = (ta + 273.15);
  ta4 = ta4 * ta4;
  ta4 = ta4 * ta4;
  tr4 = (tr + 273.15);
  tr4 = tr4 * tr4;
  tr4 = tr4 * tr4;
  taTr = tr4 - (tr4 - ta4) / emissivity;

  ktaScale = pow(2, (double)params->ktaScale);
  kvScale = pow(2, (double)params->kvScale);
  alphaScale = pow(2, (double)params->alphaScale);

  alphaCorrR[0] = 1 / (1 + params->ksTo[0] * 40);
  alphaCorrR[1] = 1;
  alphaCorrR[2] = (1 + params->ksTo[1] * params->ct[2]);
  alphaCorrR[3] = alphaCorrR[2]
                  * (1 + params->ksTo[2] * (params->ct[3] - params->ct[2]));

// ------------------------- Gain calculation-----------------------------------
  gain = frameData[778];
  if (gain > 32767) {
    gain = gain - 65536;
  }
  gain = params->gainEE / gain;

// ------------------------- To calculation-------------------------------------
  mode = (frameData[832] & 0x1000) >> 5;

  irDataCP[0] = frameData[776];
  irDataCP[1] = frameData[808];
  for ( int i = 0; i < 2; i++) {
    if (irDataCP[i] > 32767) {
      irDataCP[i] = irDataCP[i] - 65536;
    }
    irDataCP[i] = irDataCP[i] * gain;
  }
  irDataCP[0] = irDataCP[0] - params->cpOffset[0]
                * (1 + params->cpKta * (ta - 25))
                * (1 + params->cpKv * (vdd - 3.3));
  if (mode == params->calibrationModeEE) {
    irDataCP[1] = irDataCP[1] - params->cpOffset[1]
                  * (1 + params->cpKta * (ta - 25))
                  * (1 + params->cpKv * (vdd - 3.3));
  } else {
    irDataCP[1] = irDataCP[1] - (params->cpOffset[1] + params->ilChessC[0])
                  * (1 + params->cpKta * (ta - 25))
                  * (1 + params->cpKv * (vdd - 3.3));
  }

  for (int pixelNumber = 0; pixelNumber < 768; pixelNumber++) {
    ilPattern = pixelNumber / 32 - (pixelNumber / 64) * 2;
    chessPattern = ilPattern ^ (pixelNumber - (pixelNumber / 2) * 2);
    conversionPattern =
      ((pixelNumber + 2) / 4 - (pixelNumber + 3) / 4 + (pixelNumber + 1) / 4
       - pixelNumber / 4) * (1 - 2 * ilPattern);

    if (mode == 0) {
      pattern = ilPattern;
    } else {
      pattern = chessPattern;
    }

    if (pattern == frameData[833]) {
      irData = frameData[pixelNumber];
      if (irData > 32767) {
        irData = irData - 65536;
      }
      irData = irData * gain;

      kta = params->kta[pixelNumber] / ktaScale;
      kv = params->kv[pixelNumber] / kvScale;
      irData = irData - params->offset[pixelNumber] * (1 + kta * (ta - 25))
               * (1 + kv * (vdd - 3.3));

      if (mode != params->calibrationModeEE) {
        irData = irData + params->ilChessC[2] * (2 * ilPattern - 1)
                 - params->ilChessC[1] * conversionPattern;
      }

      irData = irData - params->tgc * irDataCP[subPage];
      irData = irData / emissivity;

      alphaCompensated = SPARKFUN_SCALEALPHA * alphaScale
                         / params->alpha[pixelNumber];
      alphaCompensated = alphaCompensated * (1 + params->KsTa * (ta - 25));

      Sx = alphaCompensated * alphaCompensated * alphaCompensated
           * (irData + alphaCompensated * taTr);
      Sx = sqrt(sqrt(Sx)) * params->ksTo[1];

      To =
        sqrt(sqrt(irData
                  / (alphaCompensated * (1 - params->ksTo[1] * 273.15) + Sx)
                  + taTr))
        - 273.15;

      if (To < params->ct[1]) {
        range = 0;
      } else if (To < params->ct[2]) {
        range = 1;
      } else if (To < params->ct[3]) {
        range = 2;
      } else {
        range = 3;
      }

      To =
        sqrt(sqrt(irData
                  / (alphaCompensated * alphaCorrR[range]
                     * (1 + params->ksTo[range] * (To - params->ct[range])))
                  + taTr)) - 273.15;

      result[pixelNumber] = To;
    }
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Calculates values for all 768 pixels - not absolute temperature!
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_image(uint16_t *frameData,
                                        const paramsMLX90640 *params,
                                        float *result)
{
  float vdd;
  float ta;
  float gain;
  float irDataCP[2];
  float irData;
  float alphaCompensated;
  uint8_t mode;
  int8_t ilPattern;
  int8_t chessPattern;
  int8_t pattern;
  int8_t conversionPattern;
  float image;
  uint16_t subPage;
  float ktaScale;
  float kvScale;
  float kta;
  float kv;

  if ((frameData == NULL) || (params == NULL) || (result == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  subPage = frameData[833];
  sparkfun_mlx90640_get_vdd(frameData, params, &vdd);
  sparkfun_mlx90640_get_ta(frameData, params, &ta);

  ktaScale = pow(2, (double)params->ktaScale);
  kvScale = pow(2, (double)params->kvScale);

// -------------------------- Gain calculation
//   ----------------------------------
  gain = frameData[778];
  if (gain > 32767) {
    gain = gain - 65536;
  }

  gain = params->gainEE / gain;

// -------------------------- Image calculation
//   ---------------------------------
  mode = (frameData[832] & 0x1000) >> 5;

  irDataCP[0] = frameData[776];
  irDataCP[1] = frameData[808];
  for ( int i = 0; i < 2; i++) {
    if (irDataCP[i] > 32767) {
      irDataCP[i] = irDataCP[i] - 65536;
    }
    irDataCP[i] = irDataCP[i] * gain;
  }
  irDataCP[0] = irDataCP[0] - params->cpOffset[0]
                * (1 + params->cpKta * (ta - 25))
                * (1 + params->cpKv * (vdd - 3.3));
  if (mode == params->calibrationModeEE) {
    irDataCP[1] = irDataCP[1] - params->cpOffset[1]
                  * (1 + params->cpKta * (ta - 25))
                  * (1 + params->cpKv * (vdd - 3.3));
  } else {
    irDataCP[1] = irDataCP[1] - (params->cpOffset[1] + params->ilChessC[0])
                  * (1 + params->cpKta * (ta - 25))
                  * (1 + params->cpKv * (vdd - 3.3));
  }

  for ( int pixelNumber = 0; pixelNumber < 768; pixelNumber++) {
    ilPattern = pixelNumber / 32 - (pixelNumber / 64) * 2;
    chessPattern = ilPattern ^ (pixelNumber - (pixelNumber / 2) * 2);
    conversionPattern =
      ((pixelNumber + 2) / 4 - (pixelNumber + 3) / 4 + (pixelNumber + 1) / 4
       - pixelNumber / 4) * (1 - 2 * ilPattern);

    if (mode == 0) {
      pattern = ilPattern;
    } else {
      pattern = chessPattern;
    }

    if (pattern == frameData[833]) {
      irData = frameData[pixelNumber];
      if (irData > 32767) {
        irData = irData - 65536;
      }
      irData = irData * gain;

      kta = params->kta[pixelNumber] / ktaScale;
      kv = params->kv[pixelNumber] / kvScale;
      irData = irData - params->offset[pixelNumber] * (1 + kta * (ta - 25))
               * (1 + kv * (vdd - 3.3));

      if (mode != params->calibrationModeEE) {
        irData = irData + params->ilChessC[2] * (2 * ilPattern - 1)
                 - params->ilChessC[1] * conversionPattern;
      }

      irData = irData - params->tgc * irDataCP[subPage];

      alphaCompensated = params->alpha[pixelNumber];

      image = irData * alphaCompensated;

      result[pixelNumber] = image;
    }
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Provides the current VDD of the device
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_vdd(uint16_t *frameData,
                                      const paramsMLX90640 *params,
                                      float *vdd)
{
  float temp_vdd = 0;
  float resolutionCorrection;
  int resolutionRAM;

  if ((frameData == NULL) || (params == NULL) || (vdd == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  temp_vdd = frameData[810];
  if (temp_vdd > 32767) {
    temp_vdd = temp_vdd - 65536;
  }
  resolutionRAM = (frameData[832] & 0x0C00) >> 10;
  resolutionCorrection = pow(2, (double)params->resolutionEE) / pow(2,
                                                                    (double)resolutionRAM);
  temp_vdd = (resolutionCorrection * temp_vdd - params->vdd25) / params->kVdd
             + 3.3;

  *vdd = temp_vdd;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Provides the current Ta of the device
 ******************************************************************************/
sl_status_t sparkfun_mlx90640_get_ta(uint16_t *frameData,
                                     const paramsMLX90640 *params,
                                     float *ta)
{
  float ptat;
  float ptatArt;
  float vdd;
  float temp_ta;

  if ((frameData == NULL) || (params == NULL) || (ta == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sparkfun_mlx90640_get_vdd(frameData, params, &vdd);

  ptat = frameData[800];
  if (ptat > 32767) {
    ptat = ptat - 65536;
  }

  ptatArt = frameData[768];
  if (ptatArt > 32767) {
    ptatArt = ptatArt - 65536;
  }
  ptatArt = (ptat / (ptat * params->alphaPTAT + ptatArt)) * pow(2, (double)18);

  temp_ta = (ptatArt / (1 + params->KvPTAT * (vdd - 3.3)) - params->vPTAT25);
  temp_ta = temp_ta / params->KtPTAT + 25;

  *ta = temp_ta;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Provides the sub-page for a selected frame data
 ******************************************************************************/
sl_status_t mlx90640_get_sub_page_number(uint16_t *frameData,
                                         uint16_t *sub_page_number)
{
  if (frameData == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  *sub_page_number = frameData[833];
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Corrects the values of the broken pixels and/or the outlier pixels.
 ******************************************************************************/
sl_status_t mlx90640_bad_pixels_correction(uint16_t *pixels,
                                           float *to,
                                           int mode,
                                           paramsMLX90640 *params)
{
  float ap[4];
  uint8_t pix;
  uint8_t line;
  uint8_t column;

  if ((pixels == NULL) || (to == NULL) || (params == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  pix = 0;
  while (pixels[pix] != 0xFFFF) {
    line = pixels[pix] >> 5;
    column = pixels[pix] - (line << 5);

    if (mode == 1) {
      if (line == 0) {
        if (column == 0) {
          to[pixels[pix]] = to[33];
        } else if (column == 31) {
          to[pixels[pix]] = to[62];
        } else {
          to[pixels[pix]] = (to[pixels[pix] + 31] + to[pixels[pix] + 33]) / 2.0;
        }
      } else if (line == 23) {
        if (column == 0) {
          to[pixels[pix]] = to[705];
        } else if (column == 31) {
          to[pixels[pix]] = to[734];
        } else {
          to[pixels[pix]] = (to[pixels[pix] - 33] + to[pixels[pix] - 31]) / 2.0;
        }
      } else if (column == 0) {
        to[pixels[pix]] = (to[pixels[pix] - 31] + to[pixels[pix] + 33]) / 2.0;
      } else if (column == 31) {
        to[pixels[pix]] = (to[pixels[pix] - 33] + to[pixels[pix] + 31]) / 2.0;
      } else {
        ap[0] = to[pixels[pix] - 33];
        ap[1] = to[pixels[pix] - 31];
        ap[2] = to[pixels[pix] + 31];
        ap[3] = to[pixels[pix] + 33];
        to[pixels[pix]] = get_median(ap, 4);
      }
    } else {
      if (column == 0) {
        to[pixels[pix]] = to[pixels[pix] + 1];
      } else if ((column == 1) || (column == 30)) {
        to[pixels[pix]] = (to[pixels[pix] - 1] + to[pixels[pix] + 1]) / 2.0;
      } else if (column == 31) {
        to[pixels[pix]] = to[pixels[pix] - 1];
      } else {
        if ((is_pixel_bad(pixels[pix] - 2,
                          params) == 0)
            && (is_pixel_bad(pixels[pix] + 2, params) == 0)) {
          ap[0] = to[pixels[pix] + 1] - to[pixels[pix] + 2];
          ap[1] = to[pixels[pix] - 1] - to[pixels[pix] - 2];
          if (fabs(ap[0]) > fabs(ap[1])) {
            to[pixels[pix]] = to[pixels[pix] - 1] + ap[1];
          } else {
            to[pixels[pix]] = to[pixels[pix] + 1] + ap[0];
          }
        } else {
          to[pixels[pix]] = (to[pixels[pix] - 1] + to[pixels[pix] + 1]) / 2.0;
        }
      }
    }
    pix = pix + 1;
  }
  return SL_STATUS_OK;
}

// -----------------------------------------------------------------------------
//                     Static Local function definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_VDD_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int16_t kVdd;
  int16_t vdd25;

  kVdd = eeData[51];

  kVdd = (eeData[51] & 0xFF00) >> 8;
  if (kVdd > 127) {
    kVdd = kVdd - 256;
  }
  kVdd = 32 * kVdd;
  vdd25 = eeData[51] & 0x00FF;
  vdd25 = ((vdd25 - 256) << 5) - 8192;

  mlx90640->kVdd = kVdd;
  mlx90640->vdd25 = vdd25;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_PTAT_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  float KvPTAT;
  float KtPTAT;
  int16_t vPTAT25;
  float alphaPTAT;

  KvPTAT = (eeData[50] & 0xFC00) >> 10;
  if (KvPTAT > 31) {
    KvPTAT = KvPTAT - 64;
  }
  KvPTAT = KvPTAT / 4096;

  KtPTAT = eeData[50] & 0x03FF;
  if (KtPTAT > 511) {
    KtPTAT = KtPTAT - 1024;
  }
  KtPTAT = KtPTAT / 8;

  vPTAT25 = eeData[49];

  alphaPTAT = (eeData[16] & 0xF000) / pow(2, (double)14) + 8.0f;

  mlx90640->KvPTAT = KvPTAT;
  mlx90640->KtPTAT = KtPTAT;
  mlx90640->vPTAT25 = vPTAT25;
  mlx90640->alphaPTAT = alphaPTAT;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_gain_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int16_t gainEE;

  gainEE = eeData[48];
  if (gainEE >= 32767) {
    gainEE = gainEE - 65536;
  }

  mlx90640->gainEE = gainEE;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_Tgc_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  float tgc;
  tgc = eeData[60] & 0x00FF;
  if (tgc > 127) {
    tgc = tgc - 256;
  }
  tgc = tgc / 32.0f;

  mlx90640->tgc = tgc;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_resolution_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  uint8_t resolutionEE;
  resolutionEE = (eeData[56] & 0x3000) >> 12;

  mlx90640->resolutionEE = resolutionEE;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_KsTa_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  float KsTa;
  KsTa = (eeData[60] & 0xFF00) >> 8;
  if (KsTa > 127) {
    KsTa = KsTa - 256;
  }
  KsTa = KsTa / 8192.0f;

  mlx90640->KsTa = KsTa;
}

// ------------------------------------------------------------------------------

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_KsTo_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int32_t KsToScale;
  int8_t step;

  step = ((eeData[63] & 0x3000) >> 12) * 10;

  mlx90640->ct[0] = -40;
  mlx90640->ct[1] = 0;
  mlx90640->ct[2] = (eeData[63] & 0x00F0) >> 4;
  mlx90640->ct[3] = (eeData[63] & 0x0F00) >> 8;

  mlx90640->ct[2] = mlx90640->ct[2] * step;
  mlx90640->ct[3] = mlx90640->ct[2] + mlx90640->ct[3] * step;
  mlx90640->ct[4] = 400;

  KsToScale = (eeData[63] & 0x000F) + 8;
  KsToScale = 1UL << KsToScale;

  mlx90640->ksTo[0] = eeData[61] & 0x00FF;
  mlx90640->ksTo[1] = (eeData[61] & 0xFF00) >> 8;
  mlx90640->ksTo[2] = eeData[62] & 0x00FF;
  mlx90640->ksTo[3] = (eeData[62] & 0xFF00) >> 8;

  for (int i = 0; i < 4; i++) {
    if (mlx90640->ksTo[i] > 127) {
      mlx90640->ksTo[i] = mlx90640->ksTo[i] - 256;
    }
    mlx90640->ksTo[i] = mlx90640->ksTo[i] / KsToScale;
  }

  mlx90640->ksTo[4] = -0.0002;
}

// ------------------------------------------------------------------------------

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_Alpha_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int accRow[24];
  int accColumn[32];
  int p = 0;
  int alphaRef;
  uint8_t alphaScale;
  uint8_t accRowScale;
  uint8_t accColumnScale;
  uint8_t accRemScale;
  float alphaTemp[768];
  float temp;

  accRemScale = eeData[32] & 0x000F;
  accColumnScale = (eeData[32] & 0x00F0) >> 4;
  accRowScale = (eeData[32] & 0x0F00) >> 8;
  alphaScale = ((eeData[32] & 0xF000) >> 12) + 30;
  alphaRef = eeData[33];

  for (int i = 0; i < 6; i++) {
    p = i * 4;
    accRow[p + 0] = (eeData[34 + i] & 0x000F);
    accRow[p + 1] = (eeData[34 + i] & 0x00F0) >> 4;
    accRow[p + 2] = (eeData[34 + i] & 0x0F00) >> 8;
    accRow[p + 3] = (eeData[34 + i] & 0xF000) >> 12;
  }

  for (int i = 0; i < 24; i++) {
    if (accRow[i] > 7) {
      accRow[i] = accRow[i] - 16;
    }
  }

  for (int i = 0; i < 8; i++) {
    p = i * 4;
    accColumn[p + 0] = (eeData[40 + i] & 0x000F);
    accColumn[p + 1] = (eeData[40 + i] & 0x00F0) >> 4;
    accColumn[p + 2] = (eeData[40 + i] & 0x0F00) >> 8;
    accColumn[p + 3] = (eeData[40 + i] & 0xF000) >> 12;
  }

  for (int i = 0; i < 32; i++) {
    if (accColumn[i] > 7) {
      accColumn[i] = accColumn[i] - 16;
    }
  }

  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 32; j++) {
      p = 32 * i + j;
      alphaTemp[p] = (eeData[64 + p] & 0x03F0) >> 4;
      if (alphaTemp[p] > 31) {
        alphaTemp[p] = alphaTemp[p] - 64;
      }
      alphaTemp[p] = alphaTemp[p] * (1 << accRemScale);
      alphaTemp[p] =
        (alphaRef
         + (accRow[i] <<
            accRowScale) + (accColumn[j] << accColumnScale) + alphaTemp[p]);
      alphaTemp[p] = alphaTemp[p] / pow(2, (double)alphaScale);
      alphaTemp[p] = alphaTemp[p] - mlx90640->tgc
                     * (mlx90640->cpAlpha[0] + mlx90640->cpAlpha[1]) / 2;
      alphaTemp[p] = SPARKFUN_SCALEALPHA / alphaTemp[p];
    }
  }

  temp = alphaTemp[0];
  for (int i = 1; i < 768; i++) {
    if (alphaTemp[i] > temp) {
      temp = alphaTemp[i];
    }
  }

  alphaScale = 0;
  while (temp < 32767.4) {
    temp = temp * 2;
    alphaScale = alphaScale + 1;
  }

  for (int i = 0; i < 768; i++) {
    temp = alphaTemp[i] * pow(2, (double)alphaScale);
    mlx90640->alpha[i] = (temp + 0.5);
  }
  mlx90640->alphaScale = alphaScale;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_offset_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int occRow[24];
  int occColumn[32];
  int p = 0;
  int16_t offsetRef;
  uint8_t occRowScale;
  uint8_t occColumnScale;
  uint8_t occRemScale;

  occRemScale = (eeData[16] & 0x000F);
  occColumnScale = (eeData[16] & 0x00F0) >> 4;
  occRowScale = (eeData[16] & 0x0F00) >> 8;
  offsetRef = eeData[17];
  if (offsetRef >= 32767) {
    offsetRef = offsetRef - 65536;
  }

  for (int i = 0; i < 6; i++) {
    p = i * 4;
    occRow[p + 0] = (eeData[18 + i] & 0x000F);
    occRow[p + 1] = (eeData[18 + i] & 0x00F0) >> 4;
    occRow[p + 2] = (eeData[18 + i] & 0x0F00) >> 8;
    occRow[p + 3] = (eeData[18 + i] & 0xF000) >> 12;
  }

  for (int i = 0; i < 24; i++) {
    if (occRow[i] > 7) {
      occRow[i] = occRow[i] - 16;
    }
  }

  for (int i = 0; i < 8; i++) {
    p = i * 4;
    occColumn[p + 0] = (eeData[24 + i] & 0x000F);
    occColumn[p + 1] = (eeData[24 + i] & 0x00F0) >> 4;
    occColumn[p + 2] = (eeData[24 + i] & 0x0F00) >> 8;
    occColumn[p + 3] = (eeData[24 + i] & 0xF000) >> 12;
  }

  for (int i = 0; i < 32; i++) {
    if (occColumn[i] > 7) {
      occColumn[i] = occColumn[i] - 16;
    }
  }

  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 32; j++) {
      p = 32 * i + j;
      mlx90640->offset[p] = (eeData[64 + p] & 0xFC00) >> 10;
      if (mlx90640->offset[p] > 31) {
        mlx90640->offset[p] = mlx90640->offset[p] - 64;
      }
      mlx90640->offset[p] = mlx90640->offset[p] * (1 << occRemScale);
      mlx90640->offset[p] =
        (offsetRef
         + (occRow[i] <<
            occRowScale)
         + (occColumn[j] << occColumnScale) + mlx90640->offset[p]);
    }
  }
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_KtaPixel_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int p = 0;
  int8_t KtaRC[4];
  int8_t KtaRoCo;
  int8_t KtaRoCe;
  int8_t KtaReCo;
  int8_t KtaReCe;
  uint8_t ktaScale1;
  uint8_t ktaScale2;
  uint8_t split;
  float ktaTemp[768];
  float temp;

  KtaRoCo = (eeData[54] & 0xFF00) >> 8;
  if (KtaRoCo >= 127) {
    KtaRoCo = KtaRoCo - 256;
  }
  KtaRC[0] = KtaRoCo;

  KtaReCo = (eeData[54] & 0x00FF);
  if (KtaReCo >= 127) {
    KtaReCo = KtaReCo - 256;
  }
  KtaRC[2] = KtaReCo;

  KtaRoCe = (eeData[55] & 0xFF00) >> 8;
  if (KtaRoCe >= 127) {
    KtaRoCe = KtaRoCe - 256;
  }
  KtaRC[1] = KtaRoCe;

  KtaReCe = (eeData[55] & 0x00FF);
  if (KtaReCe >= 127) {
    KtaReCe = KtaReCe - 256;
  }
  KtaRC[3] = KtaReCe;

  ktaScale1 = ((eeData[56] & 0x00F0) >> 4) + 8;
  ktaScale2 = (eeData[56] & 0x000F);

  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 32; j++) {
      p = 32 * i + j;
      split = 2 * (p / 32 - (p / 64) * 2) + p % 2;
      ktaTemp[p] = (eeData[64 + p] & 0x000E) >> 1;
      if (ktaTemp[p] > 3) {
        ktaTemp[p] = ktaTemp[p] - 8;
      }
      ktaTemp[p] = ktaTemp[p] * (1 << ktaScale2);
      ktaTemp[p] = KtaRC[split] + ktaTemp[p];
      ktaTemp[p] = ktaTemp[p] / pow(2, (double)ktaScale1);
    }
  }

  temp = fabs(ktaTemp[0]);
  for (int i = 1; i < 768; i++) {
    if (fabs(ktaTemp[i]) > temp) {
      temp = fabs(ktaTemp[i]);
    }
  }

  ktaScale1 = 0;
  while (temp < 63.4) {
    temp = temp * 2;
    ktaScale1 = ktaScale1 + 1;
  }

  for (int i = 0; i < 768; i++) {
    temp = ktaTemp[i] * pow(2, (double)ktaScale1);
    if (temp < 0) {
      mlx90640->kta[i] = (temp - 0.5);
    } else {
      mlx90640->kta[i] = (temp + 0.5);
    }
  }
  mlx90640->ktaScale = ktaScale1;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_KvPixel_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  int p = 0;
  int8_t KvT[4];
  int8_t KvRoCo;
  int8_t KvRoCe;
  int8_t KvReCo;
  int8_t KvReCe;
  uint8_t kvScale;
  uint8_t split;
  float kvTemp[768];
  float temp;

  KvRoCo = (eeData[52] & 0xF000) >> 12;
  if (KvRoCo > 7) {
    KvRoCo = KvRoCo - 16;
  }
  KvT[0] = KvRoCo;

  KvReCo = (eeData[52] & 0x0F00) >> 8;
  if (KvReCo > 7) {
    KvReCo = KvReCo - 16;
  }
  KvT[2] = KvReCo;

  KvRoCe = (eeData[52] & 0x00F0) >> 4;
  if (KvRoCe > 7) {
    KvRoCe = KvRoCe - 16;
  }
  KvT[1] = KvRoCe;

  KvReCe = (eeData[52] & 0x000F);
  if (KvReCe > 7) {
    KvReCe = KvReCe - 16;
  }
  KvT[3] = KvReCe;

  kvScale = (eeData[56] & 0x0F00) >> 8;

  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 32; j++) {
      p = 32 * i + j;
      split = 2 * (p / 32 - (p / 64) * 2) + p % 2;
      kvTemp[p] = KvT[split];
      kvTemp[p] = kvTemp[p] / pow(2, (double)kvScale);
    }
  }

  temp = fabs(kvTemp[0]);
  for (int i = 1; i < 768; i++) {
    if (fabs(kvTemp[i]) > temp) {
      temp = fabs(kvTemp[i]);
    }
  }

  kvScale = 0;
  while (temp < 63.4) {
    temp = temp * 2;
    kvScale = kvScale + 1;
  }

  for (int i = 0; i < 768; i++) {
    temp = kvTemp[i] * pow(2, (double)kvScale);
    if (temp < 0) {
      mlx90640->kv[i] = (temp - 0.5);
    } else {
      mlx90640->kv[i] = (temp + 0.5);
    }
  }
  mlx90640->kvScale = kvScale;
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_CP_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  float alphaSP[2];
  int16_t offsetSP[2];
  float cpKv;
  float cpKta;
  uint8_t alphaScale;
  uint8_t ktaScale1;
  uint8_t kvScale;

  alphaScale = ((eeData[32] & 0xF000) >> 12) + 27;

  offsetSP[0] = (eeData[58] & 0x03FF);
  if (offsetSP[0] > 511) {
    offsetSP[0] = offsetSP[0] - 1024;
  }

  offsetSP[1] = (eeData[58] & 0xFC00) >> 10;
  if (offsetSP[1] > 31) {
    offsetSP[1] = offsetSP[1] - 64;
  }
  offsetSP[1] = offsetSP[1] + offsetSP[0];

  alphaSP[0] = (eeData[57] & 0x03FF);
  if (alphaSP[0] > 511) {
    alphaSP[0] = alphaSP[0] - 1024;
  }
  alphaSP[0] = alphaSP[0] /  pow(2, (double)alphaScale);

  alphaSP[1] = (eeData[57] & 0xFC00) >> 10;
  if (alphaSP[1] > 31) {
    alphaSP[1] = alphaSP[1] - 64;
  }
  alphaSP[1] = (1 + alphaSP[1] / 128) * alphaSP[0];

  cpKta = (eeData[59] & 0x00FF);
  if (cpKta > 127) {
    cpKta = cpKta - 256;
  }
  ktaScale1 = ((eeData[56] & 0x00F0) >> 4) + 8;
  mlx90640->cpKta = cpKta / pow(2, (double)ktaScale1);

  cpKv = (eeData[59] & 0xFF00) >> 8;
  if (cpKv > 127) {
    cpKv = cpKv - 256;
  }
  kvScale = (eeData[56] & 0x0F00) >> 8;
  mlx90640->cpKv = cpKv / pow(2, (double)kvScale);

  mlx90640->cpAlpha[0] = alphaSP[0];
  mlx90640->cpAlpha[1] = alphaSP[1];
  mlx90640->cpOffset[0] = offsetSP[0];
  mlx90640->cpOffset[1] = offsetSP[1];
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
void extract_CILC_parameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  float ilChessC[3];
  uint8_t calibrationModeEE;

  calibrationModeEE = (eeData[10] & 0x0800) >> 4;
  calibrationModeEE = calibrationModeEE ^ 0x80;

  ilChessC[0] = (eeData[53] & 0x003F);
  if (ilChessC[0] > 31) {
    ilChessC[0] = ilChessC[0] - 64;
  }
  ilChessC[0] = ilChessC[0] / 16.0f;

  ilChessC[1] = (eeData[53] & 0x07C0) >> 6;
  if (ilChessC[1] > 15) {
    ilChessC[1] = ilChessC[1] - 32;
  }
  ilChessC[1] = ilChessC[1] / 2.0f;

  ilChessC[2] = (eeData[53] & 0xF800) >> 11;
  if (ilChessC[2] > 15) {
    ilChessC[2] = ilChessC[2] - 32;
  }
  ilChessC[2] = ilChessC[2] / 8.0f;

  mlx90640->calibrationModeEE = calibrationModeEE;
  mlx90640->ilChessC[0] = ilChessC[0];
  mlx90640->ilChessC[1] = ilChessC[1];
  mlx90640->ilChessC[2] = ilChessC[2];
}

/***************************************************************************//**
 * Extracts calibration values from dumped EEPROM data
 ******************************************************************************/
int extract_deviating_pixels(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
  uint16_t pixCnt = 0;
  uint16_t brokenPixCnt = 0;
  uint16_t outlierPixCnt = 0;
  int warn = 0;
  int i;

  for (pixCnt = 0; pixCnt < 5; pixCnt++) {
    mlx90640->brokenPixels[pixCnt] = 0xFFFF;
    mlx90640->outlierPixels[pixCnt] = 0xFFFF;
  }

  pixCnt = 0;
  while (pixCnt < 768 && brokenPixCnt < 5 && outlierPixCnt < 5) {
    if (eeData[pixCnt + 64] == 0) {
      mlx90640->brokenPixels[brokenPixCnt] = pixCnt;
      brokenPixCnt = brokenPixCnt + 1;
    } else if ((eeData[pixCnt + 64] & 0x0001) != 0) {
      mlx90640->outlierPixels[outlierPixCnt] = pixCnt;
      outlierPixCnt = outlierPixCnt + 1;
    }
    pixCnt = pixCnt + 1;
  }

  if (brokenPixCnt > 4) {
    warn = -3;
  } else if (outlierPixCnt > 4) {
    warn = -4;
  } else if ((brokenPixCnt + outlierPixCnt) > 4) {
    warn = -5;
  } else {
    for (pixCnt = 0; pixCnt < brokenPixCnt; pixCnt++) {
      for (i = pixCnt + 1; i < brokenPixCnt; i++) {
        warn = check_adjacent_pixels(mlx90640->brokenPixels[pixCnt],
                                     mlx90640->brokenPixels[i]);
        if (warn != 0) {
          return warn;
        }
      }
    }

    for (pixCnt = 0; pixCnt < outlierPixCnt; pixCnt++) {
      for (i = pixCnt + 1; i < outlierPixCnt; i++) {
        warn = check_adjacent_pixels(mlx90640->outlierPixels[pixCnt],
                                     mlx90640->outlierPixels[i]);
        if (warn != 0) {
          return warn;
        }
      }
    }

    for (pixCnt = 0; pixCnt < brokenPixCnt; pixCnt++) {
      for (i = 0; i < outlierPixCnt; i++) {
        warn = check_adjacent_pixels(mlx90640->brokenPixels[pixCnt],
                                     mlx90640->outlierPixels[i]);
        if (warn != 0) {
          return warn;
        }
      }
    }
  }
  return warn;
}

/***************************************************************************//**
 * Helper function
 ******************************************************************************/
int check_adjacent_pixels(uint16_t pix1, uint16_t pix2)
{
  int pixPosDif;

  pixPosDif = pix1 - pix2;
  if ((pixPosDif > -34) && (pixPosDif < -30)) {
    return -6;
  }
  if ((pixPosDif > -2) && (pixPosDif < 2)) {
    return -6;
  }
  if ((pixPosDif > 30) && (pixPosDif < 34)) {
    return -6;
  }

  return 0;
}

/***************************************************************************//**
 * Helper function - returns the median of the the gives "values" array
 ******************************************************************************/
static float get_median(float *values, int n)
{
  float temp;

  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (values[j] < values[i]) {
        temp = values[i];
        values[i] = values[j];
        values[j] = temp;
      }
    }
  }

  if (n % 2 == 0) {
    return ((values[n / 2] + values[n / 2 - 1]) / 2.0);
  } else {
    return values[n / 2];
  }
}

/***************************************************************************//**
 * Checks a pixel if it's working properly
 ******************************************************************************/
static int is_pixel_bad(uint16_t pixel, paramsMLX90640 *params)
{
  for (int i = 0; i < 5; i++) {
    if ((pixel == params->outlierPixels[i])
        || (pixel == params->brokenPixels[i])) {
      return 1;
    }
  }
  return 0;
}

/***************************************************************************//**
 * Initiates an I2C read of the device
 ******************************************************************************/
static sl_status_t sparkfun_mlx90640_i2c_read(uint16_t startAddress,
                                              uint16_t nMemAddressRead,
                                              uint16_t *data)
{
  uint8_t i2cData[2 * nMemAddressRead];
  uint16_t counter = 0;
  uint16_t i = 0;
  uint16_t *p = data;

  uint8_t cmd[2] = { 0, 0 };
  cmd[0] = startAddress >> 8;
  cmd[1] = startAddress & 0x00FF;

  if (I2C_MASTER_SUCCESS != i2c_master_write_then_read(&mlx90640_i2c,
                                                       cmd,
                                                       2,
                                                       (uint8_t *)i2cData,
                                                       2 * nMemAddressRead)) {
    return SL_STATUS_TRANSMIT;
  }

  for (counter = 0; counter < nMemAddressRead; counter++) {
    i = counter << 1;
    *p++ = (uint16_t)i2cData[i] * 256 + (uint16_t)i2cData[i + 1];
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Initiates an I2C write to the device
 ******************************************************************************/
static sl_status_t sparkfun_mlx90640_i2c_write(uint16_t writeAddress,
                                               uint16_t data)
{
  uint8_t cmd[4] = { 0, 0, 0, 0 };
  static uint16_t dataCheck;

  cmd[0] = writeAddress >> 8;
  cmd[1] = writeAddress & 0x00FF;
  cmd[2] = data >> 8;
  cmd[3] = data & 0x00FF;

  if (I2C_MASTER_SUCCESS != i2c_master_write(&mlx90640_i2c, cmd, 4)) {
    return SL_STATUS_TRANSMIT;
  }

  if (SL_STATUS_OK != sparkfun_mlx90640_i2c_read(writeAddress, 1, &dataCheck)) {
    return SL_STATUS_FAIL;
  }

  if (dataCheck != data) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Issues an I2C general reset
 ******************************************************************************/
static sl_status_t sparkfun_mlx90640_i2c_general_reset(void)
{
  uint8_t cmd[2] = { 0x00, 0x06 };

  if (I2C_MASTER_SUCCESS != i2c_master_write(&mlx90640_i2c, cmd, 2)) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}
