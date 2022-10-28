#ifndef _WS2812_H
#define _WS2812_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include "BMP.h"
#include "u8g2.h"

#define LED_NUM 60      //LED数量
#define NUM (24 * LED_NUM + 128)   //数据数量

#define ON  68
#define OFF 34

void LEDShow(uint8_t time);
void LEDCloseAll(void);
void ShowRainbow(uint8_t);
void WS281x_SetPixelColor(uint16_t n, uint32_t GRBColor);
void WS281x_SetPixelRGB(uint16_t n, uint8_t red, uint8_t green, uint8_t blue);
uint32_t Wheel(uint8_t WheelPos);
uint32_t Change(uint32_t RGB);
uint32_t WS281x_Color(uint8_t red, uint8_t green, uint8_t blue);
uint8_t ReadShow(FIL* fp, BITMAPFILEHEADER* Header, BMP_INFOHEADER* INFO,BMP_24 bmp24[IMG_WIDTH]);
void Show_BMP(u8g2_t *u8g2);
void Show_Color(u8g2_t *u8g2);
void Rainbow(u8g2_t *u8g2);
void SingleColor(u8g2_t *u8g2);
uint8_t SysConfig(u8g2_t *u8g2);
uint8_t Ctrl_Speed(u8g2_t *u8g2);
uint8_t Ctrl_Light(u8g2_t *u8g2);

#endif