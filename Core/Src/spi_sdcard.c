#include "main.h"
#include "spi_sdcard.h"

uint8_t sd_type = 0;

/* SD 释放 */
static void sd_deselect(void)
{
    SD_CS(1);                       /* 取消SD卡片选 */
    sd_spi_read_write_byte(0xff);   /* 提供额外的8个时钟 */
}

/* SD 选中 */
static uint8_t sd_select(void)
{
    SD_CS(0);

    if (sd_wait_ready() == 0)
    {
        return SD_OK;   /* 等待成功 */
    }

    sd_deselect();
    return SD_ERROR;    /* 等待失败 */
}

/* 等待SD卡准备 */
static uint8_t sd_wait_ready(void)
{
    uint32_t t = 0;

    do
    {
        if (sd_spi_read_write_byte(0XFF) == 0XFF)
        {
            return SD_OK;   /* OK */
        }
        t++;
    } while (t < 0XFFFFFF); /* 等待 */

    return SD_ERROR;
}

/* 等待SD卡回应 */
static uint8_t sd_get_response(uint8_t response)
{
    uint16_t count = 0xFFFF;    /* 等待次数 */

    while ((sd_spi_read_write_byte(0XFF) != response) && count)
    {
        count--;    /* 等待得到准确的回应 */
    }

    if (count == 0)   /* 等待超时 */
    {
        return SD_ERROR;
    }

    return SD_OK;   /* 正确回应 */
}

/* 读取数据 */
static uint8_t sd_receive_data(uint8_t *buf, uint16_t len)
{
    if (sd_get_response(0xFE))   /* 等待SD卡发回数据起始令牌0xFE */
    {
        return SD_ERROR;
    }

    while (len--)   /* 开始接收数据 */
    {
        *buf = sd_spi_read_write_byte(0xFF);
        buf++;
    }

    /* 下面是2个伪CRC（dummy CRC） */
    sd_spi_read_write_byte(0xFF);
    sd_spi_read_write_byte(0xFF);

    return SD_OK;   /* 读取成功 */
}

/* 向SD卡发送一个命令 */
static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg)
{
    uint8_t res;
    uint8_t retry = 0;
    uint8_t crc = 0X01; /* 默认 CRC = 忽略CRC + 停止 */

    if (cmd & 0x80)     /* ACMD发送前, 需要先发送一个 CMD55 命令 */
    {
        cmd &= 0x7F;                    /* 清除最高位, 获取ACMD命令 */
        res = sd_send_cmd(CMD55, 0);    /* 发送CMD55 */

        if (res > 1)
        {
            return res;
        }
    }

    if (cmd != CMD12)   /* 当 cmd 不等于 多块读结束命令时(CMD12), 等待卡选中成功 */
    {
        sd_deselect();  /* 取消上次片选 */

        if (sd_select())
        {
            return 0xFF;/* 选中失败 */
        }
    }

    /* 发送命令包 */
    sd_spi_read_write_byte(cmd | 0x40); /* 起始 + 命令索引号 */
    sd_spi_read_write_byte(arg >> 24);  /* 参数[31 : 24] */
    sd_spi_read_write_byte(arg >> 16);  /* 参数[23 : 16] */
    sd_spi_read_write_byte(arg >> 8);   /* 参数[15 : 8] */
    sd_spi_read_write_byte(arg);        /* 参数[7 : 0] */

    if (cmd == CMD0) crc = 0X95;        /* CMD0 的CRC值固定为 0X95 */

    if (cmd == CMD8) crc = 0X87;        /* CMD8 的CRC值固定为 0X87 */

    sd_spi_read_write_byte(crc);

    if (cmd == CMD12)   /* cmd 等于 多块读结束命令(CMD12)时 */
        sd_spi_read_write_byte(0xff);   /* CMD12 跳过一个字节 */

    retry = 10; /* 重试次数 */

    do          /* 等待响应，或超时退出 */
    {
        res = sd_spi_read_write_byte(0xFF);
    } while ((res & 0X80) && retry--);

    return res; /* 返回状态值 */
}

/* 读取SD卡CID信息 */
uint8_t sd_get_cid(uint8_t *cid_data)
{
    uint8_t res;

    res = sd_send_cmd(CMD10, 0);            /* 发CMD10命令，读CID */

    if (res == 0x00)
        res = sd_receive_data(cid_data, 16);/* 接收16个字节的数据 */

    sd_deselect();  /* 取消片选 */

    return res;
}

/* 读取SD卡CSD信息 */
uint8_t sd_get_csd(uint8_t *csd_data)
{
    uint8_t res;
    res = sd_send_cmd(CMD9, 0);             /* 发CMD9命令，读CSD */

    if (res == 0)
    {
        res = sd_receive_data(csd_data, 16);/* 接收16个字节的数据 */
    }

    sd_deselect();  /* 取消片选 */

    return res;
}

