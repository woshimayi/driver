#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define P_debug(arg...) printf("%s %d\n", __FUNCTION__, __LINE__, arg)

#define P_error(arg...) printf(__FUNCTION__, __LINE__)

// #define XMLFUNC(XX) static XML_STATUS XX (const char *, TOKEN_TYPE, const char *)

#define PP(fmt, ...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)

// struct file_operations ext2_file_operations = {
//	.llseek = generic_file_llseek,
//	.read = generic_file_read,
//	.write = generic_file_write,
//	.aio_read = generic_file_aio_read,
//	.aio_write = generic_file_aio_write,
//	.ioct = ext2_ioctl,
//	.mmap = generic_file_mmap,
//	.open = generic_file_open,
//	.release = ext2_release_file,
//	.fsync = ext2_sync_file,
//	.readv = generic_file_readv,
//	.writev = generic_file_writev,
//	.sendfile = generic_file_sendfile,
// };

typedef enum
{
    XML_STS_OK = 0,
    XML_STS_ERR
} XML_STATUS;

typedef enum
{
    TOKEN_INVALID,
    TAGBEGIN,
    TAGEND,
    TAGDATA,
    ATTRIBUTE,
    ATTRIBUTEVALUE
} TOKEN_TYPE;

// static XML_STATUS fdownload(const char *str, TOKEN_TYPE i, const char *str1)
// {
//     printf("sdfsd\n");
// }

int main()
{
    P_debug("%s %d\n", "sdf", 23);
    P_error("%s %d\n", "sdf", 23);
    PP();

    float a = 1.234;
    unsigned long b = 0;
    float c = 0.0;
    printf("%d %f %ld %d\n", sizeof(int), sizeof(float), sizeof(unsigned long), sizeof(a));
    memcpy(&b, &a, sizeof(a));
    memcpy(&c, &b, sizeof(c));
    printf("a = %f b = %ld, c = %f\n", a, b, c);

    // printf("%s\n", XMLFUNC(fdownload));

    int i;
    PP("i = %p", i);

    return 0;
}
