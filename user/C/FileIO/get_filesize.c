/*
 * @*************************************: 
 * @FilePath: /user/C/FileIO/get_filesize.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-29 11:50:51
 * @LastEditors: dof
 * @LastEditTime: 2023-08-29 11:52:04
 * @Descripttion: 
 * @**************************************: 
 */


#if 1
#include <stdio.h>
#include <sys/stat.h>

int main() {
    struct stat st;
    if (stat("filename", &st) == 0) {
        printf("File size: %ld bytes\n", st.st_size);
    } else {
        printf("Failed to get file size\n");
    }
    return 0;
}
#else if 0

#include <stdio.h>

int main() {
    FILE *file = fopen("filename", "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fclose(file);
        printf("File size: %ld bytes\n", fileSize);
    } else {
        printf("Failed to open file\n");
    }
    return 0;
}
#else

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int file = open("filename", O_RDONLY);
    if (file != -1) {
        off_t fileSize = lseek(file, 0, SEEK_END);
        close(file);
        printf("File size: %ld bytes\n", (long)fileSize);
    } else {
        printf("Failed to open file\n");
    }
    return 0;
}

#endif