<!--
 * @*************************************: 
 * @FilePath: /network/IPV4/dns/Linux网络应用开发---进程通信IPC汇总.md
 * @version: 
 * @Author: dof
 * @Date: 2023-11-03 13:16:43
 * @LastEditors: dof
 * @LastEditTime: 2023-11-03 13:16:44
 * @Descripttion: IPC 通信
 * @**************************************: 
-->

Linux网络应用开发---进程通信IPC汇总

图片

 1
一. 进程简述：
进程是操作系统的概念，每当我们执行一个程序时，对于操作系统来讲就创建了一个进程,在这个过程中，伴随着资源的分配和释放。可以认为进程是一个程序的一次执行过程。

1）Linux下进程结构

Linux下一个进程在内存里有三部分的数据，就是"代码段"、"堆栈段"和"数据段"。其实学过汇编语言的人一定知道，一般的CPU都有上述三种段寄存器，以方便操作系统的运行。

这三个部分也是构成一个完整的执行序列的必要的部分。所以不同的进程间，由于linux系统虚拟内存地址的管理，这三个段也是独立存在的，所以进程间是不能相互访问数据的，需要通过系统提供的特殊方法来进行相互通信。

2）Linux下进程通信的方法：

进程用户空间是相互独立的，一般而言是不能相互访问的。但很多情况下进程间需要互相通信，来完成系统的某项功能。进程通过与内核及其它进程之间的互相通信来协调它们的行为。

比较常用的IPC通信方法有：

管道（有名和无名）、信号、信号量、共享内存、消息队列和套接字socket通信。

图片



3）进程通信使用场景：

（1）数据传输：进程间数据传输；

（2）通知事件：一个进程向另一个或一组进程发送消息，通知某个事件的发生（如子进程终止时需通知父进程）；

（3）资源共享：多个进程共享资源，需要内核提供同步互斥机制；

（4）进程控制：某进程需要控制另一个进程的执行（如Debug进程），此时控制进程需要拦截另一个进程的所有陷入、异常、状态等。



二. 进程通信的方法：
1. 有名管道和无名管道

a. 无名管道（父子进程、兄弟进程间通信）:

---特点：

(1) 半双工。数据同一时刻只能单向传输；

(2) 数据从管道一端写入，另一端读出；

(3) 写入管道的数据遵循先进先出；

(4) 管道非普通文件，不属于某个文件系统，只存在于内存；

(5) 无名管道只能在具有公共祖先的进程（父子进程、兄弟进程等）之间使用。

---操作步骤：

(1)创建: pipe函数用来创建无名管道

(2)操作: read读；write写

(3)关闭操作端口: close

例子程序：

		#include <stdio.h>
		#include <unistd.h>
		#include <stdlib.h>
		#include <sys/wait.h>
		#include <sys/types.h>
		#include <sys/stat.h>
		int main(void)
		{
			char buf[32] = {0};
			pid_t pid;

			//  数量为 2 个：一个读端， 一个写端，
			int fd[2] = {-1};
			// 创建无名管道
			pipe(fd);
			printf("fd[0] is %d\n", fd[0]);
			printf("fd[2] is %d\n", fd[1]);
			
			// 创建进程
			pid = fork();
			if (pid < 0)
			{
			   printf("error\n");
			} 
			if (pid > 0)
			{
				int status;
				close(fd[0]);
				write(fd[1], "hello", 5);
				close(fd[1]);
				wait(&status);
				exit(0);
			} 
			if (pid == 0)
			{
			   close(fd[1]);
			   read(fd[0], buf, 32);
			   printf("buf is %s\n", buf);
			   close(fd[0]);
			   exit(0);
			}
			return 0;
		}	  
b. 有名管道(允许无亲缘关系进程间的通信)。

----特点：

(1) 它可以使互不相关的两个进程实现彼此通信

(2) 该管道可以通过路径名来指出，并且在文件系统中是可见的。在建立管道之后，两个进程就可以把它当作普通文件进行读写，使用非常方便。

