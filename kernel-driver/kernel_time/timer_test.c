/* test-local.c
 *
 * Sample module for local.h usage.
 */

#include <asm/local.h>
#include <linux/module.h>
#include <linux/timer.h>

static DEFINE_PER_CPU(local_t, counters) = LOCAL_INIT(0);

struct timer_list test_timer;

/* IPI called on each CPU. */
static void test_each(void *info)
{
    /* Increment the counter from a non preemptible context */
    pr_debug("Increment on cpu %d\n", smp_processor_id());
    local_inc(this_cpu_ptr(&counters));

    /* This is what incrementing the variable would look like within a
     * preemptible context (it disables preemption) :
     *
     * local_inc(&get_cpu_var(counters));
     * put_cpu_var(counters);
     */
}

static void do_test_timer(unsigned long data)
{
    int cpu;

    /* Increment the counters */
    on_each_cpu(test_each, NULL, 1);
    /* Read all the counters */
    pr_debug("Counters read from CPU %d\n", smp_processor_id());
    for_each_online_cpu(cpu)
    {
        pr_debug("Read : CPU %d, count %ld\n", cpu,
                 local_read(&per_cpu(counters, cpu)));
    }
    mod_timer(&test_timer, jiffies + 10);
}

static int __init test_init(void)
{
    /* initialize the timer that will increment the counter */
    timer_setup(&test_timer, (void *)do_test_timer, 0);
    mod_timer(&test_timer, jiffies + 1);
    pr_err("Test module loaded jiffies = %d hz = %d\n", jiffies, HZ);

    return 0;
}

static void __exit test_exit(void)
{
    del_timer_sync(&test_timer);
    pr_err("Test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mathieu Desnoyers");
MODULE_DESCRIPTION("Local Atomic Ops");