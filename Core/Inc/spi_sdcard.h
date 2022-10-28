#ifndef __SPI_SD
#define __SPI_SD

#include "spi.h"
#include "system_stm32f4xx.h"

/* SD 片选引脚定义 */
#define SD_CS_GPIO_PORT                 GPIOA
#define SD_CS_GPIO_PIN                  GPIO_PIN_3
#define SD_CS_GPIO_CLK_ENABLE()         do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define sd_spi_read_write_byte(x)       spi1_read_write_byte(x)             /* SD卡 SPI读写函数 */
#define sd_spi_speed_low()              spi1_set_speed(7)                   /* SD卡 SPI低速模式 */
#define sd_spi_speed_high()             spi1_set_speed(0)                   /* SD卡 SPI高速模式 */

/* SD_CS 端口定义 */
#define SD_CS(x)   do{ x ? \
                      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_RESET); \
                   }while(0)    /* SD_CS */

/* SD卡 返回值定义 */
#define SD_OK           0
#define SD_ERROR        1

/* SD卡 类型定义 */
#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06

/* SD卡 命令定义 */
#define CMD0    (0)             /* GO_IDLE_STATE */
#define CMD1    (1)             /* SEND_OP_COND (MMC) */
#define	ACMD41  (0x80 + 41)     /* SEND_OP_COND (SDC) */
#define CMD8    (8)             /* SEND_IF_COND */
#define CMD9    (9)             /* SEND_CSD */
#define CMD10   (10)            /* SEND_CID */
#define CMD12   (12)            /* STOP_TRANSMISSION */
#define ACMD13  (0x80 + 13)     /* SD_STATUS (SDC) */
#define CMD16   (16)            /* SET_BLOCKLEN */
#define CMD17   (17)            /* READ_SINGLE_BLOCK */
#define CMD18   (18)            /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)            /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80 + 23)     /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)            /* WRITE_BLOCK */
#define CMD25   (25)            /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)            /* ERASE_ER_BLK_START */
#define CMD33   (33)            /* ERASE_ER_BLK_END */
#define CMD38   (38)            /* ERASE */
#define CMD55   (55)            /* APP_CMD */
#define CMD58   (58)            /* READ_OCR */

/* SD卡的类型 */
extern uint8_t  sd_type;


/* 函数申明 */
static void sd_deselect(void);                              /* SD卡取消选中 */
static uint8_t sd_select(void);                             /* SD卡 选中 */
static uint8_t sd_wait_ready(void);                         /* 等待SD卡准备好 */
static uint8_t sd_get_response(uint8_t response);           /* 等待SD卡回应 */

static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg);      /* SD卡发送命令 */
static uint8_t sd_receive_data(uint8_t *buf, uint16_t len); /* SD卡接收一次数据 */

uint8_t sd_init(void);                                      /* SD 卡初始化 */
uint32_t sd_get_sector_count(void);                         /* 获取SD卡的总扇区数(扇区数) */
uint8_t sd_get_cid(uint8_t *cid_data);                      /* 获取SD卡的CID信息 */
uint8_t sd_get_csd(uint8_t *csd_data);                      /* 获取SD卡的CSD信息 */

uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);  /* 读SD卡(fatfs/usb调用) */

#endif