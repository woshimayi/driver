/*
 * @*************************************:
 * @FilePath     : /kernel-driver/kprobe_test/kprobe_param_parse/kprobe_param_parse.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-06-18 14:44:09
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-18 14:44:20
 * @Descripttion :  kprobe mul param parse
 * @compile      :
 * @**************************************:
 */

// my_kprobe_module.c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h> // 包含sys_write等系统调用的声明
#include <linux/slab.h>     // 用于kmalloc
#include <linux/uaccess.h>  // 用于copy_from_user

// 声明我们要探测的系统调用函数
// 对于 sys_write，其符号通常是 __sys_write 或 sys_write
// 请根据你的内核版本和 arch/arm/kernel/calls.S 或 arch/arm/include/asm/unistd.h 来确认
// 这里的例子假设是 __sys_write
static struct kprobe kp = {
    .symbol_name = "__sys_write", // 替换为你想要探测的函数名
};

// pre_handler：在被探测函数执行之前被调用
// 参数：struct kprobe *p - 指向kprobe结构体
//       struct pt_regs *regs - 包含CPU寄存器状态的结构体
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    // 在ARM架构下，函数的第一个参数通常在r0寄存器中，第二个在r1，以此类推。
    // 在 pt_regs 结构体中，它们对应 ARM_r0, ARM_r1, ARM_r2, ARM_r3。

    // --- 示例：针对 sys_write(fd, buf, count) 的参数获取 ---
    // sys_write 原型大致是：ssize_t sys_write(unsigned int fd, const char __user *buf, size_t count);
    unsigned int fd = regs->ARM_r0;                                 // r0: 无符号整数类型
    const char __user *buf_ptr = (const char __user *)regs->ARM_r1; // r1: 用户空间指针类型，需要显式转换为指针
    size_t count = regs->ARM_r2;                                    // r2: size_t 类型 (通常是无符号整数)

    printk(KERN_INFO "KPROBE: %s: Called at %p, fd=%u, buf_ptr=%p, count=%zu\n",
           kp.symbol_name, p->addr, fd, buf_ptr, count);

    // 示例：从用户空间缓冲区读取数据 (谨慎操作，不能睡眠！)
    char kernel_buf[64]; // 较小的缓冲区，避免在栈上分配过多
    if (count > 0 && count < sizeof(kernel_buf))
    {
        // copy_from_user 是安全的，如果用户空间地址无效，它会返回非零。
        // 在kprobe handler中，避免任何可能引起页面错误并等待的操作。
        // 对于只读取数据的场景，probe_kernel_read 是一个更推荐的、不会引起页错误的替代。
        // 但这里为了通用性，仍使用 copy_from_user 演示。
        unsigned long ret = copy_from_user(kernel_buf, buf_ptr, count);
        if (ret == 0)
        {                             // 成功复制
            kernel_buf[count] = '\0'; // 确保null终止
            printk(KERN_INFO "KPROBE: Content (first %zu bytes): '%s'\n", count, kernel_buf);
        }
        else
        {
            printk(KERN_WARNING "KPROBE: Failed to copy %lu bytes from user buffer (ret=%lu).\n", (unsigned long)count, ret);
        }
    }

    // --- 更多不同类型参数的获取示例 (假设虚构函数) ---
    // 假设被探测函数原型:
    // int my_kernel_function(int arg_int, const char __user *arg_str, unsigned long arg_ulong, bool arg_bool, void *arg_ptr, long arg_long);
    // 并且我们只获取前4个寄存器传递的参数：
    // regs->ARM_r0 对应 int arg_int
    // regs->ARM_r1 对应 const char __user *arg_str
    // regs->ARM_r2 对应 unsigned long arg_ulong
    // regs->ARM_r3 对应 bool arg_bool (布尔值通常作为整数 0 或 1 传递)

    int arg_int = (int)regs->ARM_r0; // 将unsigned long强制转换为int
    // char __user *arg_str = (char __user *)regs->ARM_r1; // 已在上面处理
    unsigned long arg_ulong = regs->ARM_r2; // 直接赋值
    bool arg_bool = (bool)regs->ARM_r3;     // 将unsigned long转换为bool (0为false, 非0为true)

    // 如果有第五个参数或其他在栈上的参数，获取它们会非常复杂且不推荐，
    // 因为这需要精确地解析堆栈帧，并且依赖于ABI、编译器优化和内核版本。
    // 通常从 regs->ARM_sp (堆栈指针) 或 regs->ARM_fp (帧指针) 开始计算偏移量。

    // 打印这些虚构参数 (注意，这不会真的发生，只是类型转换示例)
    printk(KERN_INFO "KPROBE (Generic Args): arg_int=%d, arg_ulong=%lu, arg_bool=%d\n",
           arg_int, arg_ulong, (int)arg_bool);

    // 返回 0 表示让原始函数继续执行。
    // 返回非 0 会跳过原始函数的执行，直接返回到调用者（极度危险，除非你完全模拟了原始函数的所有行为）。
    return 0;
}

// post_handler：在被探测函数执行之后被调用
static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    // regs->ARM_r0 在 post_handler 中通常包含函数的返回值
    // 对于 sys_write，返回值是 ssize_t (写入的字节数或错误码)
    ssize_t ret_val = regs->ARM_r0;
    printk(KERN_INFO "KPROBE: %s: Post handler. Return value: %ld\n",
           kp.symbol_name, ret_val);
}

// fault_handler：在探测点发生异常时被调用 (可选)
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    printk(KERN_ERR "KPROBE: %s: fault_handler: p->addr = 0x%p, trap #%d\n",
           kp.symbol_name, p->addr, trapnr);
    /* Return 0 to indicate that everything is OK. */
    return 0;
}

static int __init kprobe_init(void)
{
    int ret;
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;

    ret = register_kprobe(&kp);
    if (ret < 0)
    {
        printk(KERN_ERR "register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    printk(KERN_INFO "kprobe registered on %s\n", kp.symbol_name);
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_INFO "kprobe unregistered from %s\n", kp.symbol_name);
}

module_init(kprobe_init);
module_exit(kprobe_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kprobe example for ARM argument retrieval.");
