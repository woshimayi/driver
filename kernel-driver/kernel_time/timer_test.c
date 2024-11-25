/*
 * @*************************************:
 * @FilePath     : /kernel-driver/kernel_timer/timer_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-16 19:24:26
 * @LastEditors  : dof
 * @LastEditTime : 2024-11-06 19:46:02
 * @Descripttion :  参考内核文档目录  Documentation/core-api/local_ops.rst 中的例子，do_test_timer 加(void *) 强转
 * @compile      :
 * @**************************************:
 */

#include <asm/local.h>
#include <linux/module.h>
#include <linux/timer.h>

#include <linux/ktime.h>
#include <linux/time.h>

static DEFINE_PER_CPU(local_t, counters) = LOCAL_INIT(0);

static struct timer_list test_timer;

/* IPI called on each CPU. */
static void test_each(void *info)
{
    /* Increment the counter from a non preemptible context */
    printk(KERN_ERR "Increment on cpu %d\n", smp_processor_id());
    local_inc(this_cpu_ptr(&counters));

    /* This is what incrementing the variable would look like within a
     * preemptible context (it disables preemption) :
     *
     * local_inc(&get_cpu_var(counters));
     * put_cpu_var(counters);
     */
}
int get_current_minute(void) {
    time64_t stamp;
	stamp = ktime_get_real_seconds();
	printk("stamp = %lld, %ld", stamp, sys_tz.tz_minuteswest);
	struct tm tm;
	time64_to_tm(stamp, 0, &tm);
	printk("%ld-%d-%d %d:%d:%d\n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
    printk("%d\n", tm.tm_hour*3600+ tm.tm_min*60+ tm.tm_sec);

    return tm.tm_min;
}

static void do_test_timer(unsigned long data)
{
    int cpu;

    /* Increment the counters */
    on_each_cpu(test_each, NULL, 1);
    /* Read all the counters */
    printk(KERN_ERR "Counters read from CPU %d\n", smp_processor_id());
    for_each_online_cpu(cpu)
    {
        printk(KERN_ERR "Read : CPU %d, count %ld\n", cpu,
               local_read(&per_cpu(counters, cpu)));
    }
    mod_timer(&test_timer, jiffies + 1000);

    get_current_minute();
}

static int __init test_init(void)
{
    /* initialize the timer that will increment the counter */
    timer_setup(&test_timer, (void *)do_test_timer, 0);
    mod_timer(&test_timer, jiffies + 1);

    return 0;
}

static void __exit test_exit(void)
{
    timer_shutdown_sync(&test_timer);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mathieu Desnoyers");
MODULE_DESCRIPTION("Local Atomic Ops");
