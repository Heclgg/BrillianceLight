#ifndef WS2812_U8G2_INIT_H
#define WS2812_U8G2_INIT_H

#include "u8g2.h"

#define OLED_ADDRESS 0x78

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
void delay_us(uint32_t time);
void u8g2_Init(u8g2_t *u8g2);
void Begin_menu(u8g2_t *u8g2);

#endif //WS2812_U8G2_INIT_H
