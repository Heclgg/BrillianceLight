#include <tim.h>
#include "WS2812.h"
#include "BMP.h"
#include "main.h"
#include "usart.h"

extern BMP_INFOHEADER INFO;
extern uint32_t send_Buf[NUM];
extern BMP_24 img_bmp24[IMG_WIDTH];
extern uint8_t time;
extern uint16_t light;

/* DMA 发送 PWM  */
void LEDShow(uint8_t time)
{
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)send_Buf, NUM);
    HAL_Delay(time);
    HAL_TIM_PWM_Stop_DMA(&htim1,TIM_CHANNEL_1);
}

/* 关闭所有灯珠 */
void LEDCloseAll(void)
{
    uint32_t i;
    for (i = 0; i < LED_NUM * 24; i++)
        send_Buf[i] = OFF;
    for (i = LED_NUM * 24; i < NUM; i++)
        send_Buf[i] = 0;
    LEDShow(time);
}

/* 颜色转换 三色8位 RGB 转换为24位 GRB */
uint32_t WS281x_Color(uint8_t red, uint8_t green, uint8_t blue)
{
    return green << 16 | red << 8 | blue;
}

/* 24位 GRB 颜色写入 */
void WS281x_SetPixelColor(uint16_t n, uint32_t GRBColor)
{
    uint8_t i;
    if (n < LED_NUM)
        for (i = 0; i < 24; i++)
            send_Buf[24 * n + i] = (((GRBColor << i) & 0X800000) ? ON : OFF);
}

/* 三色8位 RGB 写入 */
void WS281x_SetPixelRGB(uint16_t n, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t i;
    if (n <= LED_NUM)
        for (i = 0; i < 24; i++)
            send_Buf[24 * n + i] = (((WS281x_Color(red - light, green - light, blue - light) << i) & 0X800000) ? ON : OFF);
}

/* 24位 RGB 转 GRB */
uint32_t Change(uint32_t RGB)
{
    uint8_t R,G,B;
    R = RGB >> 16;
    G = (RGB >> 8) & 0xff;
    B = RGB & 0xff;
    return G << 16 | R << 8 | B;
}

/**
 * 读取图片信息并显示
 * 返回 0 为错误 ， 返回 1 为成功
 */
uint8_t ReadShow(FIL* fp, BITMAPFILEHEADER* Header, BMP_INFOHEADER* INFO,BMP_24 bmp24[IMG_WIDTH])
{
    if (ImgReadHeader(Header, fp) == 0)
        return 0;
    ImgReadInfo(INFO,fp);
    f_lseek(fp,Header->bfOffBits);
    uint32_t byteswritten;

    if (INFO->biBitCount == 24)
    {
        LEDCloseAll();      //Close all WS2812 lights
        uint8_t buffer[3];
        for (int i = 0; i <= INFO->biHeight; i++)            // 图像高度
        {
            for (int j = INFO->biWidth - 1; j >= 0; j--)         // 图像宽度（灯带数目）
            {
                f_read(fp,buffer,3*sizeof(uint8_t),&byteswritten);
                img_bmp24[j].r_val = buffer[2];
                img_bmp24[j].g_val = buffer[1];
                img_bmp24[j].b_val = buffer[0];
                WS281x_SetPixelRGB(j,(img_bmp24[j].r_val),(img_bmp24[j].g_val), (img_bmp24[j].b_val));
//                UART_printf(&huart1,"r=%d,g=%d,b=%d  i=%d j=%d \r\n",img_bmp24[j].r_val,img_bmp24[j].g_val,img_bmp24[j].b_val,i,j);       //用于调试
            }
            LEDShow(time);
        }
    }
    else
        return 0;

    LEDCloseAll();
    return 1;       //Succeed
}

uint32_t Wheel(uint8_t WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return WS281x_Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return WS281x_Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return WS281x_Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

/**
 * 循环显示彩虹色
 */
void ShowRainbow(uint8_t wait)
{
    uint32_t timestamp = HAL_GetTick();
    uint16_t i;
    static uint8_t j;
    static uint32_t next_time = 0;

    uint32_t flag = 0;
    if (next_time < wait)
    {
        if ((uint64_t)timestamp + wait - next_time > 0)
            flag = 1;
    }
    else if (timestamp > next_time)
    {
        flag = 1;
    }
    if (flag) // && (timestamp - next_time < wait*5))
    {
        j++;
        next_time = timestamp + wait;
        for (i = 0; i < LED_NUM; i++)
        {
            WS281x_SetPixelColor(i, Wheel((i + j) & 255));
        }
    }
    LEDShow(time);
}

void Show_BMP(u8g2_t *u8g2)
{
    uint8_t buf[20];        //文件名
    uint8_t i = 0;          //文件索引
    uint8_t key;
    extern BITMAPFILEHEADER HEADER;

    u8g2_ClearBuffer(u8g2);
    u8g2_DrawStr(u8g2,36,32,"BMP");
    u8g2_SendBuffer(u8g2);

    while(1)
    {
        key = key_scan(0);
        u8g2_ClearBuffer(u8g2);
        if(key == 1)
        {
            HAL_Delay(5);
            i--;
            sprintf(buf,"%d.bmp",i);
            u8g2_DrawStr(u8g2,20,32,buf);
            u8g2_SendBuffer(u8g2);
        }
        if(key == 2)
        {
            HAL_Delay(5);
            f_open(&USERFile, buf, FA_READ);
            if(ReadShow(&USERFile,&HEADER,&INFO,img_bmp24))
            {
                u8g2_DrawStr(u8g2,20,32,"Finish");
                f_close(&USERFile);
//                UART_printf(&huart1,"Finished");
            }
            u8g2_SendBuffer(u8g2);
        }
        if(key == 3)
        {
            HAL_Delay(5);
            i++;
            sprintf(buf,"%d.bmp",i);
            u8g2_DrawStr(u8g2,20,32,buf);
            u8g2_SendBuffer(u8g2);
        }
    }
}

void Show_Color(u8g2_t *u8g2)
{
    uint8_t key;

    u8g2_ClearBuffer(u8g2);
    u8g2_DrawStr(u8g2,34,32,"Color");
    u8g2_SendBuffer(u8g2);

    while(1)
    {
        key = key_scan(0);

        if(key == 1)
            Rainbow(u8g2);
        if(key == 2)
            SingleColor(u8g2);
    }
}

void Rainbow(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);
    u8g2_DrawStr(u8g2,16,32,"Rainbow");
    u8g2_SendBuffer(u8g2);

    while(1)
        ShowRainbow(5);

}

