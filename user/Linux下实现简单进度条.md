Linux下实现简单进度条
原创LLZK_ 发布于2016-12-31 21:28:48 阅读数 912  收藏
展开

我们在使用计算机不免有拷贝文件或者下载文件的时候，总会看到行行色色的进度条，如下图的两种：

下载文件：

![这里写图片描述](https://img-blog.csdn.net/20161231190810560?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvTExaS18=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

拷贝文件：

![这里写图片描述](https://img-blog.csdn.net/20161231190849023?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvTExaS18=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)


在实现进度条之前首先要明白两个问题：

回车与换行的区别
printf的缓冲区问题
可能你不明白为什么一个小小的进度条关这俩兄弟什么事，但请耐心往下看！

回车与换行
在计算机还没有出现之前，有一种叫做电传打字机（Teletype Model 33）的东西，每秒钟可以打10个字符。但是它有一个问题，就是打完一行换行的时候要用去0.2秒，正好可以打两个字符。要是在这0.2秒里面，又有新的字符传过来，那么这个字符将会丢失。
为了解决这个问题，研制人员想了个办法，在每行的后面加两个表示结束的字符。

换行—-告诉打字机把打印头定位在左边界；
回车—-告诉打字机把纸向下移动一行；
后来这个发明就被延续到了计算机上，但是因为系统的不同，设计也有所不同。

Unix系统—-结尾只有“换行”—-‘\n’;
Mac系统—-结尾只有“回车”—-‘\r’;
Windows系统—-结尾有是“回车”“换行”—-‘\r\n’;
所以，回车是‘\r’,而换行是‘\n’。我们平时在Windows下敲一下Enter键实际上是输入了两个字符‘\r\n’。

这对我们实现进度条有什么用？
我们实现的这个简答进度条的原理是将不同的字符数组以较短的时间t为间隔，依次打印输出至屏幕。
如我定义一个字符数组并将其第一个字符初始化为‘\0’,将其以字符串的形式输出至屏幕：

char bar[102];
bar[0] = '\0';
printf("%s")
1
2
3
我们知道，字符串以‘\0’为结尾，所以上面这段伪代码的结果是输出一段空字符串。下面我再做点小修饰：

```c
char bar[102];
int rate = 0;
bar[0] = '\0';
printf("%s")
while(rate <= 100)
{
    printf("[%s] \n",bar);
    sleep(3);
    bar[rate] = '=';
    rate++;
    bar[rate] = '\0';
}
```

注意，此时我在字符串的后面加了一个‘\n’，进行了换行操作，所以你会看到下面的输出结果：

```
[===]
[====]
[=====]
[======]
```


如果我将‘\n’改成‘\r’呢？当第一次的字符串打印完毕后 ，不换行，执行”回车“操作，直接到这一行的开头进行第二次打印，覆盖之前的内容。。。就这样一直到循环结束。

```c
char bar[102];
int rate = 0;
bar[0] = '\0';
printf("%s")
while(rate <= 100)
{
    printf("[%s] \n",bar);
    sleep(3);
    bar[rate] = '=';
    rate++;
    bar[rate] = '\0';
}
```

想象一下，输出结果是什么？
你会得到一个递增的进度条？不！你什么也得不到，请继续往下看！

printf的缓冲区问题
在Linux下实现一个动态进度条只是上面的还不够，在上面的代码中有这两行：

```c
printf("[%s] \r",bar);
sleep(3);
```

这两句实现出来就是先输出，然后等待3s，再执行下面语句。如果你将这两句单独拿出来实现以下就会发现，它的实际执行结果是，先等待3s，再输出。很奇怪是不是！这就涉及到了printf的IO缓冲区了。

在printf的实现中有一步调用write的操作。而write是一个系统调用，系统调用是软中断，频繁调用会使内核频繁陷入内核态，效率不是很高，所以printf的实现中在调用write之前，加了一个IO缓冲区。printf输出数据的时候实际上是先往用户空间的IO缓冲区写，在满足条件的情况下才会调用write并且刷新缓冲区，这样会提高内核工作的效率。
满足条件的情况有以下几种：

缓冲区填满；
写入的字符中有‘\n’；
调用fflush函数手动刷新缓冲区；
调用scanf要从缓冲区中读数据时，也会将缓冲区的数据刷新；
printf语句生命结束时；
满足上面任意一个条件，缓冲区都会进行刷新，然后将数据输出至屏幕。缓冲区的大小一般为1024bytes，我们进度条的实现最多输出不到150个字符而且后面还不能加‘\n’换行符。所以我们只能在每次printf后面调用fflush手动刷新IO缓冲区，以达到在sleep之前输出printf内容的目的。优化后，伪代码为：

```c
char bar[102];
int rate = 0;
bar[0] = '\0';
printf("%s")
while(rate <= 100)
{
    printf("[%s] \n",bar);
    fflush(stdout);
    sleep(3);
    bar[rate] = '=';
    rate++;
    bar[rate] = '\0';
}
```

结果展示：

实现代码:

```c


/*************************************************************************
    > File Name: progbar.c
    > Author:lzk 
    > Mail:939142928@qq,com 
    > Created Time: Sat 31 Dec 2016 10:45:12 PM CST
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
void progbar()
{
    char bar[102];
    char fun[5]="-//|\";
    char*tmp = fun;
    int rate = 0;
    while(rate <= 100) 
    {
        printf("[%-100s][%d]% [%c] \r",bar,rate,*tmp);
        tmp++;
        if(*tmp == fun[4])
        {
            tmp = fun;
        }
        fflush(stdout);
        usleep(50000);
        bar[rate] = '=';
        rate++;
        bar[rate]='\0';
    }
}

int main()
{
    progbar();
    return 0;   
}
```