#ifndef SCL_STN1110_CONFIG_H_
#define SCL_STN1110_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

// <<< sl:start pin_tool >>>

// <gpio optional=true> STN1110_RESET
// $[GPIO_STN1110_RESET]
#warning "RESET pin for STN1110 is not configured"
// #define STN1110_RESET_PORT       gpioPortC
// #define STN1110_RESET_PIN        6
// [GPIO_STN1110_RESET]$

// <gpio optional=true> STN1110_INT
// $[GPIO_STN1110_INT]
#warning "INT pin for STN1110 is not configured"
// #define STN1110_INT_PORT         gpioPortB
// #define STN1110_INT_PIN          3
// [GPIO_STN1110_INT]$

// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif /* SCL_STN1110_CONFIG_H_ */
