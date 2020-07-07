#include   <stdio.h>  
#include   <Winsock2.h>  
#include   <windows.h>  
  
typedef enum {false = 0, true = !false}bool;  
typedef struct _DNSHEAD{        //dns 头部  
    USHORT ID;  
    USHORT tag; // dns 标志(参数)  
    USHORT numQ;        // 问题数  
    USHORT numA;        // 答案数  
    USHORT numA1;       // 权威答案数  
    USHORT numA2;       // 附加答案数  
}DnsHead;  
typedef struct _DNSQUERY    //dns 查询结构  
{  
    //    char   name[64];  
    //      //查询的域名,这是一个大小在0到63之间的字符串；  
    USHORT type;  
    //查询类型，大约有20个不同的类型  
    USHORT classes;  
    //查询类,通常是A类既查询IP地址。  
      
}DnsQuery;  
  
#pragma comment(lib,"ws2_32.lib")  
  
// 初始化操作  
bool initWSA();  
  
//显示错误  
void displayErrWSA(char *str);  
  
//创建套接字  
SOCKET CreateSocket(int type);  
  
//UDP sendto  
int MySendto(SOCKET sockTo, const char FAR * buf,int len,char *addr,USHORT port);  
  
//TCP 连接  
bool MyConnect(SOCKET s, char *addr,USHORT );  
  
// UDP recvfrom  
int MyRecvFrom(SOCKET s, char FAR * buf,int len,char *addr,USHORT port);  
  
//设置DNS 头部  
bool SetDNSHead(char *name,char *buf);  
  
int main(int arg,char *are[])  
{  
    int Result=0;  
    char buf[1024]={0};  
    char addr[16] = "192.168.1.1";// dns 服务器地址  
    char *name = 0; //要查询的域名  
    SOCKET sockTo;  
    int len;  
    DnsHead *DnsH;  
    char *getIP;  
    //int i;  
      
    if ( !initWSA() )//初始化  
    {  
        displayErrWSA("initWSA err!");  
        return 1;  
    }  
      
    //创建套接字  
    if ( (sockTo = CreateSocket(SOCK_DGRAM)) == 0)  
    {  
        displayErrWSA("CreatSocket err!");  
        return 1;  
    }  
    while (1)  
    {  
        if (arg < 2)  
        {  
            char temp[1024]={0};  
            printf("\n请输入要查询的域名:");  
            scanf("%s",temp);  
            if (temp[0] == 'q' ||temp[0] == 'Q')  
            {  
                break;  
            }  
            name =  temp;  
        }  
        else  
        {  
            arg = 1;  
            name =  are[1];  
        }  
          
        //设置dns 头部  
        SetDNSHead(name,buf);  
          
        //发送出去的请求数据长度  
        len = sizeof(DnsHead)+sizeof(DnsQuery)+strlen(name)+2;  
          
        //      for (int  i =0;i<50;i+=2)  
        //      {  
        //          printf("x",(UCHAR)buf[i]);  
        //          printf("x ",(UCHAR)buf[i+1]);  
        //      }  
        //发送DNS 请求  
        if ( ( Result =MySendto(sockTo,buf,len,addr,53) ) <= 0)  
        {  
            displayErrWSA("sendto err!");  
            continue;  
        }  
          
        //接收应答  
        if ( (Result =MyRecvFrom(sockTo,buf,1024,addr,53) ) <=  0)  
        {  
            displayErrWSA("recvfrom err!");  
            continue;  
        }  
          
        //简单的取得返回的 IP 地址( 收到的最后4字节 )  
        DnsH = (DnsHead *)buf;  
        if (DnsH->numA == 0)  
        {  
            printf("无法解析 %s 的IP 地址。\n",name);  
            continue;  
        }  
        getIP =(char *)buf +Result - 4;  
        printf("%s 的IP地址为: ",name);  
        for (Result= 0 ;Result<4 ;Result++)  
        {  
            printf("%u.",(UCHAR )getIP[Result]);  
        }  
        printf("\n");  
    }  
    return 0;  
}  
  
