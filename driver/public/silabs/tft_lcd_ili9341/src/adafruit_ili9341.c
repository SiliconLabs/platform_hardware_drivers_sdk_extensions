/***************************************************************************//**
 * @file adafruit_ili9341.c
 * @brief Adafruit ILI9341 TFT LCD with Touchscreen Source File
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
#if defined(SL_CATALOG_ADAFRUIT_TFT_LCD_ILI9341_PRESENT)
#include "em_usart.h"
#include "adafruit_ili9341_spi_usart_config.h"
#elif defined(SL_CATALOG_ADAFRUIT_TFT_LCD_ILI9341_DMA_PRESENT)
#include "spidrv.h"
#include "adafruit_ili9341_spi_dma_config.h"
#include "sl_spidrv_ili9341_config.h"
#endif

#include "adafruit_ili9341.h"

// -----------------------------------------------------------------------------
//                       Macros
// -----------------------------------------------------------------------------
#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))
#define _swap_int16_t(a, b) \
  {                         \
    int16_t t = a;          \
    a = b;                  \
    b = t;                  \
  }

#define SPI_CS_LOW() \
  GPIO_PinOutClear(ADAFRUIT_ILI9341_CS_PORT, ADAFRUIT_ILI9341_CS_PIN);
#define SPI_CS_HIGH() \
  GPIO_PinOutSet(ADAFRUIT_ILI9341_CS_PORT, ADAFRUIT_ILI9341_CS_PIN);

#define SPI_DC_LOW() \
  GPIO_PinOutClear(ILI9341_SPI_DC_PORT, ILI9341_SPI_DC_PIN);
#define SPI_DC_HIGH() \
  GPIO_PinOutSet(ILI9341_SPI_DC_PORT, ILI9341_SPI_DC_PIN);

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
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_ili9341_spi_send_command(uint8_t command,
                                                     uint8_t *data,
                                                     uint8_t len)
{
  SPI_CS_LOW();
  SPI_DC_LOW();
  USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, command);
  if (len != 0) {
    SPI_DC_HIGH();
    while (len--) {
      USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, *data);
      data++;
    }
  }
  SPI_CS_HIGH();

  return SL_STATUS_OK;
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
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_ili9341_spi_write_command(uint8_t cmd)
{
  SPI_DC_LOW();
  USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, cmd);
  SPI_DC_HIGH();

  return SL_STATUS_OK;
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
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_ili9341_spi_write16(uint16_t data)
{
  uint8_t bytes[2];

  bytes[0] = data >> 8;
  bytes[1] = data & 0x00FF;

  USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, bytes[0]);
  USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, bytes[1]);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *  Read 8 bits of data from display configuration memory (not RAM).
 *
 * @param[out] result
 *  Data read from display configuration memory.
 * @param[in] command
 *  The command register to read data from.
 * @param[in] index
 *  The byte index into the command to read from.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_ili9341_spi_read_command8(uint8_t *result,
                                                      uint8_t command,
                                                      uint8_t index)
{
  SPI_CS_LOW();
  SPI_DC_LOW();
  USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, command);
  SPI_DC_HIGH();
  do {
    *result = USART_SpiTransfer(ADAFRUIT_ILI9341_PERIPHERAL, 0x00);
  } while (index--); // Discard bytes up to index'th
  SPI_CS_HIGH();

  return SL_STATUS_OK;
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
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_ili9341_write_color(uint16_t color, uint32_t len)
{
  while (len--) {
    adafruit_ili9341_spi_write16(color);
  }

  return SL_STATUS_OK;
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
 *  SL_STATUS_FAIL if the process is failed.
 ******************************************************************************/
static sl_status_t adafruit_ili9341_write_pixels(uint16_t *colors, uint32_t len)
{
  if (!len) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  while (len--) {
    adafruit_ili9341_spi_write16(*colors);
    colors++;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *  A lower-level version of adafruit_ili9341_fill_rect(). This version
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
static sl_status_t adafruit_ili9341_write_fill_rect_preclipped(int16_t x,
                                                               int16_t y,
                                                               int16_t w,
                                                               int16_t h,
                                                               uint16_t color)
{
  adafruit_ili9341_set_addr_window(x, y, w, h);
  adafruit_ili9341_write_color(color, (uint32_t)w * h);

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
static sl_status_t adafruit_ili9341_fill_rect(int16_t x,
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
    if (x < ILI9341_TFTWIDTH) {
      if (h < 0) {
        y += h + 1;
        h = -h;
      }
      if (y < ILI9341_TFTHEIGHT) {
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
            if (x2 >= ILI9341_TFTWIDTH) {
              w = ILI9341_TFTWIDTH - x;
            }
            if (y2 >= ILI9341_TFTHEIGHT) {
              h = ILI9341_TFTHEIGHT - y;
            }
            SPI_CS_LOW();
            adafruit_ili9341_write_fill_rect_preclipped(x, y, w, h, color);
            SPI_CS_HIGH();
          }
        }
      }
    }
  }

  return SL_STATUS_OK;
}

