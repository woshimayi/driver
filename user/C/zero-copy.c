/*
 * @*************************************: 
 * @FilePath     : /user/C/zero-copy.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-08-28 20:43:31
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-28 21:05:39
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */

// 文件传输：优先使用 sendfile

// 任意fd传输:使用 splice

// 数据复制：使用 tee

// 高性能网络：考虑 AF_XDP 或 RDMA

// 兼容性要求：使用 mmap + write


#include <sys/mman.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <fcntl.h>

/**
 * @brief 优点：减少一次内核到用户空间的拷贝
 *        缺点：仍然需要CPU参与数据拷贝
 * 
 * @param sockfd 
 * @param filename 
 */
void send_file_mmap(int sockfd, const char* filename) {
    int fd = open(filename, O_RDONLY);
    off_t file_size = lseek(fd, 0, SEEK_END);
    
    // 内存映射文件
    char* file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    // 直接发送映射的内存数据
    send(sockfd, file_data, file_size, 0);
    
    munmap(file_data, file_size);
    close(fd);
}

/**
 * @brief 优点：完全零拷贝，性能最好
 *        缺点：只能用于文件到socket的传输
 * 
 * @param sockfd 
 * @param filename 
 */
void send_file_sendfile(int sockfd, const char* filename) {
    int fd = open(filename, O_RDONLY);
    off_t file_size = lseek(fd, 0, SEEK_END);
    off_t offset = 0;
    
    // 零拷贝发送文件
    sendfile(sockfd, fd, &offset, file_size);
    close(fd);
}



#include <fcntl.h>
#include <unistd.h>

/**
 * @brief 优点：支持任意fd间传输，更灵活
 *        缺点：需要管道作为中间缓冲
 * 
 * @param source_fd 
 * @param dest_fd 
 */
void pipe_splice(int source_fd, int dest_fd) {
    int pipes[2];
    pipe(pipes);
    
    // 使用splice在fd之间移动数据
    while (1) {
        ssize_t bytes = splice(source_fd, NULL, pipes[1], NULL, 4096, SPLICE_F_MOVE);
        if (bytes <= 0) break;
        
        splice(pipes[0], NULL, dest_fd, NULL, bytes, SPLICE_F_MOVE);
    }
    
    close(pipes[0]);
    close(pipes[1]);
}


#include <fcntl.h>
#include <unistd.h>


/**
 * @brief 原理：在两个管道之间复制数据，不消耗数据 
 * 优点：可以复制数据到多个目的地
 * 缺点：使用相对复杂
 * 
 * @param source_fd 
 * @param dest_fd1 
 * @param dest_fd2 
 */
void duplicate_data(int source_fd, int dest_fd1, int dest_fd2) {
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    
    // 使用tee复制数据
    while (1) {
        ssize_t bytes = tee(source_fd, pipe1[1], 4096, SPLICE_F_NONBLOCK);
        if (bytes <= 0) break;
        
        // 数据可以同时发送到两个目的地
        splice(pipe1[0], NULL, dest_fd1, NULL, bytes, SPLICE_F_MOVE);
        splice(pipe1[0], NULL, dest_fd2, NULL, bytes, SPLICE_F_MOVE);
    }
}

// 网卡支持的直接数据放置 (DDP)
// 原理: 网卡直接从内存读取数据, 完全绕过CPU
// 需要特定的硬件和驱动支持
// 通常通过RDMA（Remote Direct Memory Access）实现
// 优点：真正的零拷贝，性能极致
// 缺点：需要特殊硬件支持


// 使用AF_XDP socket
// 原理: eBPF和XDP提供的零拷贝网络包处理
// 需要Linux 4.18+内核支持
// 使用eBPF程序直接处理网络包
// 优点：高性能网络处理
// 缺点: 实现复杂,需要eBPF知识