(3) FIFO严格遵循先进先出原则，对管道及FIFO的读总是从开始处返回数据，对它们的写则把数据添加到末尾。有名管道不支持如Iseek()等文件的定位操作。

---操作步骤：

(1) 创建有名管道文件: mkfifo即是命令也是函数；mknod也可以创建管道文件;

(2) 打开有名管道: open;

(3) 读/写: read/write

(4) 关闭: close

例子程序：

		(1)named_pipe_write.c:
			#include <stdio.h>
			#include <unistd.h>
			#include <stdlib.h>
			#include <sys/wait.h>
			#include <sys/types.h>
			#include <sys/stat.h>
			#include <fcntl.h>
			int main(int argc, char *argv[])
			{
				int ret;
				char buf[32] = {0};
				int fd;
				if (argc < 2)
				{
				   printf("Usage:%s <fifo name> \n", argv[0]);
				   return -1;
				} 
				if (access(argv[1], F_OK) == -1)
				{	
					///创建有名管道文件
					ret = mkfifo(argv[1], 0666);
					if (ret == -1)
					{
						 printf("mkfifo is error \n");
						 return -2;
					} 
					printf("mkfifo is ok \n");
				 }
					///打开有名管道文件
				 fd = open(argv[1], O_WRONLY);
				 while (1)
				 {
					sleep(1);
					write(fd, "hello", 5);
				 } 
				 close(fd);
				 return 0;
			}
			 
		
		(2) named_pipe_read.c
			#include <stdio.h>
			#include <unistd.h>
			#include <stdlib.h>
			#include <sys/wait.h>
			#include <sys/types.h>
			#include <sys/stat.h>
			#include <fcntl.h>
			#include <string.h>
			int main(int argc, char *argv[])
			{
				char buf[32] = {0};
				int fd;
				if (argc < 2)
				{
					 printf("Usage:%s <fifo name> \n", argv[0]);
					 return -1;
				} 
				///打开有名管道，write已经创建的文件；
				fd = open(argv[1], O_RDONLY);
				while (1)
				{
					 sleep(1);
					 read(fd, buf, 32);
					 printf("buf is %s\n", buf);
					 memset(buf, 0, sizeof(buf));
				} 
				close(fd);
				return 0;
			}
2.信号量
1) 概念和原理：

信号量是一种计数器，用于控制对共享资源的访问。每次进程访问共享资源时，都要先获取一个信号量，如果信号量的值大于0，

则进程可以继续访问，否则进程需要等待。访问完成后，进程会释放信号量，使其值加1，以便其他进程访问；

2） 相关函数：

linux系统提供如下函数来对信号量值进行操作的，包括的头文件为sys/sem.h。

--semget函数：创建一个新信号量或者获取一个已有的信号量的键

--semop函数： 对信号量进行改变，做p或者v操作

--semctl函数：用来直接控制信号量信息

--删除信号量：ipcrm -s id

3）例子程序：

---增加信号量的值例子(sempore_add.c )：		//=============================sempore_add.c==========================
			#include <sys/types.h>
			#include <sys/ipc.h>
			#include <sys/sem.h>
			#include <stdio.h>

			#define KEY 1234

			union semun {
				int val;
				struct semid_ds *buf;
				ushort *array;
			};

			int main()
			{
				int semid = semget(KEY, 1, IPC_CREAT | 0666);
				if (semid < 0) {
					perror("semget error");
					return 1;
				}

				union semun arg;
				arg.val = 1;
				if (semctl(semid, 0, SETVAL, arg) < 0) {
					perror("semctl error");
					return 1;
				}

				struct sembuf buf;
				buf.sem_num = 0;
				buf.sem_op = 1;
				buf.sem_flg = SEM_UNDO;

				if (semop(semid, &buf, 1) < 0) {
					perror("semop error");
					return 1;
				}

				printf("Semaphore value: %d\n", semctl(semid, 0, GETVAL, arg));
				return 0;
			}
	
