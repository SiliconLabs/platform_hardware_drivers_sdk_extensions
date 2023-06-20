/***************************************************************************//**
* @file   mikroe_nt3h2111_i2c.h
* @brief  Types for I2C interface for NT3H2111.
********************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
********************************************************************************
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
********************************************************************************
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
*******************************************************************************/

#ifndef _MIKROE_NT3H2111_I2C_H_
#define _MIKROE_NT3H2111_I2C_H_

#include "sl_status.h"
#include "sl_i2cspm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup NT3H2111 Driver
 * @brief NT3H2111 I2C Types.
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @brief
 *  Read data through I2C.
 *
 * @param[in] i2c_handle
 *  The I2C instance to use
 *
 * @param[in] i2c_addr
 *  The I2C address to probe.
 *
 * @param[in] size
 *  Number of bytes to be read
 *
 * @param[out] pdata
 *  Buffer to store read data
 *
 * @returns
 *  I2C transfer result code.
 *****************************************************************************/
sl_status_t nt3h2111_i2c_read_bytes(sl_i2cspm_t *i2c_handle,
                                    uint8_t i2c_addr,
                                    uint32_t size,
                                    uint8_t *pdata);

/**************************************************************************//**
 * @brief
 *  Write data through I2C.
 *
 * @param[in] i2c_handle
 *  The I2C instance to use
 *
 * @param[in] i2c_addr
 *  The I2C address to probe.
 *
 * @param[in] size
 *  Number of bytes to be written
 *
 * @param[out] pdata
 *  Buffer of data to be written
 *
 * @returns
 *  I2C transfer result code.
 *****************************************************************************/
sl_status_t nt3h2111_i2c_write_bytes(sl_i2cspm_t *i2c_handle,
                                     uint8_t i2c_addr,
                                     uint32_t size,
                                     uint8_t *pdata);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup NT3H2111 I2C Driver) */
#endif
