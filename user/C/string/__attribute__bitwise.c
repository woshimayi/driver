

#include <stdio.h>




int main(int argc, char const *argv[])
{
    
    typedef __attribute__((bitwise)) unsigned int flags_t;

    flags_t flags = 0x01; // 设置第一个标志位

    // 错误示例：将标志位与普通整数相加
    // unsigned int value = 10;
    // unsigned int result = flags + value; // Sparse 会发出警告

    // 正确示例：使用位操作设置标志位
    flags |= 0x02; // 设置第二个标志位

    return 0;
}