/* 获取SD卡扇区数 */
uint32_t sd_get_sector_count(void)
{
    uint8_t csd[16];
    uint32_t capacity;
    uint8_t n;
    uint16_t csize;

    if (sd_get_csd(csd) != 0)       /* 取CSD信息，如果期间出错，返回0 */
    {
        return 0;                   /* 返回0表示获取容量失败 */
    }

    /* 如果为SDHC卡，按照下面方式计算 */
    if ((csd[0] & 0xC0) == 0x40)    /* V2.00的卡 */
    {
        csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
        capacity = (uint32_t)csize << 10;       /* 得到扇区数 */
    }
    else     /* V1.XX的卡 / MMC V3卡 */
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
        capacity = (uint32_t)csize << (n - 9);  /* 得到扇区数 */
    }

    return capacity;    /* 注意这里返回的是扇区数量, 不是实际容量的字节数, 换成字节数 得 * 512 */
}

/* SD卡初始化 */
uint8_t sd_init(void)
{
    uint8_t res;        /*  存放SD卡的返回值 */
    uint16_t retry;     /*  用来进行超时计数 */
    uint8_t ocr[4];
    uint16_t i;
    uint8_t cmd;

    sd_spi_speed_low(); /* 设置到低速模式 */

    for (i = 0; i < 10; i++)
    {
        sd_spi_read_write_byte(0XFF);       /* 发送最少74个脉冲 */
    }

    retry = 20;

    do
    {
        res = sd_send_cmd(CMD0, 0);         /* 进入IDLE状态 */
    } while ((res != 0X01) && retry--);

    sd_type = 0;        /* 默认无卡 */

    if (res == 0X01)
    {
        if (sd_send_cmd(CMD8, 0x1AA) == 1)   /* SD V2.0 */
        {
            for (i = 0; i < 4; i++)
            {
                ocr[i] = sd_spi_read_write_byte(0XFF);  /* 得到R7的32位响应 */
            }

            if (ocr[2] == 0X01 && ocr[3] == 0XAA)       /* 卡是否支持2.7~3.6V */
            {
                retry = 1000;

                do
                {
                    res = sd_send_cmd(ACMD41, 1UL << 30);   /* 发送ACMD41 */
                } while (res && retry--);

                if (retry && sd_send_cmd(CMD58, 0) == 0)    /* 鉴别SD2.0卡版本开始 */
                {
                    for (i = 0; i < 4; i++)
                    {
                        ocr[i] = sd_spi_read_write_byte(0XFF);  /* 得到OCR值 */
                    }

                    if (ocr[0] & 0x40)          /* 检查CCS */
                    {
                        sd_type = SD_TYPE_V2HC; /* V2.0 SDHC卡 */
                    }
                    else
                    {
                        sd_type = SD_TYPE_V2;   /* V2.0 卡 */
                    }
                }
            }
        }
        else     /* SD V1.x / MMC V3 */
        {
            res = sd_send_cmd(ACMD41, 0);   /* 发送ACMD41 */
            retry = 1000;

            if (res <= 1)
            {
                sd_type = SD_TYPE_V1;   /* SD V1卡 */
                cmd = ACMD41;           /* 命令等于 ACMD41 */
            }
            else     /* MMC卡不支持 ACMD41 识别 */
            {
                sd_type = SD_TYPE_MMC;  /* MMC V3 */
                cmd = CMD1;             /* 命令等于 CMD1 */
            }

            do
            {
                res = sd_send_cmd(cmd, 0);  /* 发送 ACMD41 / CMD1 */
            } while (res && retry--);   /* 等待退出IDLE模式 */

            if (retry == 0 || sd_send_cmd(CMD16, 512) != 0)
            {
                sd_type = SD_TYPE_ERR;  /* 错误的卡 */
            }
        }
    }

    sd_deselect();          /* 取消片选 */
    sd_spi_speed_high();    /* 高速模式 */

    if (sd_type)            /* 卡类型有效, 初始化完成 */
    {
        return SD_OK;
    }

    return res;
}

/* 读取SD卡 —— fatfs/usb可调用 */
uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t res;
    long long lsaddr = saddr;

    if (sd_type != SD_TYPE_V2HC)
    {
        lsaddr <<= 9;   /* 转换为字节地址 */
    }

    if (cnt == 1)
    {
        res = sd_send_cmd(CMD17, lsaddr);       /* 读命令 */

        if (res == 0)   /* 指令发送成功 */
        {
            res = sd_receive_data(pbuf, 512);   /* 接收512个字节 */
        }
    }
    else
    {
        res = sd_send_cmd(CMD18, lsaddr);       /* 连续读命令 */

        do
        {
            res = sd_receive_data(pbuf, 512);   /* 接收512个字节 */
            pbuf += 512;
        } while (--cnt && res == 0);

        sd_send_cmd(CMD12, 0);  /* 发送停止命令 */
    }

    sd_deselect();  /* 取消片选 */
    return res;
}