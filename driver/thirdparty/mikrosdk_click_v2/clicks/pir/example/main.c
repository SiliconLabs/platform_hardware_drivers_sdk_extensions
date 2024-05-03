/*!
 * \file 
 * \brief Pir Click example
 * 
 * # Description
 * This application which generates a voltage when exposed to infrared radiation.
 *
 * The demo application is composed of two sections :
 * 
 * ## Application Init 
 * Initializes device.
 * 
 * ## Application Task  
 * Reads ADC data, converts it to miliVolts and logs scaled value in miliVolts.
 * 
 * \author MikroE Team
 *
 */
// ------------------------------------------------------------------- INCLUDES

#include "board.h"
#include "log.h"
#include "pir.h"

// ------------------------------------------------------------------ VARIABLES

static pir_t pir;
static log_t logger;

// ------------------------------------------------------ APPLICATION FUNCTIONS

void application_init ( void )
{
    log_cfg_t log_cfg;
    pir_cfg_t cfg;

    /** 
     * Logger initialization.
     * Default baud rate: 115200
     * Default log level: LOG_LEVEL_DEBUG
     * @note If USB_UART_RX and USB_UART_TX 
     * are defined as HAL_PIN_NC, you will 
     * need to define them manually for log to work. 
     * See @b LOG_MAP_USB_UART macro definition for detailed explanation.
     */
    LOG_MAP_USB_UART( log_cfg );
    log_init( &logger, &log_cfg );
    log_info( &logger, "---- Application Init ----" );

    //  Click initialization.

    pir_cfg_setup( &cfg );
    PIR_MAP_MIKROBUS( cfg, MIKROBUS_1 );
    pir_init( &pir, &cfg );
}

void application_task ( void )
{
    uint16_t adc_val;
    float map_out;

    adc_val = pir_get_adc( &pir );
    map_out = pir_scale_results( &pir, adc_val, 0, 3303 );
    
    log_printf( &logger, " Voltage: %.2f  miliVolts \r\n", map_out);
   
    Delay_ms( 500 );
}

void main ( void )
{
    application_init( );

    for ( ; ; )
    {
        application_task( );
    }
}

// ------------------------------------------------------------------------ END
