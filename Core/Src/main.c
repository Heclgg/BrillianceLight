/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "u8g2.h"
#include "u8g2_Init.h"
#include "stdarg.h"
#include "stdio.h"
#include "spi_sdcard.h"
#include "malloc.h"
#include "WS2812.h"
#include "BMP.h"
#include "pic.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY_DOWN    HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin)
#define KEY         HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)
#define KEY_UP      HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
unsigned char filename[16];     //�ļ�???
u8g2_t u8g2;
uint32_t send_Buf[NUM] = {0};
FATFS *fs[1];
char *fn;   //�ļ�����
uint8_t time = 25;
uint16_t light = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void BMP_Select();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
BMP_24 img_bmp24[IMG_WIDTH];
BITMAPFILEHEADER HEADER;
BMP_INFOHEADER INFO;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    unsigned char work[520] = {0};
    uint8_t key = 0;
    uint32_t sd_size = 0;
    uint8_t res = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
    u8g2_Init(&u8g2);
    LEDCloseAll();
    SD_CS(1);
    sd_init();
    disk_initialize(0);
    my_mem_init(SRAMIN);     /* Ϊfatfs��ر��������ڴ� */

    /* ���� SD  */
    if(f_mount(&USERFatFS,"0:",1) == FR_OK)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);    //���سɹ����??? LED ����
//        UART_printf(&huart1,"f_mount sucess!!! \r\n");
        f_mkfs("0:",FM_FAT32,0,work,sizeof(work));

        /* ��ʾ�ļ������ļ� */
//        scan_files("0:");
//        UART_printf(&huart1,"%s\r\n", fn);
    }
    else
    {
        UART_printf(&huart1,"f_mount error: %d \r\n", retUSER);
        UART_printf(&huart1,"%d \r\n",f_mount(&USERFatFS,"0:",1));
        retUSER = f_mkfs("0:",FS_FAT32,0,work,sizeof(work));
        UART_printf(&huart1,"%d \r\n",f_mount(&USERFatFS,"0:",1));
        UART_printf(&huart1,"mkfs: %d \r\n",retUSER);
    }

    /* SD �����ز��ɹ� LED ��˸�����سɹ����� */
    while(f_mount(&USERFatFS,"0:",1) != FR_OK)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
        HAL_Delay(50);
        u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,25,32,"NO_SD");
        u8g2_SendBuffer(&u8g2);
    }
    HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);

    /* �������� */
//    retUSER = f_open(&USERFile, "1.bmp", FA_READ);
//    if(retUSER)
//        UART_printf(&huart1,"f_open file error : %d \r\n",retUSER);
//    else
//        UART_printf(&huart1,"f_open file sucess!!! \r\n");
//
//    if(ReadShow(&USERFile,&HEADER,&INFO,img_bmp24))
//        UART_printf(&huart1,"Draw finished!!! \r\n");
//    else
//        UART_printf(&huart1,"Draw failed! \r\n");
//
//    retUSER = f_close(&USERFile);
//    if(retUSER)
//        UART_printf(&huart1,"f_close error %d \r\n",retUSER);
//    else
//        UART_printf(&huart1,"Close Succeed! \r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  /* ��ʾ SD ������ */
//    sd_size = sd_get_sector_count();
//    UART_printf(&huart1,"SD size:%d MB\r\n",sd_size>>11);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      Begin_menu(&u8g2);
      key = key_scan(0);
      switch (key)
      {
          case 1:
              Show_BMP(&u8g2);
              break;
          case 2:
              SysConfig(&u8g2);
              break;
          case 3:
              Show_Color(&u8g2);
              break;
          default:
              ;
      }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/*
 * mode == 1 ʱ֧������
 * mode == 0 ʱ��֧������
*/
uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  /* �������ɿ���־ */
    uint8_t keyval = 0;

    if (mode) key_up = 1;       /* ֧������ */

    if (key_up && (KEY_DOWN == 0 || KEY == 0 || KEY_UP == 0))
    {
        HAL_Delay(5);
        key_up = 0;

        if (KEY_DOWN == 0)  keyval = 1;

        if (KEY == 0)       keyval = 2;

        if (KEY_UP == 0)    keyval = 3;
    }
    else if (KEY_DOWN == 1 && KEY == 1 && KEY_UP == 1) /* û���κΰ�������, ��ǰ����ɿ� */
    {
        key_up = 1;
    }

    return keyval;
}

int UART_printf(UART_HandleTypeDef *huart, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int length;
    char buffer[128];
    length = vsnprintf(buffer, 128, fmt, ap);
    HAL_UART_Transmit(huart,buffer,length,HAL_MAX_DELAY);
    va_end(ap);
    return length;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
