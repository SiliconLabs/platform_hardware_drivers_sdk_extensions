/***************************************************************************//**
 * @file adafruit_st7789_dma.c
 * @brief Adafruit ST7789 Color TFT Display Driver source file.
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
// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include "em_cmu.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_sleeptimer.h"
#include "sl_component_catalog.h"
#include "spidrv.h"
#include "adafruit_st7789_spi_dma_config.h"
#include "sl_spidrv_st7789_config.h"
#include "adafruit_st7789.h"

// -----------------------------------------------------------------------------
//                       Macros
// -----------------------------------------------------------------------------
#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))
#define BYTE_PER_PIXEL          (2)       /* 2 Bytes color data per pixel */
#define MAX_XFER_PIXEL_COUNT    ((DMADRV_MAX_XFER_COUNT) / BYTE_PER_PIXEL)

#define SPI_CS_LOW(handle) \
  GPIO_PinOutClear(handle->portCs, handle->pinCs);
#define SPI_CS_HIGH(handle) \
  GPIO_PinOutSet(handle->portCs, handle->pinCs);

#define SPI_DC_LOW() \
  GPIO_PinOutClear(ST7789_SPI_DC_PORT, ST7789_SPI_DC_PIN);
#define SPI_DC_HIGH() \
  GPIO_PinOutSet(ST7789_SPI_DC_PORT, ST7789_SPI_DC_PIN);

// -----------------------------------------------------------------------------
//                       Local Variables
// -----------------------------------------------------------------------------
static SPIDRV_Handle_t gspi_handle = NULL;
static uint16_t _colstart = 0, _colstart2 = 0;
static uint16_t _rowstart = 0, _rowstart2 = 0;
static uint16_t _xstart = 0, _ystart = 0;
static uint16_t _width = ST7789_TFTWIDTH, _height = ST7789_TFTHEIGHT;
static bool color_swap_enabled = false;
static uint32_t gtotal_pixel = 0;
static uint32_t gpixel_transmit = 0;
static uint32_t gpixel_transmit_counter = 0;
static uint8_t *pColorBuffer = NULL;
static void (*flush_area_callback)(void *arg) = NULL;
static void *flush_area_callback_arg = NULL;

static uint16_t dma_buffer[(DMADRV_MAX_XFER_COUNT + 1) / 2];
static uint8_t *pColorSwap = NULL;

static sl_status_t adafruit_st7789_spi_send_command(uint8_t command,
                                                    const uint8_t *data,
                                                    uint8_t len);
static sl_status_t adafruit_st7789_spi_write_command(uint8_t cmd);
static sl_status_t adafruit_st7789_spi_write16(uint16_t data);
static sl_status_t adafruit_st7789_spi_write32(uint32_t data);
static sl_status_t adafruit_st7789_write_color(uint16_t color, uint32_t len);
static sl_status_t adafruit_st7789_write_pixels(uint16_t *colors, uint32_t len);
static sl_status_t adafruit_st7789_write_fill_rect_preclipped(int16_t x,
                                                              int16_t y,
                                                              int16_t w,
                                                              int16_t h,
                                                              uint16_t color);
static sl_status_t adafruit_st7789_fill_rect(int16_t x,
                                             int16_t y,
                                             int16_t w,
                                             int16_t h,
                                             uint16_t color);
static void st7789_flush_area_transmit_callback(
  struct SPIDRV_HandleData *handle,
  Ecode_t transferStatus,
  int itemsTransferred);

