/*
 * @*************************************: 
 * @FilePath     : /user/C/attribute__bitwise.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-08-27 11:07:19
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-27 11:18:18
 * @Descripttion :   __attribute__((bitwise)) 是 GCC 编译器的一个扩展属性，主要用于类型安全和防止位掩码的错误使用。
 * @compile      :  gcc   -Wall -Wextra -Wno-attributes attribute__bitwise.c 
 * @**************************************: 
 */


typedef enum __attribute__((bitwise)) {
    FLAG_READ    = (1 << 0),  // 1
    FLAG_WRITE   = (1 << 1),  // 2  
    FLAG_EXECUTE = (1 << 2)   // 4
} FilePermissions;

typedef enum __attribute__((bitwise)) {
    NET_FLAG_TCP = (1 << 0),  // 1
    NET_FLAG_UDP = (1 << 1),  // 2
    NET_FLAG_SSL = (1 << 2)   // 4
} NetworkFlags;




#include <stdio.h>

// 定义带bitwise属性的枚举
typedef enum __attribute__((bitwise)) {
    PERM_NONE    = 0,
    PERM_READ    = 1 << 0,  // 0001
    PERM_WRITE   = 1 << 1,  // 0010  
    PERM_EXECUTE = 1 << 2,  // 0100
    PERM_ALL     = PERM_READ | PERM_WRITE | PERM_EXECUTE  // 0111
} Permissions;

// 需要显式类型转换的函数
void set_permissions(Permissions perms) {
    printf("Setting permissions: %d\n", (int)perms);
}

int main() {
    // Permissions user_perms = PERM_READ | PERM_WRITE;  // ✓ 正确
    // printf("User permissions: %d\n", (int)user_perms);
    // set_permissions(user_perms);  // ✓ 正确

    // Permissions bad_perms = user_perms | 0x10;  // ✗ 编译错误
    // int raw_value = user_perms;  // ✗ 编译错误
    
    FilePermissions perms = FLAG_READ | FLAG_WRITE;  // 正确
    FilePermissions bad_perms = perms | NET_FLAG_TCP; // 编译器不会报错（危险！）
    
    
    // FilePermissions perms = FLAG_READ | FLAG_WRITE;      // ✓ 正确
    // FilePermissions bad_perms = perms | NET_FLAG_TCP;    // ✗ 编译错误！
    // error: incompatible types when initializing type 'FilePermissions' using type 'int's
    
    
    return 0;
}
