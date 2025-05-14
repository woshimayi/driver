#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_QQ 4000000000ULL
#define BITMAP_SIZE (MAX_QQ / 8 + 1)

#undef PP
#define PP(fmt, ...) printf("\033[0;32;31m[%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)


unsigned char *bitmap;

// 设置 bitmap 中第 n 位的值为 1
void setBit(unsigned long long n) {
    if (n >= MAX_QQ) 
    {
        PP("Out of range");
        return;
    }
    PP("%d|%d", n / 8,  (1 << (n % 8)));
    bitmap[n / 8] |= (1 << (n % 8));
}

// 检查 bitmap 中第 n 位是否为 1
bool checkBit(unsigned long long n) {
    if (n >= MAX_QQ) return false;
    return (bitmap[n / 8] & (1 << (n % 8))) != 0;
}

int main() {
    bitmap = (unsigned char *)malloc(BITMAP_SIZE);
    if (bitmap == NULL) {
        perror("Failed to allocate memory for bitmap");
        return 1;
    }
    memset(bitmap, 0, BITMAP_SIZE); // 初始化为 0

    unsigned long long qq1 = 1234567890;
    unsigned long long qq2 = 1876543210;
    unsigned long long qq3 = 1234567890; // 重复的 QQ

    setBit(qq1);
    setBit(qq2);

    printf("QQ %llu 是否存在: %s\n", qq1, checkBit(qq1) ? "是" : "否");
    printf("QQ %llu 是否存在: %s\n", qq2, checkBit(qq2) ? "是" : "否");
    printf("QQ %llu 是否存在: %s\n", qq3, checkBit(qq3) ? "是" : "否");

    free(bitmap);
    return 0;
}