/***************************************************************************//**
 * @brief
 *  Adafruit_SPITFT Send Command handles complete sending of commands and
 *  data.
 *
 * @param[in] command
 *  The Command Byte.
 * @param[in] data
 *  A pointer to the Data bytes to send.
 * @param[in] len
 *  The number of bytes we should send.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_IO or SL_STATUS_INVALID_PARAMETER if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_st7789_spi_send_command(uint8_t command,
                                                    const uint8_t *data,
                                                    uint8_t len)
{
  Ecode_t retVal;
  sl_status_t stt = SL_STATUS_OK;

  SPI_CS_LOW(gspi_handle);
  SPI_DC_LOW();

  retVal = SPIDRV_MTransmitB(gspi_handle, &command, 1);
  SPI_DC_HIGH();

  if (ECODE_OK != retVal) {
    stt = SL_STATUS_IO;
    goto end;
  }

  if (len != 0) {
    if (NULL != data) {
      retVal = SPIDRV_MTransmitB(gspi_handle, data, len);
      if (ECODE_OK != retVal) {
        stt = SL_STATUS_IO;
        goto end;
      }
    } else {
      stt = SL_STATUS_INVALID_PARAMETER;
      goto end;
    }
  }

  end:
  SPI_CS_HIGH(gspi_handle);
  return stt;
}

/***************************************************************************//**
 * @brief
 *  Write a single command byte to the display.
 *
 * @param[in] cmd
 *  8-bit command to write.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_IO if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_st7789_spi_write_command(uint8_t cmd)
{
  SPI_DC_LOW();
  Ecode_t retVal = SPIDRV_MTransmitB(gspi_handle, &cmd, 1);
  SPI_DC_HIGH();

  return ECODE_OK == retVal ? SL_STATUS_OK : SL_STATUS_IO;
}

/***************************************************************************//**
 * @brief
 *  Write a single command word to the display.
 *
 * @param[in] cmd
 *  16-bit command to write.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_IO if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_st7789_spi_write16(uint16_t data)
{
  uint8_t bytes[2];

  bytes[0] = data >> 8;
  bytes[1] = data & 0x00FF;

  Ecode_t retVal = SPIDRV_MTransmitB(gspi_handle, bytes, 2);

  return ECODE_OK == retVal ? SL_STATUS_OK : SL_STATUS_IO;
}

/***************************************************************************//**
 * @brief
 *  Write a double command word to the display.
 *
 * @param[in] cmd
 *  32-bit command to write.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_IO if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_st7789_spi_write32(uint32_t data)
{
  uint8_t bytes[4];

  bytes[0] = (data >> 24);
  bytes[1] = (data >> 16) & 0x00FF;
  bytes[2] = (data >> 8) & 0x0000FF;
  bytes[3] = data & 0x000000FF;

  Ecode_t retVal = SPIDRV_MTransmitB(gspi_handle, bytes, 4);

  return ECODE_OK == retVal ? SL_STATUS_OK : SL_STATUS_IO;
}

/***************************************************************************//**
 * @brief
 *  Issue a series of pixels, all the same color.
 *
 * @param[in] color
 *  16-bit pixel color in '565' RGB format.
 * @param[in] len
 *  Number of pixels to draw.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_IO if the process is failed.
 *  SL_STATUS_INVALID_PARAMETER if (len = 0)
 ******************************************************************************/