---减少信号量的值例子（sempore_sub.c）：

			//=============================sempore_sub.c==========================
			
			#include <sys/types.h>
			#include <sys/ipc.h>
			#include <sys/sem.h>
			#include <stdio.h>

			#define KEY 1234

			union semun {
				int val;
				struct semid_ds *buf;
				ushort *array;
			};

			int main()
			{
				int semid = semget(KEY, 1, IPC_CREAT | 0666);
				if (semid < 0) {
					perror("semget error");
					return 1;
				}

				union semun arg;
				arg.val = 1;
				if (semctl(semid, 0, SETVAL, arg) < 0) {
					perror("semctl error");
					return 1;
				}

				struct sembuf buf;
				buf.sem_num = 0;
				buf.sem_op = -1;
				buf.sem_flg = SEM_UNDO;

				if (semop(semid, &buf, 1) < 0) {
					perror("semop error");
					return 1;
				}

				printf("Semaphore value: %d\n", semctl(semid, 0, GETVAL, arg));
				
				
				////销毁信号量  
				if (semctl(semid, 0, IPC_RMID, 0) < 0) {
					perror("semctl error");
					return 1;
				}

				printf("Semaphore destroyed\n");
				return 0;
			} 


3.信号
（1）信号概念和原理：

信号是一种异步通信方式，当进程接收到一个信号时，会打断当前的执行，转而执行与该信号相关联的信号处理函数。 比较类似软中断。但信号和中断有所不同，中断的响应和处理都发生在内核空间，而信号的响应发生在内核空间， 信号处理程序的执行却发生在用户空间。

Linux提供了许多信号，如SIGINT、SIGTERM、SIGKILL等，可以在linux系统 Shell 中查看所有信号和对应的编号：kill -l

（2）主要函数：

a. void (*signal(int sig,void (*func)(int)))(int);

说明：绑定收到某个信号后 的回调函数.第一个参数为信号，第二个参数为对此信号挂接用户自己的处理函数指针。 返回值为以前信号处理程序的指针。例子：int ret = signal(SIGSTOP, sig_handle);

b. int sigaction(int signum, const struct sigaction *act,struct sigaction *oldact);

说明：由于 signal 不够健壮，推荐使用 sigaction 函数，sigaction 函数重新实现了 signal 函数

c. int kill(pid_t pid,int sig);

说明：kill函数向进程号为pid的进程发送信号，信号值为sig。当pid为0时，向当前系统的所有进程发送信号sig。

kill的pid参数有四种情况：

---pid>0, 则发送信号sig给进程号为pid的进程

---pid=0，则发送信号sig给当前进程所属组中的所有进程

---pid=-1,则发送信号sig给除1号进程与当前进程外的所有进程

---pid<-1,则发送信号sig给属于进程组pid的所有进程

例子：结束父进程 kill(getppid(), SIGKILL);

d. int raise(int sig);

说明: 向当前进程中自举一个信号sig, 即向当前进程发送信号。相当于 kill(getpid(),sig);

e. unsigned int alarm(unsigned int seconds);

说明: 用来设置信号SIGALRM在经过参数seconds指定的秒数后传送给目前的进程. 如果参数seconds为0,则之前设置的闹钟会被取消,并将剩下的时间返回

f. int sigqueue(pid_t pid, int sig, const union sigval value);

说明：用于向指定的进程发送特定的信号，并且可以传递一个额外的数据值。它提供了比 kill 函数更丰富的功能，可以用于进程间的高级通信。