static void spi_usart_init(void)
{
  USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

  usartInit.msbf = true;
  usartInit.clockMode = usartClockMode0;
  usartInit.master = true;
  usartInit.baudrate = ADAFRUIT_ILI9341_SPI_BITRATE;

#if defined(_CMU_HFPERCLKEN0_MASK)
  CMU_ClockEnable(cmuClock_HFPER, true);
#endif
  CMU_ClockEnable(cmuClock_GPIO, true);
#if ADAFRUIT_ILI9341_PERIPHERAL_NO == 0
  CMU_ClockEnable(cmuClock_USART0, true);
#elif ADAFRUIT_ILI9341_PERIPHERAL_NO == 1
  CMU_ClockEnable(cmuClock_USART1, true);
#elif ADAFRUIT_ILI9341_PERIPHERAL_NO == 2
  CMU_ClockEnable(cmuClock_USART2, true);
#endif

  usartInit.databits = usartDatabits8;
  USART_InitSync(ADAFRUIT_ILI9341_PERIPHERAL, &usartInit);

#if defined(USART_ROUTEPEN_TXPEN)
  ADAFRUIT_ILI9341_PERIPHERAL->ROUTELOC0 =
    (ADAFRUIT_ILI9341_PERIPHERAL->ROUTELOC0
     & ~(_USART_ROUTELOC0_TXLOC_MASK
         | _USART_ROUTELOC0_RXLOC_MASK
         | _USART_ROUTELOC0_CLKLOC_MASK))
    | (ADAFRUIT_ILI9341_TX_LOC  << _USART_ROUTELOC0_TXLOC_SHIFT)
    | (ADAFRUIT_ILI9341_RX_LOC  << _USART_ROUTELOC0_RXLOC_SHIFT)
    | (ADAFRUIT_ILI9341_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);

  ADAFRUIT_ILI9341_PERIPHERAL->ROUTEPEN = USART_ROUTEPEN_TXPEN
                                          | USART_ROUTEPEN_RXPEN
                                          | USART_ROUTEPEN_CLKPEN
                                          | USART_ROUTEPEN_CSPEN;
#else
  GPIO->USARTROUTE[ADAFRUIT_ILI9341_PERIPHERAL_NO].ROUTEEN =
    GPIO_USART_ROUTEEN_TXPEN
    | GPIO_USART_ROUTEEN_RXPEN
    | GPIO_USART_ROUTEEN_CLKPEN;
  GPIO->USARTROUTE[ADAFRUIT_ILI9341_PERIPHERAL_NO].TXROUTE =
    ((uint32_t)ADAFRUIT_ILI9341_TX_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
    | ((uint32_t)ADAFRUIT_ILI9341_TX_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[ADAFRUIT_ILI9341_PERIPHERAL_NO].RXROUTE =
    ((uint32_t)ADAFRUIT_ILI9341_RX_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
    | ((uint32_t)ADAFRUIT_ILI9341_RX_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[ADAFRUIT_ILI9341_PERIPHERAL_NO].CLKROUTE =
    ((uint32_t)ADAFRUIT_ILI9341_CLK_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
    | ((uint32_t)ADAFRUIT_ILI9341_CLK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);
#endif

  GPIO_PinModeSet(ADAFRUIT_ILI9341_TX_PORT,
                  ADAFRUIT_ILI9341_TX_PIN,
                  gpioModePushPull,
                  0);
  GPIO_PinModeSet(ADAFRUIT_ILI9341_RX_PORT,
                  ADAFRUIT_ILI9341_RX_PIN,
                  gpioModeInput,
                  0);
  GPIO_PinModeSet(ADAFRUIT_ILI9341_CLK_PORT,
                  ADAFRUIT_ILI9341_CLK_PIN,
                  gpioModePushPull,
                  0);
  GPIO_PinModeSet(ADAFRUIT_ILI9341_CS_PORT,
                  ADAFRUIT_ILI9341_CS_PIN,
                  gpioModePushPull,
                  0);
  GPIO_PinModeSet(ILI9341_SPI_DC_PORT,
                  ILI9341_SPI_DC_PIN,
                  gpioModePushPull,
                  0);
}

// -----------------------------------------------------------------------------
//                       Public Function
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Initialize the Adafruit 2.4" TFT LCD with Touchscreen.
 *****************************************************************************/
sl_status_t adafruit_ili9341_spi_usart_init(void)
{
  static uint8_t init_cmd[] = {
    0xEF, 3, 0x03, 0x80, 0x02,
    0xCF, 3, 0x00, 0xC1, 0x30,
    0xED, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 3, 0x85, 0x00, 0x78,
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00, 0x00,
    ILI9341_PWCTR1, 1, 0x23,               // Power control VRH[5:0]
    ILI9341_PWCTR2, 1, 0x10,               // Power control SAP[2:0];BT[3:0]
    ILI9341_VMCTR1, 2, 0x3e, 0x28,         // VCM control
    ILI9341_VMCTR2, 1, 0x86,               // VCM control2
    ILI9341_MADCTL, 1, 0x48,               // Memory Access Control
    ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
    ILI9341_PIXFMT, 1, 0x55,
    ILI9341_FRMCTR1, 2, 0x00, 0x18,
    ILI9341_DFUNCTR, 3, 0x08, 0x82, 0x27,  // Display Function Control
    0xF2, 1, 0x00,                         // 3Gamma Function Disable
    ILI9341_GAMMASET, 1, 0x01,              // Gamma curve selected
    ILI9341_GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,  // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    ILI9341_GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,  // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    ILI9341_SLPOUT, 0x80,                  // Exit Sleep
    ILI9341_DISPON, 0x80,                  // Display on
    0x00
  };

  spi_usart_init();

  adafruit_ili9341_spi_send_command(ILI9341_SWRESET, NULL, 0);
  sl_sleeptimer_delay_millisecond(150);

  uint8_t cmd, x, num_args;
  uint8_t *addr = init_cmd;
  while ((cmd = pgm_read_byte(addr++)) > 0) {
    x = pgm_read_byte(addr++);
    num_args = x & 0x7F;
    adafruit_ili9341_spi_send_command(cmd, addr, num_args);
    addr += num_args;
    if (x & 0x80) {
      sl_sleeptimer_delay_millisecond(150);
    }
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Enable/Disable display color inversion.
 *****************************************************************************/
sl_status_t adafruit_ili9341_invert_display(bool invert)
{
  return adafruit_ili9341_spi_send_command(
    invert ? ILI9341_INVON : ILI9341_INVOFF, NULL, 0);
}

/**************************************************************************//**
 * Scroll display memory.
 *****************************************************************************/
sl_status_t adafruit_ili9341_scroll_to(uint16_t y)
{
  uint8_t data[2];

  data[0] = y >> 8;
  data[1] = y & 0xff;

  return adafruit_ili9341_spi_send_command(ILI9341_VSCRSADD, data, 2);
}

/**************************************************************************//**
 * Set the height of the Top and Bottom Scroll Margins.
 *****************************************************************************/
sl_status_t adafruit_ili9341_set_scroll_margins(uint16_t top, uint16_t bottom)
{
  if (top + bottom <= ILI9341_TFTHEIGHT) {
    uint16_t middle = ILI9341_TFTHEIGHT - (top + bottom);
    uint8_t data[6];
    data[0] = top >> 8;
    data[1] = top & 0xff;
    data[2] = middle >> 8;
    data[3] = middle & 0xff;
    data[4] = bottom >> 8;
    data[5] = bottom & 0xff;
    return adafruit_ili9341_spi_send_command(ILI9341_VSCRDEF, data, 6);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }
}

/**************************************************************************//**
 * Set the "address window" - the rectangle we will write to RAM.
 *****************************************************************************/
sl_status_t adafruit_ili9341_set_addr_window(uint16_t x1, uint16_t y1,
                                             uint16_t w, uint16_t h)
{
  static uint16_t old_x1 = 0xffff, old_x2 = 0xffff;
  static uint16_t old_y1 = 0xffff, old_y2 = 0xffff;
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);

  if ((x1 != old_x1) || (x2 != old_x2)) {
    // Column address set
    adafruit_ili9341_spi_write_command(ILI9341_CASET);
    adafruit_ili9341_spi_write16(x1);
    adafruit_ili9341_spi_write16(x2);
    old_x1 = x1;
    old_x2 = x2;
  }
  if ((y1 != old_y1) || (y2 != old_y2)) {
    // Row address set
    adafruit_ili9341_spi_write_command(ILI9341_PASET);
    adafruit_ili9341_spi_write16(y1);
    adafruit_ili9341_spi_write16(y2);
    old_y1 = y1;
    old_y2 = y2;
  }
  // Write to RAM
  return adafruit_ili9341_spi_write_command(ILI9341_RAMWR);
}

/**************************************************************************//**
 * Read 8 bits of data from ILI9341 configuration memory.
 *****************************************************************************/
sl_status_t adafruit_ili9341_read_command8(uint8_t *result,
                                           uint8_t command,
                                           uint8_t index)
{
  uint8_t data = 0x10 + index;

  // Set Index Register
  adafruit_ili9341_spi_send_command(0xD9, &data, 1);
  adafruit_ili9341_spi_read_command8(result, command, 0);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Draw a single pixel to the display at requested coordinates.
 *****************************************************************************/
sl_status_t adafruit_ili9341_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x >= 0) && (x < ILI9341_TFTWIDTH) && (y >= 0)
      && (y < ILI9341_TFTHEIGHT)) {
    // THEN set up transaction (if needed) and draw...
    SPI_CS_LOW();
    adafruit_ili9341_set_addr_window(x, y, 1, 1);
    adafruit_ili9341_spi_write16(color);
    SPI_CS_HIGH();

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}

/**************************************************************************//**
 * Fill the screen completely with one color.
 *****************************************************************************/
sl_status_t adafruit_ili9341_fill_screen(uint16_t color)
{
  return adafruit_ili9341_fill_rect(0,
                                    0,
                                    ILI9341_TFTWIDTH,
                                    ILI9341_TFTHEIGHT,
                                    color);
}

/**************************************************************************//**
 * Draw a 16-bit image (565 RGB) at the specified (x,y) position.
 *****************************************************************************/
sl_status_t adafruit_ili9341_draw_rgb_bitmap(int16_t x,
                                             int16_t y,
                                             uint16_t *color,
                                             int16_t w,
                                             int16_t h)
{
  int16_t x2;
  int16_t y2;
  int16_t bx1 = 0;
  int16_t by1 = 0;
  int16_t save_w = w;

  if ((x >= ILI9341_TFTWIDTH)
      || (y >= ILI9341_TFTHEIGHT)
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
  if (x2 >= ILI9341_TFTWIDTH) {
    w = ILI9341_TFTWIDTH - x;
  }
  if (y2 >= ILI9341_TFTHEIGHT) {
    h = ILI9341_TFTHEIGHT - y;
  }
  color += by1 * save_w + bx1;
  SPI_CS_LOW();
  adafruit_ili9341_set_addr_window(x, y, w, h);
  while (h--) {
    adafruit_ili9341_write_pixels(color, w);
    color += save_w;
  }
  SPI_CS_HIGH();

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Fill the screen area  with color. After the process
 * is complete call the user callback to notify the higher layer.
 *****************************************************************************/
sl_status_t adafruit_ili9341_flush_area_rgb565(int16_t x1, int16_t y1,
                                               int16_t x2, int16_t y2,
                                               uint8_t *pcolor,
                                               bool color_swap,
                                               void (*callback)(void *arg),
                                               void *callback_arg)
{
  sl_status_t sc = SL_STATUS_FAIL;

  if ((pcolor == NULL) || (callback == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((x1 >= 0) && (x1 < ILI9341_TFTWIDTH)
      && (x2 >= 0) && (x2 < ILI9341_TFTWIDTH)
      && (y1 >= 0) && (y1 < ILI9341_TFTHEIGHT)
      && (y2 >= 0) && (y2 < ILI9341_TFTHEIGHT)
      && (x2 >= x1) && (y2 >= y1)) {
    uint16_t *p_color = (uint16_t *)pcolor;

    for (int16_t y = y1; y <= y2; y++) {
      for (int16_t x = x1; x <= x2; x++) {
        uint16_t color;

        if (color_swap) {
          color = *p_color >> 8 | ((uint16_t)(*p_color & 0xff) << 8);
        } else {
          color = *p_color;
        }
        sc = adafruit_ili9341_draw_pixel(x, y, color);
        if (SL_STATUS_OK != sc) {
          goto end;
        }
        p_color++;
      }
    }
  }
  end:
  callback(callback_arg);
  return sc;
}
