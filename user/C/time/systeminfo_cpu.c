/*
 * @*************************************: 
 * @FilePath: /user/C/time/systeminfo_cpu.c
 * @version: 
 * @Author: dof
 * @Date: 2022-01-21 09:31:31
 * @LastEditors: dof
 * @LastEditTime: 2022-01-22 13:43:04
 * @Descripttion: 获取cpu mem disk 网络流量 信息
 * @**************************************: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define MAXBUFSIZE 1024
#define WAIT_SECOND 3       //暂停时间，单位为“秒”


typedef uint64_t   UINT64;
typedef uint32_t   UINT32;
typedef uint16_t   UINT16;
typedef uint8_t    UINT8;


#define MDMVS_MIPSEB         "mipseb"
#define MDMVS_MIPSEL         "mipsel"
#define MDMVS_ARM            "arm"
#define MDMVS_I386           "i386"

typedef  struct occupy
{
    char		 name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
} CPU_OCCUPY;
 
typedef struct PACKED
{
    char name[20];
    long total;
    char name2[20];
    long free;
}MEM_OCCUPY;
 
float g_cpu_used;
int cpu_num;                    //定义一个全局的int类型cup_num
void  cal_occupy(CPU_OCCUPY *, CPU_OCCUPY *);
void  get_occupy(CPU_OCCUPY *);
void  get_mem_occupy(MEM_OCCUPY *);
float get_io_occupy();
void  get_disk_occupy(char ** reused);
void  getCurrentDownloadRates(long int * save_rate);
int rutSys_getCpuClass(char *cpuClass);

int main()
{
    CPU_OCCUPY ocpu, ncpu;
    MEM_OCCUPY mem;

	char cpuClass[64] = {0};

	rutSys_getCpuClass(cpuClass);
	printf("cpuCLass: %s\n", cpuClass);

	//获取cpu核数
	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    printf("cpu mum:%d\n", cpu_num);
 
    //获取cpu使用率
    get_occupy(&ocpu);
    sleep(1);
    get_occupy(&ncpu);
    cal_occupy(&ocpu, &ncpu);
    printf("cpu used:%4.2f \n", g_cpu_used);
 
    //获取内存使用率
    get_mem_occupy(&mem);
 
    double using = ((double)(mem.total - mem.free) / mem.total) * 100;
    printf("mem used:%4.2f\n", using);
 
    //获取io使用率
    printf("io used:%4.2f\n", get_io_occupy());
 
    //获取当前磁盘的使用率
    char t[20] = "";
    char *used = t;
    get_disk_occupy(&used);
 
    //char used[20]=" " ;
    //get_disk_occupy((char **)&used);
    printf("disk used:%s\n", used);
 
    //网络延迟
    long int start_download_rates;      //保存开始时的流量计数
    long int end_download_rates;        //保存结果时的流量计数
    getCurrentDownloadRates(&start_download_rates);    //获取当前流量，并保存在start_download_rates里
    sleep(WAIT_SECOND);     //休眠多少秒，这个值根据宏定义中的WAIT_SECOND的值来确定
    getCurrentDownloadRates(&end_download_rates);    //获取当前流量，并保存在end_download_rates里
    printf("download is : %4.2f byte/s \n", ((float)(end_download_rates - start_download_rates)) / WAIT_SECOND );
}
 
void  cal_occupy (CPU_OCCUPY *o, CPU_OCCUPY *n)
{
    double od, nd;
    double id, sd;
    double scale;
 
    od = (double) (o->user + o->nice + o->system + o->idle);   //第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double) (n->user + n->nice + n->system + n->idle);   //第二次(用户+优先级+系统+空闲)的时间再赋给od
    scale = 100.0 / (float)(nd - od);         //100除强制转换(nd-od)之差为float类型再赋给scale这个变量
    id = (double) (n->user - o->user);        //用户第一次和第二次的时间之差再赋给id
    sd = (double) (n->system - o->system);    //系统第一次和第二次的时间之差再赋给sd
    g_cpu_used = ((sd + id) * 100.0) / (nd - od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
}
 
void  get_occupy (CPU_OCCUPY *o)
{
    FILE *fd;
    int n;
    char buff[MAXBUFSIZE];
 
    fd = fopen ("/proc/stat", "r");     //这里只读取stat文件的第一行及cpu总信息，如需获取每核cpu的使用情况，请分析stat文件的接下来几行。
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %u %u %u %u", o->name, &o->user, &o->nice, &o->system, &o->idle);
    fclose(fd);
}
 
void get_mem_occupy(MEM_OCCUPY * mem)
{
    FILE * fd;
    char buff[MAXBUFSIZE];
 
    fd = fopen("/proc/meminfo", "r");
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %ld", mem->name, &mem->total);
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %ld", mem->name2, &mem->free);
}
 
float get_io_occupy()
{
    char cmd[] = "iostat -d -x";
    char buffer[MAXBUFSIZE];
    char a[20];
    float arr[20];
    FILE* pipe = popen(cmd, "r");
 
    if (!pipe)
    {
        return -1;
    }
 
    fgets(buffer, sizeof(buffer), pipe);
    fgets(buffer, sizeof(buffer), pipe);
    fgets(buffer, sizeof(buffer), pipe);
    fgets(buffer, sizeof(buffer), pipe);
    sscanf(buffer, "%s %f %f %f %f %f %f %f %f %f %f %f %f %f ", a, &arr[0], &arr[1], &arr[2], &arr[3], &arr[4],
           &arr[5], &arr[6], &arr[7], &arr[8], &arr[9], &arr[10], &arr[11], &arr[12]);
 
    //printf("%f\n",arr[12]);
    
    pclose(pipe);
    return arr[12];
}
 
void get_disk_occupy(char ** reused)
{
    char currentDirectoryPath[ MAXBUFSIZE ];
 
    getcwd(currentDirectoryPath, MAXBUFSIZE);
 
    //printf("当前目录：%s\n",currentDirectoryPath);
    char cmd[50] = "df ";
    strcat(cmd, currentDirectoryPath);
 
    //printf("%s\n",cmd);
 
    char buffer[MAXBUFSIZE];
    FILE* pipe = popen(cmd, "r");
    char fileSys[20];
    char blocks[20];
    char used[20];
    char free[20];
    char percent[10];
    char moment[20];
 
    if (!pipe)
    {
        return;
    }
 
    if (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        sscanf(buffer, "%s %s %s %s %s %s", fileSys, blocks, used, free, percent, moment);
    }
 
    if (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        sscanf(buffer, "%s %s %s %s %s %s", fileSys, blocks, used, free, percent, moment);
    }
 
    //printf("desk used:%s\n",percent);
    strcpy(*reused, percent);
    return;
}
 
void getCurrentDownloadRates(long int * save_rate)
{
    char intface[] = "eth0:";  //这是网络接口名，根据主机配置
    //char intface[] = "wlan0:";
    FILE * net_dev_file;
    char buffer[1024];
    size_t bytes_read;
    char * match;
 
    if ((net_dev_file = fopen("/proc/net/dev", "r")) == NULL)
    {
        printf("open file /proc/net/dev/ error!\n");
        exit(EXIT_FAILURE);
    }
 
    int i = 0;
    while (i++ < 20)
    {
        if (fgets(buffer, sizeof(buffer), net_dev_file) != NULL)
        {
            if (strstr(buffer, intface) != NULL)
            {
                //printf("%d   %s\n",i,buffer);
                sscanf(buffer, "%s %ld", buffer, save_rate);
                break;
            }
        }
    }
 
    if (i == 20)
    {
        *save_rate = 0.01;
    }
 
    fclose(net_dev_file);     //关闭文件
    return;
}



/**
 * @brief 获取cpu 型号
 * 
 * @param cpuClass 
 * @return CmsRet 
 */
