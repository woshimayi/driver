/*
 * @*************************************: 
 * @FilePath     : /kernel-driver/kprobe_test/kprobe_test.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-06-17 17:30:45
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-18 10:57:00
 * @Descripttion :  user  my_hi_cfe_alloc_car replace hi_cfe_alloc_car api, modify hi_cfe_alloc_car's param
 * @compile      :  
 * @**************************************: 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/version.h>
#include <asm/ptrace.h>

#define MAX_SYMBOL_LEN 64
static char symbol[MAX_SYMBOL_LEN] = "hi_cfe_alloc_car";
module_param_string(symbol, symbol, sizeof(symbol), 0644);


#undef PP
#define PP(fmt, ...) printk(KERN_ALERT  "[zzzzz :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__)



/* 获取符号地址的兼容方法 */
static unsigned long get_symbol_addr(const char *symname)
{
    struct kprobe kp = {
        .symbol_name = symname
    };
    unsigned long addr;
    
    if (register_kprobe(&kp) < 0)
        return 0;
    
    addr = (unsigned long)kp.addr;
    unregister_kprobe(&kp);
    
    return addr;
}


#if 1
typedef int32_t (*hi_cfe_alloc_car_t)(uint32_t);
hi_cfe_alloc_car_t orig_hi_cfe_alloc_car;

// 添加标志位防止递归调用
static int handler_depth = 0;  // 简单的递归深度计数器

int32_t my_hi_cfe_alloc_car(uint32_t rate)
{
    int32_t ret;
    PP( "my_hi_cfe_alloc_car: Before calling original function, rate=%u\n", rate);
    
    // 移除原子操作，因为现在用handler_depth来控制递归
    ret = orig_hi_cfe_alloc_car(rate);
    
    PP( "my_hi_cfe_alloc_car: After calling original function, ret = %d\n", ret);
    return ret;
}

int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    // 检查递归深度，防止无限递归
    if (handler_depth > 0) {
        PP( "handler_pre: Recursion detected (depth=%d), skipping\n", handler_depth);
        return 0;  // 直接返回，不处理
    }
    
    handler_depth++;  // 增加深度
    PP( "handler_pre: kprobe triggered for %s (depth=%d)\n", p->symbol_name, handler_depth);
    
    // 打印寄存器内容用于调试
    #ifdef CONFIG_ARM
    PP( "handler_pre: ARM registers - r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x\n", 
        regs->ARM_r0, regs->ARM_r1, regs->ARM_r2, regs->ARM_r3);
    #elif defined(CONFIG_ARM64)
    PP( "handler_pre: ARM64 registers - regs[0]=0x%lx, regs[1]=0x%lx, regs[2]=0x%lx, regs[3]=0x%lx\n", 
        regs->regs[0], regs->regs[1], regs->regs[2], regs->regs[3]);
    #elif defined(CONFIG_X86_64)
    PP( "handler_pre: X86_64 registers - rdi=0x%lx, rsi=0x%lx, rdx=0x%lx, rcx=0x%lx\n", 
        regs->di, regs->si, regs->dx, regs->cx);
    #elif defined(CONFIG_X86)
    PP( "handler_pre: X86 stack - sp=0x%lx, sp+4=0x%x, sp+8=0x%x\n", 
        regs->sp, *(uint32_t *)(regs->sp + 4), *(uint32_t *)(regs->sp + 8));
    #endif
    
    // 获取函数参数 - 只有一个参数rate
    uint32_t rate;
    
    // ARM架构参数传递约定：r0, r1, r2, r3
    #ifdef CONFIG_ARM
    rate = regs->ARM_r0;  // 第一个参数在r0
    PP( "handler_pre: ARM architecture, rate from r0\n");
    #elif defined(CONFIG_ARM64)
    rate = regs->regs[0];  // 第一个参数在regs[0]
    PP( "handler_pre: ARM64 architecture, rate from regs[0]\n");
    #elif defined(CONFIG_X86_64)
    rate = regs->di;  // 第一个参数在rdi
    PP( "handler_pre: X86_64 architecture, rate from rdi\n");
    #elif defined(CONFIG_X86)
    rate = *(uint32_t *)(regs->sp + 4);  // 第一个参数在栈上
    PP( "handler_pre: X86 architecture, rate from stack\n");
    #else
    rate = 0;
    PP( "handler_pre: Unknown architecture\n");
    #endif
    
    PP( "handler_pre: Function parameter - rate=%u (0x%x)\n", rate, rate);
    
    // 方案1: 只监控，不替换 - 让原始函数正常执行
    PP( "handler_pre: Monitoring only - letting original function execute\n");
    
    // 方案2: 如果你想替换函数，使用以下代码（但要注意递归问题）
    /*
    // 调用我们的替换函数
    int32_t ret = my_hi_cfe_alloc_car(rate);
    
    // 设置返回值
    #ifdef CONFIG_ARM
    regs->ARM_r0 = ret;
    #elif defined(CONFIG_ARM64)
    regs->regs[0] = ret;
    #elif defined(CONFIG_X86_64)
    regs->ax = ret;
    #elif defined(CONFIG_X86)
    regs->ax = ret;
    #endif
    
    // 跳过原始函数调用
    #ifdef CONFIG_ARM
    regs->ARM_pc = regs->ARM_lr;  // 返回到调用者
    #elif defined(CONFIG_ARM64)
    regs->pc = regs->regs[30];    // lr寄存器
    #elif defined(CONFIG_X86_64)
    regs->ip = *(unsigned long *)regs->sp;  // 返回地址
    regs->sp += 8;  // 调整栈指针
    #elif defined(CONFIG_X86)
    regs->ip = *(unsigned long *)regs->sp;
    regs->sp += 4;
    #endif
    */
    
    handler_depth--;  // 减少深度
    return 0;
}