void SingleColor(u8g2_t *u8g2)
{
    uint8_t i = 0;
    uint8_t key = 0;
    uint8_t buf[10];
    uint8_t color = 0;

    u8g2_ClearBuffer(u8g2);
    u8g2_DrawStr(u8g2,(128 - 12 * 7) / 2,32,"S_Color");
    u8g2_SendBuffer(u8g2);

    while(1)
    {
        for (i = 0; i < LED_NUM; i++)
            WS281x_SetPixelColor(i, Wheel(color & 255));
        LEDShow(time);

        key = key_scan(1);
        u8g2_ClearBuffer(u8g2);

        if(key == 1)
        {
            color -= 1;
            sprintf(buf,"%d",color);
            u8g2_DrawStr(u8g2,46,32,buf);
            u8g2_SendBuffer(u8g2);
        }

        if(key == 3)
        {
            color += 1;
            sprintf(buf,"%d",color);
            u8g2_DrawStr(u8g2,46,32,buf);
            u8g2_SendBuffer(u8g2);
        }
    }
}

uint8_t SysConfig(u8g2_t *u8g2)
{
    uint8_t key;

    u8g2_ClearBuffer(u8g2);
    u8g2_DrawStr(u8g2,(128 - 12 * 9) / 2,32,"SysConfig");
    u8g2_SendBuffer(u8g2);

    while(1)
    {
        key = key_scan(0);
        if(key == 1)
            if(Ctrl_Speed(u8g2))
                return key;
        if(key == 2)
            if(Ctrl_Light(u8g2) + 1)
                return key;
    }
}

uint8_t Ctrl_Speed(u8g2_t *u8g2)
{
    uint8_t buf[10];
    uint8_t key;

    u8g2_ClearBuffer(u8g2);
    sprintf(buf,"%d",time);
    u8g2_DrawStr(u8g2,5,50,"Time:");
    u8g2_DrawStr(u8g2,46,32,buf);
    u8g2_SendBuffer(u8g2);

    while(1)
    {
        key = key_scan(0);
        u8g2_ClearBuffer(u8g2);

        if(key == 1)
        {
            time--;
            sprintf(buf,"%d",time);
            u8g2_DrawStr(u8g2,5,50,"Time:");
            u8g2_DrawStr(u8g2,46,32,buf);
            u8g2_SendBuffer(u8g2);
        }
        if(key == 3)
        {
            time++;
            sprintf(buf,"%d",time);
            u8g2_DrawStr(u8g2,5,50,"Time:");
            u8g2_DrawStr(u8g2,46,32,buf);
            u8g2_SendBuffer(u8g2);
        }
        if(key == 2)
            return time;
    }
}

uint8_t Ctrl_Light(u8g2_t *u8g2)
{
    uint8_t buf[10];
    uint8_t key;

    u8g2_ClearBuffer(u8g2);
    sprintf(buf,"%d",light);
    u8g2_DrawStr(u8g2,5,50,"Light:");
    u8g2_DrawStr(u8g2,46,32,buf);
    u8g2_SendBuffer(u8g2);

    while(1)
    {
        key = key_scan(0);
        u8g2_ClearBuffer(u8g2);

        if(key == 1)
        {
            light--;
            sprintf(buf,"%d",light);
            u8g2_DrawStr(u8g2,5,50,"Light:");
            u8g2_DrawStr(u8g2,46,32,buf);
            u8g2_SendBuffer(u8g2);
        }
        if(key == 3)
        {
            light++;
            sprintf(buf,"%d",light);
            u8g2_DrawStr(u8g2,5,50,"light:");
            u8g2_DrawStr(u8g2,46,32,buf);
            u8g2_SendBuffer(u8g2);
        }
        if(key == 2)
            return light;
    }
}