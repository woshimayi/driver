/*
 * @*************************************: 
 * @FilePath     : /user/C/elf_test/LD_PRELOAD_test.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-04-28 10:14:52
 * @LastEditors  : dof
 * @LastEditTime : 2025-04-28 10:22:27
 * @Descripttion :  
 * @compile      :  
 * 
 * 看到了吗？我们只用了十几行代码，就实现了一个能够监控任何程序内存分配的工具！
 *     这个例子的工作原理很简单：
 *          1:定义一个与系统函数同名的malloc
 *          2:用dlsym(RTLD_NEXT, "malloc")找到真正的 malloc 
 *          3:函数在调用真正的 malloc 前后添加我们的代码（这里是打印日志）
 *          4:通过LD_PRELOAD让系统优先加载我们的库
 *       
 *    这种技术经常用于：
 *                   调试内存问题
 *                   给程序添加日志
 *                   修改程序行为而不用改源码
 *                   临时修复运行中的服务
 *    当然，这项技术也常被黑客利用来劫持程序函数，所以理解它不仅能提升编程能力，也对安全防护很重要！
 *      
 *    build:
 *          $ gcc -shared -fPIC LD_PRELOAD_test.c -o libmemtrace.so -ldl
 *          $ LD_PRELOAD=./libmemtrace.so ./my_program
 * @**************************************: 
 */


 #define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

// 原始malloc函数指针
static void *(*real_malloc)(size_t) = NULL;

// 拦截 malloc 函数
void *malloc(size_t size)
{
    // 延迟初始化原始函数
    if (real_malloc == NULL)
    {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    // 调用原始malloc
    void *ptr = real_malloc(size);

    // 打印跟踪信息
    fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);

    return ptr;
}