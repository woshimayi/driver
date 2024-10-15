/*
 * @*************************************: 
 * @FilePath     : /user/C/string/pthread_detach.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-10-11 10:00:55
 * @LastEditors  : dof
 * @LastEditTime : 2024-10-12 14:59:28
 * @Descripttion :  
 * @compile      :  
    * pthread_detach 的作用
    在多线程编程中，pthread_detach() 函数是一个非常重要的函数，它用于将一个线程设置为分离状态。

    什么是线程分离？
    非分离状态（joinable）： 默认情况下，线程处于非分离状态。这意味着主线程可以通过调用 pthread_join() 函数等待子线程结束，并获取子线程的返回值。
    分离状态： 当一个线程被设置为分离状态后，主线程就不能再使用 pthread_join() 来等待它结束。子线程结束时，其资源会自动被系统回收。

    pthread_detach 的作用
    资源回收： 当一个线程被设置为分离状态后，系统会自动回收该线程的资源，包括栈空间、线程属性等。这可以避免资源泄漏的问题。
    提高并发性： 分离状态的线程与主线程之间没有直接的依赖关系，主线程可以继续执行而无需等待子线程结束，从而提高程序的并发性。
    使用场景

    不需要获取子线程返回值： 当主线程不需要知道子线程的执行结果时，可以将子线程设置为分离状态。
    子线程生命周期较短： 对于生命周期较短的子线程，将其设置为分离状态可以简化程序的逻辑。

    大量线程： 当创建大量线程时，将线程设置为分离状态可以避免主线程阻塞过长时间。

    注意事项:
        一次性操作： 对同一个线程，只能调用一次 pthread_detach()。
        资源回收： 分离状态的线程结束时，其资源会自动回收，但全局变量等共享资源仍然可以被其他线程访问。
        线程安全： 如果多个线程访问共享资源，需要考虑线程同步问题。

    何时使用 pthread_detach()？:
        当你不需要获取子线程的返回值时。
        当子线程的生命周期较短时。
        当你创建大量线程时。
    
    何时不使用 pthread_detach()？
        当你需要获取子线程的返回值时。
        当你需要控制子线程的生命周期时。

 * @**************************************: 
 */
#include <pthread.h>
#include <stdio.h>

void* thread_func(void* arg) {
    sleep(3);
    printf("Hello from child thread\n");
    pthread_exit(NULL);
}

int main() {
    pthread_t tid;
    pthread_attr_t attr;

    // 设置线程属性为分离状态
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // 创建线程
    pthread_create(&tid, &attr, thread_func, NULL);
    printf("tid = %d\n", tid);

    // 主线程继续执行，无需等待子线程结束
    printf("Hello from main thread\n");

    while (1)
    {
        // 主线程可以执行其他任务
        sleep(1);
        if (tid)
        {
            printf("tid = %d\n", tid);
            pthread_mutex_destroy(tid);
        }
        pthread_create(&tid, &attr, thread_func, NULL);
        printf("tid = %d\n", tid);
        printf("Main thread is running\n");
        pthread_cancel(tid);             // 取消线程执行
    }

    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
}