(3)例子程序：

			-----------------signal_receiver.c ------------------------------
			#include <stdio.h>
			#include <string.h>
			#include <signal.h>
			#include <unistd.h>
			#include <sys/types.h>
			#include <unistd.h>

			void signal_Handle(int sig, siginfo_t* info, void* ucontext)
			{
				printf("handler : sig = %d\n", sig);
				printf("handler : info->si_signo = %d\n", info->si_signo);
				printf("handler : info->si_code = %d\n", info->si_code);
				printf("handler : info->si_pid = %d\n", info->si_pid);
				printf("handler : info->si_value = %d\n", info->si_value.sival_int);
			}

			int main(int argc, char** argv)
			{
				printf("pid :%d\n", getpid());

				struct sigaction act = {0};

				act.sa_sigaction = signal_Handle;
				act.sa_flags = SA_RESTART | SA_SIGINFO;

				/* 添加信号屏蔽字 */
				/* 下面信号在信号处理程序执行时会被暂时阻塞 */
				sigaddset(&act.sa_mask, 40);
				sigaddset(&act.sa_mask, SIGINT);

				/* 设置信号的处理行为，设置后40和SIGINT信号将由act里面的信号处理函数处理 */
				sigaction(40, &act, NULL);
				sigaction(SIGINT, &act, NULL);

				while(1)
				{
					sleep(1);
				}

				return 0;
			}
			-----------------signal_sender.c ------------------------------
			#include <stdio.h>
			#include <stdlib.h>
			#include <sys/types.h>
			#include <unistd.h>
			#include <signal.h>


			int main(int argc, char** argv)
			{
				pid_t pid = atoi(argv[1]);

				union sigval sv = {123456};
				
				//向指定pid发送信号；
				sigqueue(pid, 40, sv);

				raise(SIGINT); 

				return 0;
			}
4.消息队列
(1)概念和原理：

消息队列是一种进程间通信的方式，允许一个进程向另一个进程发送消息。它是一种异步通信方式，发送方发送消息后即可继续执行，不必等待接收方的响应。

原理如下图所示：

图片



（2）特点：

---消息队列是消息的链表，存放于内存中，内核维护消息队列；

---消息队列中的消息是有类型和格式的；

---消息队列可实现消息随机查询，不一定要遵循先进先出的顺序，而是每个进程可以按照自定义的类型进行读取；

---与管道相同，读出数据后，消息队列对应数据会被删除；

---每个管道都有消息队列标识符，在整个系统中是唯一的；

---消息队列允许一个或者多个进程向它写入或者读取数据；

---内核重启或者人为删除才会删除消息队列，否则会一直存在与系统中

(3) 相关函数：

a. key_t ftok(const char *pathname, int proj_id);

说明：获取系统唯一Key值（IPC键值），系统中可能会存在许多的消息队列，通过Key这个系统唯一值，可以选择想要进入的消息队列；

b. int msgget(key_t key, int msgflg);

说明:创建或者打开一个新的消息队列。即使进程不同，但是如果key值是相同的，那么也可以进入相同的消息队列，返回相同的消息队列标识符,

c. 查看消息队列的一些Linux命令:

ipcs -q : 查看当前进程间通信之消息队列

ipcrm -q 队列号: 删除指定的消息队列；

d. int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

说明: 将新消息添加到消息队列;

e. ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);

说明： 从指定的消息队列标识符中接受信息，同时一旦接受成功，从消息队列中删除该信息。

f. int msgctl(int msqid, int cmd, struct msqid_ds *buf);

说明：对消息队列进行修改，修改属性或者删除消息队列等

图片