int rutSys_getCpuClass(char *cpuClass)
{
   int ret = 0;
   FILE *fs = NULL;
   char line[512];
   char *pChar = NULL;

   if ((fs=fopen("/proc/socinfo", "r")) != NULL)
   {
      while ( fgets(line, sizeof(line), fs) )
      {
         if (strncasecmp(line, "SoC Name", 8) == 0 &&
             (pChar = strstr(line, ":")) != NULL &&
             (pChar + 1) != NULL)
         {
            // pChar+2: read pass ": "
            strcpy(cpuClass, pChar+1);

            if ( strlen(cpuClass) > 0 && cpuClass[strlen(cpuClass)-1] == '\n' )
               cpuClass[strlen(cpuClass)-1] = '\0';
            
            printf("cpuClass:%s\n", cpuClass);
            ret=0;
            
            break;
         }
      
      }

      fclose(fs);
   }

   return ret;
}


int rutSys_getCpuInfo(UINT32 id, UINT32 *frequency, char   *architecture)
{
   int ret = 0;
   UINT32 num = 0, i = 0;
   char line[512];
   char *pChar = NULL;
   FILE *fs = NULL;

   if (architecture == NULL)
   {
      return 0;
   }

   *frequency = 0;

   fs = fopen("/proc/cpuinfo", "r");
   if (fs == NULL)
   {
      printf("Could not open /proc/cpuinfo\n");
      return ret;
   }

   while ( fgets(line, sizeof(line), fs) )
   {
      if (ret != 0)
      {
         // processor line mark the beginning of one processor
         if (strncasecmp(line, "processor", 9) == 0 &&
             (pChar = strstr(line, ":")) != NULL &&
             (pChar + 2) != NULL)
         {
            // pChar+2: read pass ": "
            num = (UINT32) strtoul(pChar+2, (char **)NULL, 10);
            // process id is started from 0
            if (num == id)
            {
               ret = 0;
            }
         }
      }
      else
      {
         // convert line to uppercase to avoid using strcasestr
         for (i = 0; line[i] != '\0'; i++)
            line[i] = toupper(line[i]);

         if (strstr(line, "BMIPS") != NULL)
            strncpy(architecture, MDMVS_MIPSEB, 32);
         else if (strstr(line, "LMIPS") != NULL)
            strncpy(architecture, MDMVS_MIPSEL, 32);
         else if (strstr(line, "ARM") != NULL)
            strncpy(architecture, MDMVS_ARM, 32);
         else if (strstr(line, "I386") != NULL)
            strncpy(architecture, MDMVS_I386, 32);
         else if ((strstr(line, "BOGOMIPS") != NULL) &&
                  ((pChar = strstr(line, ":")) != NULL))
         {
            // pChar+2: read pass ": "
            if ((pChar + 2) != NULL)
            {
               *frequency = (UINT32) strtoul(pChar+2, (char **)NULL, 10);
               break;
            }
         }
      }
   } /* while */
         
   fclose(fs);

   return ret;
}
