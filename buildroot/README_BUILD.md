
### 1:
xxx@xxx-virtual-machine:buildroot-2019.02.6$ find -name c-stack.c
./output/build/host-m4-1.4.18/lib/c-stack.c
c运行
解决方法：
搜索c-stack.c

```c
   #ifndef SIGSTKSZ
54 # define SIGSTKSZ 16384
55 //#elif HAVE_LIBSIGSEGV && SIGSTKSZ < 16384
56 ///* libsigsegv 2.6 through 2.8 have a bug where some architectures use
57 //   more than the Linux default of an 8k alternate stack when deciding
58 //   if a fault was caused by stack overflow.  */
59 //# undef SIGSTKSZ
60 //# define SIGSTKSZ 16384
61 #endif
```




### 2:
libfakeroot.c:99:40: error: '_STAT_VER' undeclared (first use in this function) 99 | #define INT_NEXT_STAT(a,b) NEXT_STAT64(_STAT_VER,a,b)
解决方法：

搜索：libfakeroot.c

```c
#ifndef _STAT_VER
 #if defined (__aarch64__)
  #define _STAT_VER 0
 #elif defined (__x86_64__)
  #define _STAT_VER 1
 #else
  #define _STAT_VER 3
 #endif
#endif
```
c运行