(3）例子程序：

			-----------------message_sender.c ------------------------------
			#include<stdio.h>
			#include<sys/msg.h>
			#include<stdlib.h>
			#include<unistd.h>
			#include<string.h>
			//定义消息
			struct mess
			{
				long type;
				char data[128];
			};
			int main()
			{
				key_t  key;
				
				///创建生成唯一的key; 
				if ((key = ftok("/home/tmp", 'a')) == -1) {
					perror("ftok");
					exit(1);
				} 
				
				
				int msgid=msgget((key_t)key,IPC_CREAT|0600); 
				if(msgid==-1)
				{
					exit(0);
				}
				struct mess dt;
				dt.type=1;
				strcpy(dt.data,"hello1");
				
				//1号消息内容hello1
				msgsnd(msgid,(void*)&dt,128,0);//标志位0				 
			}
 
		
			-----------------message_reader.c ------------------------------
			#include<stdio.h>
			#include<sys/msg.h>
			#include<stdlib.h>
			#include<unistd.h>
			 
			struct mess
			{
				long type;
				char data[128];
			};
			int main()
			{
				int msgid=msgget((key_t)1235,IPC_CREAT|0600);
				if(msgid==-1)
				{
					exit(0);
				}
				struct mess dt;
				msgrcv(msgid,(void*)&dt,128,1,0);
				printf("%s",dt.data);
				
				///删除队列
				if (msgctl(msqid, IPC_RMID, NULL) == -1) {
					perror("msgctl");
					exit(1);
				}
			}	
5.共享内存
(1)概念：

共享内存是一种高效的IPC机制，是最快的进程间通信方式，很多追求效率的程序之间进行通信的时候都会选择它；它允许多个进程共享同一个物理内存区域，从而避免了数据拷贝和进程切换的开销。

原理如下图所示：

图片



（2）共享内存的建立与释放：

---共享内存的建立大致包括以下两个过程：

a.在物理内存当中申请共享内存空间;

b.将申请到的共享内存挂接到地址空间，即建立映射关系;

---共享内存的释放大致包括以下两个过程：

a. 将共享内存与地址空间去关联，即取消映射关系。

b. 释放共享内存空间，即将物理内存归还给系统。

(3)相关函数：

a.key_t ftok(const char *pathname, int proj_id);

说明：这个返回的key值可以传给共享内存参数，作为struct ipc_perm中唯一标识共享内存的key;

b. int shmget(key_t key, size_t size, int shmflg);

说明：共享内存的创建；

c. void *shmat(int shmid, const void *shmaddr, int shmflg)

说明：将共享内存连接到进程地址空间，shmat函数的第三个参数shmflg有以下三个选项：

SHM_RDONLY: 关联共享内存后只进行读取操作

SHM_RND:若shmaddr不为NULL，则关联地址自动向下调整为SHMLBA的整数倍。

0: 默认为读写权限

d. int shmctl(int shmid, int cmd, struct shmid_ds *buf);

说明：控制共享内存，cmd如下：

IPC_STAT: 获取共享内存的当前关联值，此时参数buf作为输出型参数

IPC_SET: 在进程有足够权限的前提下，将共享内存的当前关联值设置为buf所指的数据结构中的值

IPC_RMID: 删除共享内存段;

e. int shmdt(const void *shmaddr)

说明：取消共享内存与进程地址空间之间的关联

（3）内存共存相关的系统命令：

---查看共享内存命令：ipcs -m

---删除共享内存命令：ipcrm -m

(4）例子程序：共享内存数据读；

	  //shm_server.c
	  -----------------shm_server.c ------------------------------
		#include <stdio.h>
		#include <stdio.h>
		#include <sys/types.h>
		#include <sys/ipc.h>
		#include <sys/shm.h>
		#include <unistd.h>
		 
		#define PATHNAME "/home/IPC/shm/server.c" //路径名 
		#define PROJ_ID 0x6666 //整数标识符
		 
		int main()
		{
			key_t key = ftok(PATHNAME, PROJ_ID); //获取key值
			if (key < 0){
				perror("ftok");
				return 1;
			}
		 
			int shm = shmget(key, SIZE, IPC_CREAT | IPC_EXCL | 0666); //创建新的共享内存
			if (shm < 0){
				perror("shmget");
				return 2;
			}
			
			printf("key: %x\n", key); //打印key值
			printf("shm: %d\n", shm); //打印共享内存用户层id
		 
			char* mem = shmat(shm, NULL, 0); //关联共享内存
		 
			while (1)
			{ 
				//服务端不断读取共享内存当中的数据并输出
				while (1)
				{
					printf("client# %s\n", mem);
					sleep(1);
				}
		 
			}
		 
			shmdt(mem); //共享内存去关联
		 
			shmctl(shm, IPC_RMID, NULL); //释放共享内存
			return 0;
		}


客户端: 共享内存数据写；

	-----------------shm_client.c ------------------------------
		
		#include <stdio.h>
		#include <stdio.h>
		#include <sys/types.h>
		#include <sys/ipc.h>
		#include <sys/shm.h>
		#include <unistd.h>
		 
		#define PATHNAME "/home/IPC/shm/server.c" //路径名 
		#define PROJ_ID 0x6666 //整数标识符
		 
		int main()
		{
			key_t key = ftok(PATHNAME, PROJ_ID); //获取与server进程相同的key值
			if (key < 0){
				perror("ftok");
				return 1;
			}
			int shm = shmget(key, SIZE, IPC_CREAT); //获取server进程创建的共享内存的用户层id
			if (shm < 0){
				perror("shmget");
				return 2;
			}
		 
			printf("key: %x\n", key); //打印key值
			printf("shm: %d\n", shm); //打印共享内存用户层id
		 
			char* mem = shmat(shm, NULL, 0); //关联共享内存
		 
			int i = 0;
			while (1)
			{
				//客户端不断向共享内存写入数据
				int i = 0;
				while (1)
				{
					mem[i] = 'A' + i;
					i++;
					mem[i] = '\0';
					sleep(1);
				}		 
			}
		 
			shmdt(mem); //共享内存去关联
			return 0;
		}
6.套接字
(1)概念和原理：

套接字是一种用于网络通信的编程接口， 也是一种特殊的IPC通信机制，一般分为两种角色：客户端和服务器 ，既可以在本机不同进程间通信，也可以在跨网络不同的多台主机间通信，可以一对多。

流程如下图：

图片



(2)相关函数：

a. int tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

说明：创建Socket函数创建一个Socket对象，并指定通信协议和类型（流式或数据报式）。 IPPROTO_TCP表示TCP协议

b. int bind(int sock, struct sockaddr *addr, socklen_t addrlen); //Linux

说明：绑定地址，使用 bind 函数将 Socket 绑定到一个特定的IP地址和端口号上。

c. listen;

说明：设置监听，等待接收client端连接请求；

d. int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);

说明：接受连接，对于流式 Socket，使用 accept 函数接受客户端的连接请求，并返回一个新的Socket对象用于与客户端进行通信。 对于数据报式Socket，可以省略此步骤。

e. int connect(int sock, struct sockaddr *serv_addr, socklen_t addrlen);

说明：连接指定IP和端口的服务器

f. ssize_t send(int sockfd, const void *buf, size_t len, int flags);

或ssize_t write(int fd, const void *buf, size_t nbytes);

说明：数据发送；

g. ssize_t recv(int sockfd, void *buf, size_t len, int flags);

或 ssize_t read(int fd, void *buf, size_t nbytes);

说明：数据接收；

h. int close(int fd) ;

说明：关闭连接，使用 close 函数关闭Socket连接。

(3）例子程序：

本地socket 通信 服务端程序：

		----------------local_socket_server.c ------------------------------
			#include <stdlib.h>
			#include <stdio.h>
			#include <stddef.h>
			#include <sys/socket.h>
			#include <sys/un.h>
			#include <sys/types.h>
			#include <sys/stat.h>
			#include <unistd.h>
			#include <errno.h>

			#define QLEN 10
			#define IPC_SOCKET_PATH "ipctest.socket"

			int serv_listen(const char *name)
			{
				int fd, len, err, rval;
				struct sockaddr_un un;

				/* create a UNIX domain stream socket */
				if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
					return(-1);
				/* in case it already exists */
				unlink(name); 			

				/* fill in socket address structure */
				memset(&un, 0, sizeof(un));
				un.sun_family = AF_UNIX;
				strcpy(un.sun_path, name);
				len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

				/* bind the name to the descriptor */
				if (bind(fd, (struct sockaddr *)&un, len) < 0) {
					rval = -2;
					goto errout;
				}
				if (listen(fd, QLEN) < 0) { /* tell kernel we're a server */
					rval = -3;
					goto errout;
				}
				return(fd);

			errout:
				err = errno;
				close(fd);
				errno = err;
				return(rval);
			}
			
			int serv_accept(int listenfd, uid_t *uidptr)
			{
				int clifd, len, err, rval;
				time_t staletime;
				struct sockaddr_un un;
				struct stat statbuf;

				len = sizeof(un);
				if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
					return(-1); /* often errno=EINTR, if signal caught */

				/* obtain the client's uid from its calling address */
				len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
				un.sun_path[len] = 0; /* null terminate */

				if (stat(un.sun_path, &statbuf) < 0) {
					rval = -2;
					goto errout;
				}
				if (S_ISSOCK(statbuf.st_mode) == 0) {
					rval = -3; /* not a socket */
					goto errout;
				}
				if (uidptr != NULL)
					*uidptr = statbuf.st_uid; /* return uid of caller */
				/* we're done with pathname now */
				unlink(un.sun_path); 
				return(clifd);

			errout:
				err = errno;
				close(clifd);
				errno = err;
				return(rval);
			}
			
			///////////////////////////main ////////////////////////////////
			int main(void)
			{
				int lfd, cfd, n, i;
				uid_t cuid;
				char buf[1024];
				lfd = serv_listen(IPC_SOCKET_PATH);

				if (lfd < 0) {
					switch (lfd) {
						case -3:perror("listen"); break;
						case -2:perror("bind"); break;
						case -1:perror("socket"); break;
					}
					exit(-1);
				}
				cfd = serv_accept(lfd, &cuid);
				if (cfd < 0) {
					switch (cfd) {
						case -3:perror("not a socket"); break;
						case -2:perror("a bad filename"); break;
						case -1:perror("accept"); break;
					}
					exit(-1);
				}
				while (1) 
				{
					n = read(cfd, buf, 1024);
					
					if (n == -1) {
						if (errno == EINTR)
							break;
						}
					else if (n == 0) {
						printf("the other side has been closed.\n");
						break;
					}
					
					////send back data to client
					for (i = 0; i < n; i++)
					{	buf[i] = toupper(buf[i]);
						write(cfd, buf, n);
					}
				}
				
				close(cfd);
				close(lfd);
				return 0;
			}
 


本地socket 通信 客户端程序：

----------------local_socket_client.c ------------------------------

			#include <stdio.h>
			#include <stdlib.h>
			#include <stddef.h>
			#include <sys/stat.h>
			#include <fcntl.h>
			#include <unistd.h>
			#include <sys/socket.h>
			#include <sys/un.h>
			#include <errno.h>

			#define CLI_PATH "/var/tmp/" /* +5 for pid = 14 chars */
			#define IPC_SOCKET_PATH "ipctest.socket"
			
			/*
			* Create a client endpoint and connect to a server.
			* Returns fd if all OK, <0 on error.
			*/
			int cli_conn(const char *name)
			{
				int fd, len, err, rval;
				struct sockaddr_un un;

				/* create a UNIX domain stream socket */
				if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
					return(-1);

				/* fill socket address structure with our address */
				memset(&un, 0, sizeof(un));
				un.sun_family = AF_UNIX;
				sprintf(un.sun_path, "%s%05d", CLI_PATH, getpid());
				len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

				/* in case it already exists */
				unlink(un.sun_path); 
				if (bind(fd, (struct sockaddr *)&un, len) < 0) {
					rval = -2;
					goto errout;
				}

				/* fill socket address structure with server's address */
				memset(&un, 0, sizeof(un));
				un.sun_family = AF_UNIX;
				strcpy(un.sun_path, name);
				len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
				if (connect(fd, (struct sockaddr *)&un, len) < 0) {
					rval = -4;
					goto errout;
				}
			return(fd);
				errout:
				err = errno;
				close(fd);
				errno = err;
				return(rval);
			}
			
			///////////////////////////main ////////////////////////////////
			int main(void)
			{
				int fd, n;
				char buf[1024];

				fd = cli_conn(IPC_SOCKET_PATH);
				if (fd < 0) {
					switch (fd) {
						case -4:perror("connect"); break;
						case -3:perror("listen"); break;
						case -2:perror("bind"); break;
						case -1:perror("socket"); break;
					}
					exit(-1);
				}
				while (fgets(buf, sizeof(buf), stdin) != NULL) {
					write(fd, buf, strlen(buf));
					n = read(fd, buf, sizeof(buf));
					write(STDOUT_FILENO, buf, n);
				}
				close(fd);
				return 0;
			}
不同主机端的套接字IPC通信属于网络通信话题，这里就不再详细论述了。