// 初始化操作  
bool initWSA()  
{  
    WORD   wVersionRequested;  
    WSADATA   wsaData;  
    int Result;  
    wVersionRequested = MAKEWORD( 2, 2 );  
    Result = WSAStartup( wVersionRequested, &wsaData );  
    if(Result   !=   0 )  
    {  
        return false;  
    }  
      
    if( LOBYTE( wsaData.wVersion) != 2 ||  
        HIBYTE(wsaData.wVersion)!= 2 )  
    {  
        WSACleanup();  
        return false;  
    }  
    return true;  
}  
  
SOCKET CreateSocket(int type)  
{  
    SOCKET  sock=socket(AF_INET,type,0);  
    if (sock == INVALID_SOCKET )  
    {  
        return 0;  
    }  
    return sock;  
}  
  
int MySendto(SOCKET sockTo, const char FAR * buf,int len,char *addr,USHORT port)  
{  
    //设置发送数据到的 套接字及地址结构  
    SOCKADDR_IN   addrTo;  
    addrTo.sin_addr.S_un.S_addr=inet_addr(addr);  
    addrTo.sin_family=AF_INET;  
    addrTo.sin_port=htons(port);  
      
    return sendto(  sockTo, buf, len, 0,  
        (struct sockaddr *)&addrTo, sizeof(struct sockaddr)  );  
}  
  
bool MyConnect(SOCKET sockTo, char *addr,USHORT port)  
{  
      
    int   Result;  
      
    //设置连接到的 套接字及地址结构  
    SOCKADDR_IN   addrTo;  
    addrTo.sin_addr.S_un.S_addr=(inet_addr(addr));  
    addrTo.sin_family=AF_INET;  
    addrTo.sin_port=htons(port);  
      
    //连接  
    Result = connect(sockTo,(struct sockaddr *)&addrTo,sizeof(SOCKADDR_IN));  
    if(SOCKET_ERROR == Result)  
    {  
        return false;  
    }  
    return true;  
}  
  
int MyRecvFrom(SOCKET s, char FAR * buf,int len,char *addr,USHORT port)  
{  
      
    //设置发送数据到的 套接字及地址结构  
    int addrlen;  
    SOCKADDR_IN   addrFrom;  
    addrFrom.sin_addr.S_un.S_addr=inet_addr(addr);  
    addrFrom.sin_family=AF_INET;  
    addrFrom.sin_port=htons(port);  
    addrlen = sizeof(SOCKADDR_IN);  
    return recvfrom( s, buf, len, 0, (SOCKADDR *)&addrFrom, &addrlen);  
}  
  
int  ChName(char *fname,char *tname);//域名转化  
bool SetDNSHead(char *name,char *buf)  
{  
    DnsHead *DnsH;  
    DnsQuery *DnsQ;  
    int NameLen;  
    memset(buf,0,sizeof(DnsHead));  
      
    //设置头部  
    DnsH = (DnsHead *)buf;  
    DnsH->ID = (USHORT)1;  
    DnsH->tag = htons(0x0100);  
    DnsH->numQ = htons(1);  
    DnsH->numA = 0;  
      
    DnsQ =(DnsQuery *) ( buf+ sizeof(DnsHead) );  
    NameLen = ChName(name,(char *)DnsQ);  
      
    //设置查询信息  
    DnsQ = (DnsQuery *)( (char *)DnsQ + NameLen );  
    DnsQ->classes = htons(1);  
    DnsQ->type = htons(1);  
    return true;  
}  
  
//显示错误信息  
void displayErrWSA(char *str)  
{  
    printf("\n%s,err = %d\n",str,WSAGetLastError());  
    getchar();  
}  
  
//域名转化  
int  ChName(char *fname,char *tname)  
{  
      
    int j =0;  
    int i =strlen(fname)-1;  
    int k = i+1;  
    tname[i+2] = 0;  
    for (; i>=0;i--,k--)  
    {  
        if (fname[i] == '.')  
        {  
            tname[k] = j;  
            j=0;  
        }  
        else  
        {  
            tname[k] = fname[i];  
            j++;  
        }  
    }  
    tname[k] = j;  
    return strlen(tname)+1;  
}  
