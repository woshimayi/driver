#include    <sys/types.h>
#include    <sys/stat.h>
#include    <sys/mman.h>    /* mmap() */
#include    <fcntl.h>
#include	<stdio.h>
#include <stdlib.h>
#include <string.h>
//#include    "ourhdr.h"

#ifndef    MAP_FILE    /* 44BSD defines this & requires it to mmap files */
#define    MAP_FILE    0    /* to compile under systems other than 44BSD */
#endif

int
main(int argc, char *argv[])
{
    int            fdin, fdout;
    char        *src, *dst;
    struct stat    statbuf;

    if (argc != 3)
        printf("usage: a.out <fromfile> <tofile>");

    if ( (fdin = open(argv[1], O_RDONLY)) < 0)
        printf("can't open %s for reading", argv[1]);

    if ( (fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC,
                                               "w")) < 0)
        printf("can't creat %s for writing", argv[1]);

    if (fstat(fdin, &statbuf) < 0)    /* need size of input file */
        printf("fstat error");

            /* set size of output file */
    if (lseek(fdout, statbuf.st_size - 1, SEEK_SET) == -1)
        printf("lseek error");
    if (write(fdout, "", 1) != 1)
        printf("write error");

    if ( (src = mmap(0, statbuf.st_size, PROT_READ,
                     MAP_FILE | MAP_SHARED, fdin, 0)) == (caddr_t) -1)
        printf("mmap error for input");

    if ( (dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE,
                     MAP_FILE | MAP_SHARED, fdout, 0)) == (caddr_t) -1)
        printf("mmap error for output");

    memcpy(dst, src, statbuf.st_size);    /* does the file copy */

    exit(0);
}