static sl_status_t adafruit_st7789_write_color(uint16_t color, uint32_t len)
{
  uint32_t i, n, size;
  Ecode_t retVal;

  if (!len) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  for (n = 0; n < len; n += MAX_XFER_PIXEL_COUNT) {
    if (n + MAX_XFER_PIXEL_COUNT <= len) {
      size = MAX_XFER_PIXEL_COUNT;
    } else {
      size = len - n;
    }

    for (i = 0; i < size; i++) {
      dma_buffer[i] = color >> 8 | ((uint16_t)(color & 0xff) << 8);
    }
    retVal = SPIDRV_MTransmitB(gspi_handle, dma_buffer, size * 2);
    if (ECODE_OK != retVal) {
      goto error;
    }
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_IO;
}

/***************************************************************************//**
 * @brief
 *  Issue a series of pixels from memory to the display.
 *
 * @param[in] colors
 *  Pointer to array of 16-bit pixel values in '565' RGB format.
 * @param[in] len
 *  Number of elements in 'colors' array.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_IO if the process is failed.
 *  SL_STATUS_INVALID_PARAMETER if (len = 0) or (colors == NULL)
 ******************************************************************************/
static sl_status_t adafruit_st7789_write_pixels(uint16_t *colors, uint32_t len)
{
  uint32_t i, n, size;
  Ecode_t retVal;

  if ((!len) || (colors == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (n = 0; n < len; n += MAX_XFER_PIXEL_COUNT) {
    if (n + MAX_XFER_PIXEL_COUNT <= len) {
      size = MAX_XFER_PIXEL_COUNT;
    } else {
      size = len - n;
    }

    for (i = 0; i < size; i++) {
      dma_buffer[i] = *colors >> 8 | ((uint16_t)(*colors & 0xff) << 8);
      colors++;
    }
    retVal = SPIDRV_MTransmitB(gspi_handle, dma_buffer, size * 2);
    if (ECODE_OK != retVal) {
      goto error;
    }
  }

  return SL_STATUS_OK;
  error:
  return SL_STATUS_IO;
}

/***************************************************************************//**
 * @brief
 *  A lower-level version of adafruit_st7789_fill_rect(). This version
 *  requires all inputs are in-bounds, that width and height are positive,
 *  and no part extends offscreen. NO EDGE CLIPPING OR REJECTION IS PERFORMED.
 *  If higher-level graphics primitives are written to handle their own
 *  clipping earlier in the drawing process, this can avoid unnecessary
 *  function calls and repeated clipping operations in the lower-level
 *  functions.
 *
 * @param[in] x
 *  Horizontal position of first corner. MUST BE WITHIN SCREEN BOUNDS.
 * @param[in] y
 *  Vertical position of first corner. MUST BE WITHIN SCREEN BOUNDS.
 * @param[in] w
 *  Rectangle width in pixels. MUST BE POSITIVE AND NOT EXTEND OFF SCREEN.
 * @param[in] h
 *  Rectangle height in pixels. MUST BE POSITIVE AND NOT EXTEND OFF SCREEN.
 * @param[in] color
 *  16-bit fill color in '565' RGB format.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_st7789_write_fill_rect_preclipped(int16_t x,
                                                              int16_t y,
                                                              int16_t w,
                                                              int16_t h,
                                                              uint16_t color)
{
  adafruit_st7789_set_addr_window(x, y, w, h);
  adafruit_st7789_write_color(color, (uint32_t)w * h);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *  Draw a filled rectangle to the display.
 *
 * @param[in] x
 *  Horizontal position of first corner.
 * @param[in] y
 *  Vertical position of first corner.
 * @param[in] w
 *  Rectangle width in pixels (positive = right of first corner,
 *  negative = left of first corner).
 * @param[in] h
 *  Rectangle height in pixels (positive = below first corner,
 *  negative = above first corner)
 * @param[in] color
 *  16-bit fill color in '565' RGB format.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_st7789_fill_rect(int16_t x,
                                             int16_t y,
                                             int16_t w,
                                             int16_t h,
                                             uint16_t color)
{
  if (w && h) {
    if (w < 0) {
      x += w + 1;
      w = -w;
    }
    if (x < _width) {
      if (h < 0) {
        y += h + 1;
        h = -h;
      }
      if (y < _height) {
        int16_t x2 = x + w - 1;
        if (x2 >= 0) {
          int16_t y2 = y + h - 1;
          if (y2 >= 0) {
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            }
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            }
            if (x2 >= _width) {
              w = _width - x;
            }
            if (y2 >= _height) {
              h = _height - y;
            }
            SPI_CS_LOW(gspi_handle);
            adafruit_st7789_write_fill_rect_preclipped(x, y, w, h, color);
            SPI_CS_HIGH(gspi_handle);
          }
        }
      }
    }
  }

  return SL_STATUS_OK;
}

// -----------------------------------------------------------------------------
//                       Public Function
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Initialize the Adafruit ST7789 Color TFT Display Driver.
 *****************************************************************************/
sl_status_t adafruit_st7789_spi_dma_init(SPIDRV_Handle_t spidrv_handle)
{
  gspi_handle = spidrv_handle;

  GPIO_PinModeSet(SL_SPIDRV_ST7789_CS_PORT,
                  SL_SPIDRV_ST7789_CS_PIN,
                  gpioModePushPull,
                  0);
  GPIO_PinModeSet(ST7789_SPI_DC_PORT,
                  ST7789_SPI_DC_PIN,
                  gpioModePushPull,
                  0);

  static uint8_t init_cmd[] = {
    9,                              //  9 commands in list:
    ST7789_SWRESET, ST_CMD_DELAY,   //  1: Software reset, no args, w/delay
    150,                            //     ~150 ms delay
    ST7789_SLPOUT, ST_CMD_DELAY,    //  2: Out of sleep mode, no args, w/delay
    10,                             //      10 ms delay
    ST7789_COLMOD, 1 + ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
    0x55,                           //     16-bit color
    10,                             //     10 ms delay
    ST7789_MADCTL, 1,               //  4: Mem access ctrl (directions), 1 arg:
    0x08,                           //     Row/col addr, bottom-top refresh
    ST7789_CASET, 4,                //  5: Column addr set, 4 args, no delay:
    0x00,
    0,          //     XSTART = 0
    (ST7789_TFTWIDTH >> 8),
    (ST7789_TFTWIDTH & 0xFF),    //     XEND = ST7789_TFTWIDTH
    ST7789_RASET, 4,                //  6: Row addr set, 4 args, no delay:
    0x00,
    0,               //     YSTART = 0
    (ST7789_TFTHEIGHT >> 8),
    (ST7789_TFTHEIGHT & 0xFF),  //     YEND = ST7789_TFTHEIGHT
    ST7789_INVOFF, ST_CMD_DELAY,      //  7: INVOFF
    10,
    ST7789_NORON, ST_CMD_DELAY,     //  8: Normal display on, no args, w/delay
    10,                             //     10 ms delay
    ST7789_DISPON, ST_CMD_DELAY,    //  9: Main screen turn on, no args, delay
    10
  };

  uint8_t numCommands, cmd, numArgs;
  uint16_t ms;
  uint8_t *addr = init_cmd;

  numCommands = pgm_read_byte(addr++); // Number of commands to follow
  while (numCommands--) {              // For each command...
    cmd = pgm_read_byte(addr++);       // Read command
    numArgs = pgm_read_byte(addr++);   // Number of args to follow
    ms = numArgs & ST_CMD_DELAY;       // If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;          // Mask out delay bit
    adafruit_st7789_spi_send_command(cmd, addr, numArgs);
    addr += numArgs;

    if (ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if (ms == 255) {
        ms = 500; // If 255, delay for 500 ms
      }
      sl_sleeptimer_delay_millisecond(ms);
    }
  }

  _rowstart = _rowstart2 = (int)((320 - ST7789_TFTHEIGHT) / 2);
  _colstart = (int)((240 - ST7789_TFTWIDTH + 1) / 2);
  _colstart2 = (int)((240 - ST7789_TFTWIDTH) / 2);

  adafruit_st7789_set_rotation(adafruit_st7789_rotation_none);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Get the current display width. This value will be changed when the screen is
 * rotated.
 *****************************************************************************/
uint16_t adafruit_st7789_get_display_width(void)
{
  return _width;
}

/**************************************************************************//**
 * Get the current display height. This value will be changed when the screen is
 * rotated.
 *****************************************************************************/
uint16_t adafruit_st7789_get_display_height(void)
{
  return _height;
}

/**************************************************************************//**
 * Enable/Disable display color inversion.
 *****************************************************************************/
sl_status_t adafruit_st7789_invert_display(bool invert)
{
  return adafruit_st7789_spi_send_command(
    invert ? ST7789_INVON : ST7789_INVOFF, NULL, 0);
}

/**************************************************************************//**
 * Enable/Disable display on or off
 *****************************************************************************/
sl_status_t adafruit_st7789_enable_display(bool enable)
{
  return adafruit_st7789_spi_send_command(
    enable ? ST7789_DISPON : ST7789_DISPOFF, NULL, 0);
}

/**************************************************************************/

/*!
 *    @brief  Change whether TE pin output is on or off
 *    @param  enable True if you want the TE pin ON, false OFF
 */

/**************************************************************************/
sl_status_t adafruit_st7789_enable_tearing(bool enable)
{
  return adafruit_st7789_spi_send_command(
    enable ? ST7789_TEON : ST7789_TEOFF, NULL, 0);
}

/**************************************************************************/

/*!
 *    @brief  Change whether sleep mode is on or off
 *    @param  enable True if you want sleep mode ON, false OFF
 */

/**************************************************************************/
sl_status_t adafruit_st7789_enableSleep(bool enable)
{
  return adafruit_st7789_spi_send_command(
    enable ? ST7789_SLPIN : ST7789_SLPOUT, NULL, 0);
}

/**************************************************************************//**
 * Set the "address window" - the rectangle we will write to RAM.
 *****************************************************************************/
sl_status_t adafruit_st7789_set_addr_window(uint16_t x, uint16_t y,
                                            uint16_t w, uint16_t h)
{
  x += _xstart;
  y += _ystart;
  uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
  uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

  adafruit_st7789_spi_write_command(ST7789_CASET); // Column addr set
  adafruit_st7789_spi_write32(xa);

  adafruit_st7789_spi_write_command(ST7789_RASET); // Row addr set
  adafruit_st7789_spi_write32(ya);

  // Write to RAM
  return adafruit_st7789_spi_write_command(ST7789_RAMWR);
}

/**************************************************************************//**
 * Set the rotation of TFT
 *****************************************************************************/
sl_status_t adafruit_st7789_set_rotation(adafruit_st7789_rotation_e rotation)
{
  uint8_t madctl = 0;

  uint8_t _rotation = rotation % 4; // can't be higher than 3

  switch (_rotation) {
    case adafruit_st7789_rotation_none:
      madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
      _xstart = _colstart;
      _ystart = _rowstart;
      _width = ST7789_TFTWIDTH;
      _height = ST7789_TFTHEIGHT;
      break;
    case adafruit_st7789_rotation_90:
      madctl = ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
      _xstart = _rowstart;
      _ystart = _colstart2;
      _width = ST7789_TFTHEIGHT;
      _height = ST7789_TFTWIDTH;
      break;
    case adafruit_st7789_rotation_180:
      madctl = ST7789_MADCTL_RGB;
      _xstart = _colstart2;
      _ystart = _rowstart2;
      _width = ST7789_TFTWIDTH;
      _height = ST7789_TFTHEIGHT;
      break;
    case adafruit_st7789_rotation_270:
      madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
      _xstart = _rowstart2;
      _ystart = _colstart;
      _width = ST7789_TFTHEIGHT;
      _height = ST7789_TFTWIDTH;
      break;
  }

  return adafruit_st7789_spi_send_command(ST7789_MADCTL, &madctl, 1);
}

/**************************************************************************//**
 * Draw a single pixel to the display at requested coordinates.
 *****************************************************************************/
sl_status_t adafruit_st7789_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x >= 0) && (x < _width) && (y >= 0)
      && (y < _height)) {
    // THEN set up transaction (if needed) and draw...
    SPI_CS_LOW(gspi_handle);
    adafruit_st7789_set_addr_window(x, y, 1, 1);
    adafruit_st7789_spi_write16(color);
    SPI_CS_HIGH(gspi_handle);

    return SL_STATUS_OK;
  }

  return SL_STATUS_INVALID_PARAMETER;
}

/**************************************************************************//**
 * Fill the screen completely with one color.
 *****************************************************************************/
sl_status_t adafruit_st7789_fill_screen(uint16_t color)
{
  return adafruit_st7789_fill_rect(0,
                                   0,
                                   _width,
                                   _height,
                                   color);
}

/**************************************************************************//**
 * Draw a 16-bit image (565 RGB) at the specified (x,y) position.
 *****************************************************************************/
sl_status_t adafruit_st7789_draw_rgb_bitmap(int16_t x,
                                            int16_t y,
                                            uint16_t *color,
                                            int16_t w,
                                            int16_t h)
{
  if (color == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int16_t x2;
  int16_t y2;
  int16_t bx1 = 0;
  int16_t by1 = 0;
  int16_t save_w = w;

  if ((x >= _width)
      || (y >= _height)
      || ((x2 = (x + w - 1)) < 0)
      || ((y2 = (y + h - 1)) < 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (x < 0) {
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= _width) {
    w = _width - x;
  }
  if (y2 >= _height) {
    h = _height - y;
  }
  color += by1 * save_w + bx1;
  SPI_CS_LOW(gspi_handle);
  adafruit_st7789_set_addr_window(x, y, w, h);
  while (h--) {
    adafruit_st7789_write_pixels(color, w);
    color += save_w;
  }
  SPI_CS_HIGH(gspi_handle);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Fill the screen area  with color using SPI transmit DMA. After the process
 * is complete call the user callback to notify the higher layer.
 *****************************************************************************/
sl_status_t adafruit_st7789_flush_area_rgb565(int16_t x1, int16_t y1,
                                              int16_t x2, int16_t y2,
                                              uint8_t *pcolor,
                                              bool color_swap,
                                              void (*callback)(void *arg),
                                              void *callback_arg)
{
  if ((pcolor != NULL) && (callback != NULL)
      && (x1 >= 0) && (x1 < _width)
      && (x2 >= 0) && (x2 < _width)
      && (y1 >= 0) && (y1 < _height)
      && (y2 >= 0) && (y2 < _height)
      && (x2 >= x1) && (y2 >= y1)) {
    uint16_t width = x2 - x1 + 1;
    uint16_t hight = y2 - y1 + 1;

    SPI_CS_LOW(gspi_handle);
    adafruit_st7789_set_addr_window(x1, y1, width, hight);

    gtotal_pixel = width * hight;
    gpixel_transmit = 0;
    gpixel_transmit_counter = 0;
    color_swap_enabled = color_swap;
    flush_area_callback = callback;
    flush_area_callback_arg = callback_arg;

    if (gpixel_transmit_counter < gtotal_pixel) {
      uint32_t pixel_remaining = gtotal_pixel - gpixel_transmit_counter;
      gpixel_transmit = pixel_remaining > MAX_XFER_PIXEL_COUNT
                        ? MAX_XFER_PIXEL_COUNT : pixel_remaining;

      if (!color_swap_enabled) {
        uint16_t i;
        pColorSwap = pcolor;
        uint8_t colorData_H;
        uint8_t colorData_L;

        for (i = 0; i < gpixel_transmit; i++) {
          colorData_H =
            pColorSwap[((gpixel_transmit_counter + i) *  BYTE_PER_PIXEL) + 1];
          colorData_L =
            pColorSwap[((gpixel_transmit_counter + i) *  BYTE_PER_PIXEL)];

          /* Swap 2 bytes color data before sending to TFT LCD */
          dma_buffer[i] = colorData_H | (uint16_t)colorData_L << 8;
        }

        pColorBuffer = (uint8_t *)dma_buffer;
      } else {
        pColorBuffer = (uint8_t *)pcolor;
      }

      /* Start transmit data, the process is continued in
       * adafruit_st7789_flush_area_transmit_callback function
       */

      SPIDRV_MTransmit(gspi_handle,
                       (uint8_t *)pColorBuffer,
                       gpixel_transmit * BYTE_PER_PIXEL,
                       st7789_flush_area_transmit_callback);

      if (color_swap_enabled) {
        pColorBuffer += (gpixel_transmit * BYTE_PER_PIXEL);
      }

      gpixel_transmit_counter += gpixel_transmit;
    }

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

static void st7789_flush_area_transmit_callback(
  struct SPIDRV_HandleData *handle,
  Ecode_t transferStatus,
  int itemsTransferred)
{
  (void)itemsTransferred;

  if (handle == gspi_handle) {
    if (transferStatus == ECODE_OK) {
      if (gpixel_transmit_counter < gtotal_pixel) {
        /* Continue transmit the data */
        uint32_t pixel_remaining = gtotal_pixel - gpixel_transmit_counter;
        gpixel_transmit = pixel_remaining > MAX_XFER_PIXEL_COUNT
                          ? MAX_XFER_PIXEL_COUNT : pixel_remaining;

        if (!color_swap_enabled) {
          uint16_t i;
          uint8_t colorData_H;
          uint8_t colorData_L;

          for (i = 0; i < gpixel_transmit; i++) {
            colorData_H =
              pColorSwap[((gpixel_transmit_counter + i) *  BYTE_PER_PIXEL) + 1];
            colorData_L =
              pColorSwap[((gpixel_transmit_counter + i) *  BYTE_PER_PIXEL)];

            /* Swap 2 bytes color data before sending to TFT LCD */
            dma_buffer[i] = colorData_H | (uint16_t)colorData_L << 8;
          }

          pColorBuffer = (uint8_t *)dma_buffer;
        }
        SPIDRV_MTransmit(gspi_handle,
                         (uint8_t *)pColorBuffer,
                         gpixel_transmit * BYTE_PER_PIXEL,
                         st7789_flush_area_transmit_callback);

        if (color_swap_enabled) {
          pColorBuffer += (gpixel_transmit * BYTE_PER_PIXEL);
        }

        gpixel_transmit_counter += gpixel_transmit;
      } else {
        /* After the process is complete call the user callback to
         * notify the higher layer.
         */
        SPI_CS_HIGH(gspi_handle);

        if (flush_area_callback != NULL) {
          flush_area_callback(flush_area_callback_arg);
        }
      }
    }
  }
}
