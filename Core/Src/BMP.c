#include "BMP.h"

uint8_t ImgReadHeader(BITMAPFILEHEADER*Header, FIL* fp)
{
    uint32_t byteswritten;
    uint8_t buffer[100];
    f_read(fp, buffer, sizeof(BITMAPFILEHEADER), (UINT *) &byteswritten);
    //fread(buffer, sizeof(BITMAPFILEHEADER), 1, fp);
    Header->bfType =buffer[0]<<8| buffer[1];
    Header->bfSize = buffer[5] << 24 | buffer[4] << 16 | buffer[3] << 8 | buffer[4];
    Header->bfReserved1 = buffer[7] << 8 | buffer[6];
    Header->bfReserved2 = buffer[9] << 8 | buffer[8];
    Header->bfOffBits= buffer[13]<<24 | buffer[12] << 16 | buffer[11] << 8 | buffer[10];
    if (Header->bfType == 0x424D)
        return 1;
    return 0;
}

void ImgReadInfo(BMP_INFOHEADER* INFO, FIL* fp)
{
    uint32_t byteswritten;
//    fseek(fp, 14L, SEEK_SET);
    f_lseek(fp,14);
    uint8_t buffer[100];
    f_read(fp, buffer, sizeof(BMP_INFOHEADER), (UINT *) &byteswritten);
    // fread(buffer, sizeof(BMP_INFOHEADER), 1, fp);
    INFO->biSize= buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];
    INFO->biWidth= buffer[7] << 24 | buffer[6] << 16 | buffer[5] << 8 | buffer[4];
    INFO->biHeight = buffer[11] << 24 | buffer[10] << 16 | buffer[9] << 8 | buffer[8];
    INFO->biPlanes =buffer[13] << 8 | buffer[12];
    INFO->biBitCount =  buffer[15] << 8 | buffer[14];
    INFO->biCompression = buffer[19] << 24 | buffer[18] << 16 | buffer[17] << 8 | buffer[16];
    INFO->biSizeImage = buffer[23] << 24 | buffer[22] << 16 | buffer[21] << 8 | buffer[20];
    INFO->biXPelsPerMeter = buffer[27] << 24 | buffer[26] << 16 | buffer[25] << 8 | buffer[24];
    INFO->biYPelsPerMeter = buffer[31] << 24 | buffer[30] << 16 | buffer[29] << 8 | buffer[28];
    INFO->biClrUsed = buffer[35] << 24 | buffer[34] << 16 | buffer[33] << 8 | buffer[32];
    INFO->biClrImportant = buffer[39] << 24 | buffer[38] << 16 | buffer[37] << 8 | buffer[36];
}

uint8_t ImgReadData(FIL* fp, BITMAPFILEHEADER* Header, BMP_INFOHEADER* INFO,BMP_24 bmp24[][IMG_WIDTH])
{
    if (ImgReadHeader(Header, fp) == 0)
        return 0;
    ImgReadInfo(INFO,fp);
//    fseek(fp, Header->bfOffBits, SEEK_SET);
    f_lseek(fp,Header->bfOffBits);
    uint32_t byteswritten;
    if (INFO->biBitCount == 24)
    {
        uint8_t buffer[3];
        for (int i = 0; i < INFO->biHeight; i++)
        {
            for (int j = 0; j < INFO->biWidth; j++)
            {
                f_read(fp,buffer,3*sizeof(uint8_t),&byteswritten);
                // fread(buffer, sizeof(uint8_t), 3, fp);
                bmp24[i][j].r_val = buffer[2];
                bmp24[i][j].g_val = buffer[1];
                bmp24[i][j].b_val = buffer[0];
            }
        }
    }
    else
    {
        return 0;       //Read error (IMG Bitcount is not 24)
    }
    return 1;
}