// 添加post handler来监控函数返回值
void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    int32_t ret_value;
    
    // 获取返回值
    #ifdef CONFIG_ARM
    ret_value = regs->ARM_r0;
    #elif defined(CONFIG_ARM64)
    ret_value = regs->regs[0];
    #elif defined(CONFIG_X86_64)
    ret_value = regs->ax;
    #elif defined(CONFIG_X86)
    ret_value = regs->ax;
    #else
    ret_value = 0;
    #endif
    
    PP( "handler_post: Function %s returned %d (0x%x)\n", p->symbol_name, ret_value, ret_value);
}

#elif 0
typedef ssize_t (*vfs_read_t)(struct file *, char __user *, size_t, loff_t *);
static vfs_read_t orig_vfs_read;

static ssize_t my_vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    ssize_t ret;
    PP( "Before calling original vfs_read\n");
    
    ret = orig_vfs_read(file, buf, count, pos);
    
    PP( "After calling original vfs_read, ret = %d\n", ret);
    return ret;
}

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    #ifdef CONFIG_X86_64
    regs->ip = (unsigned long)my_vfs_read;
    #elif defined(CONFIG_X86)
    regs->eip = (unsigned long)my_vfs_read;
    #elif defined(CONFIG_ARM)
    regs->ARM_pc = (unsigned long)my_vfs_read;
    #elif defined(CONFIG_ARM64)
    regs->pc = (unsigned long)my_vfs_read;
    #endif
    return 0;
}
#endif


static struct kprobe kp = {
    .symbol_name = symbol,
    .pre_handler = handler_pre,
    .post_handler = handler_post,  // 添加post handler
};

static int __init kprobe_init(void)
{
    unsigned long orig_addr = get_symbol_addr(symbol);
    if (!orig_addr) {
        printk(KERN_ERR "Unable to find symbol: %s\n", symbol);
        return -ENOENT;
    }
    
    PP( "Found symbol %s at address %p\n", symbol, (void *)orig_addr);
    
    orig_hi_cfe_alloc_car = (hi_cfe_alloc_car_t)orig_addr;
    
    if (register_kprobe(&kp) < 0) {
        printk(KERN_ERR "register_kprobe failed for symbol: %s\n", symbol);
        return -EINVAL;
    }
    
    PP( "Successfully planted kprobe at %p for symbol %s\n", 
           (void *)kp.addr, symbol);
    PP( "Handler function at %p\n", my_hi_cfe_alloc_car);
    PP( "Recursion protection enabled with handler_depth counter\n");
    
    // 重置递归深度计数器
    handler_depth = 0;
    
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    PP( "kprobe unregistered\n");
}

module_init(kprobe_init);
module_exit(kprobe_exit);
MODULE_LICENSE("GPL");