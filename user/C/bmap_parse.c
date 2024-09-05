/*
 * @*************************************:
 * @FilePath     : /user/C/bmap_parse.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-27 13:23:14
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-27 15:17:13
 * @Descripttion :  C语言之图片.bmp读写示例
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>


#define BUF_SIZE 720 * 1280 * 3
#define IMG_NAME "fengjing720.bmp"
#define IMG_NEW "x1.bmp"

#pragma pack(push, 1) // 让结构体按照1字节对齐，确保读写顺序正确
typedef struct
{
    unsigned short bfType;      // 文件类型
    unsigned int bfSize;        // 文件大小
    unsigned short bfReserved1; // 保留字
    unsigned short bfReserved2; // 保留字
    unsigned int bfOffBits;     // 像素数据偏移
} BITMAPFILEHEADER;

typedef struct
{
    unsigned int biSize;         // 信息头大小
    int biWidth;                 // 图像宽度
    int biHeight;                // 图像高度
    unsigned short biPlanes;     // 平面数
    unsigned short biBitCount;   // 每个像素的位数
    unsigned int biCompression;  // 压缩类型
    unsigned int biSizeImage;    // 图像大小
    int biXPelsPerMeter;         // 水平分辨率
    int biYPelsPerMeter;         // 垂直分辨率
    unsigned int biClrUsed;      // 使用的颜色索引
    unsigned int biClrImportant; // 重要的颜色索引
} BITMAPINFOHEADER;
#pragma pack(pop)

BITMAPFILEHEADER bfHeader;
BITMAPINFOHEADER biHeader;

void readBMP(const char *filename, unsigned char *data, int size)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    // BITMAPFILEHEADER bfHeader;
    // BITMAPINFOHEADER biHeader;

    fread(&bfHeader, sizeof(bfHeader), 1, file);
    fread(&biHeader, sizeof(biHeader), 1, file);

    if (bfHeader.bfType != 0x4D42)
    {
        printf("Not a BMP file.\n");
        fclose(file);
        return;
    }

    printf("Width: %d, Height: %d Offset:%d\n", biHeader.biWidth, biHeader.biHeight, bfHeader.bfOffBits);

    // 移动到像素数据部分
    fseek(file, bfHeader.bfOffBits, SEEK_SET);

    // 计算像素数据大小
    //*size = biHeader.biWidth * biHeader.biHeight * 3; // 24位RGB
    // unsigned char *data = (unsigned char *)malloc(imageSize);
    if (!data)
    {
        printf("Memory allocation failed\n");
        fclose(file);
        return;
    }

    fread(data, 1, size, file);
    fclose(file);

    // 处理像素数据（例如，打印第一个像素的RGB值）
    printf("First pixel RGB: (%d, %d, %d)\n", data[0], data[1], data[2]);

    // free(data);
}

void writeBMP(const char *filename, unsigned char *data, int size)
{
    FILE *file = fopen(filename, "wb");

    if (!file)
    {
        perror("Unable to create file");
        return;
    }

    // 写入文件头和信息头
    fwrite(&bfHeader, sizeof(bfHeader), 1, file);
    fwrite(&biHeader, sizeof(biHeader), 1, file);

    // 写入像素数据
    fwrite(data, 1, size, file);
    fclose(file);
}

int main()
{
    char img_buf[BUF_SIZE];

    readBMP(IMG_NAME, img_buf, BUF_SIZE);
    printf("Width: %d, Height: %d Offset:%d\n", biHeader.biWidth, biHeader.biHeight, bfHeader.bfOffBits);
    writeBMP(IMG_NEW, img_buf, BUF_SIZE);

    return 0;
}