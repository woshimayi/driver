#include   <stdio.h>  
#include   <Winsock2.h>  
#include   <windows.h>  
  
typedef enum {false = 0, true = !false}bool;  
typedef struct _DNSHEAD{        //dns ͷ��  
    USHORT ID;  
    USHORT tag; // dns ��־(����)  
    USHORT numQ;        // ������  
    USHORT numA;        // ����  
    USHORT numA1;       // Ȩ������  
    USHORT numA2;       // ���Ӵ���  
}DnsHead;  
typedef struct _DNSQUERY    //dns ��ѯ�ṹ  
{  
    //    char   name[64];  
    //      //��ѯ������,����һ����С��0��63֮����ַ�����  
    USHORT type;  
    //��ѯ���ͣ���Լ��20����ͬ������  
    USHORT classes;  
    //��ѯ��,ͨ����A��Ȳ�ѯIP��ַ��  
      
}DnsQuery;  
  
#pragma comment(lib,"ws2_32.lib")  
  
// ��ʼ������  
bool initWSA();  
  
//��ʾ����  
void displayErrWSA(char *str);  
  
//�����׽���  
SOCKET CreateSocket(int type);  
  
//UDP sendto  
int MySendto(SOCKET sockTo, const char FAR * buf,int len,char *addr,USHORT port);  
  
//TCP ����  
bool MyConnect(SOCKET s, char *addr,USHORT );  
  
// UDP recvfrom  
int MyRecvFrom(SOCKET s, char FAR * buf,int len,char *addr,USHORT port);  
  
//����DNS ͷ��  
bool SetDNSHead(char *name,char *buf);  
  
int main(int arg,char *are[])  
{  
    int Result=0;  
    char buf[1024]={0};  
    char addr[16] = "192.168.1.1";// dns ��������ַ  
    char *name = 0; //Ҫ��ѯ������  
    SOCKET sockTo;  
    int len;  
    DnsHead *DnsH;  
    char *getIP;  
    //int i;  
      
    if ( !initWSA() )//��ʼ��  
    {  
        displayErrWSA("initWSA err!");  
        return 1;  
    }  
      
    //�����׽���  
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
            printf("\n������Ҫ��ѯ������:");  
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
          
        //����dns ͷ��  
        SetDNSHead(name,buf);  
          
        //���ͳ�ȥ���������ݳ���  
        len = sizeof(DnsHead)+sizeof(DnsQuery)+strlen(name)+2;  
          
        //      for (int  i =0;i<50;i+=2)  
        //      {  
        //          printf("x",(UCHAR)buf[i]);  
        //          printf("x ",(UCHAR)buf[i+1]);  
        //      }  
        //����DNS ����  
        if ( ( Result =MySendto(sockTo,buf,len,addr,53) ) <= 0)  
        {  
            displayErrWSA("sendto err!");  
            continue;  
        }  
          
        //����Ӧ��  
        if ( (Result =MyRecvFrom(sockTo,buf,1024,addr,53) ) <=  0)  
        {  
            displayErrWSA("recvfrom err!");  
            continue;  
        }  
          
        //�򵥵�ȡ�÷��ص� IP ��ַ( �յ������4�ֽ� )  
        DnsH = (DnsHead *)buf;  
        if (DnsH->numA == 0)  
        {  
            printf("�޷����� %s ��IP ��ַ��\n",name);  
            continue;  
        }  
        getIP =(char *)buf +Result - 4;  
        printf("%s ��IP��ַΪ: ",name);  
        for (Result= 0 ;Result<4 ;Result++)  
        {  
            printf("%u.",(UCHAR )getIP[Result]);  
        }  
        printf("\n");  
    }  
    return 0;  
}  
  
// ��ʼ������  
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
    //���÷������ݵ��� �׽��ּ���ַ�ṹ  
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
      
    //�������ӵ��� �׽��ּ���ַ�ṹ  
    SOCKADDR_IN   addrTo;  
    addrTo.sin_addr.S_un.S_addr=(inet_addr(addr));  
    addrTo.sin_family=AF_INET;  
    addrTo.sin_port=htons(port);  
      
    //����  
    Result = connect(sockTo,(struct sockaddr *)&addrTo,sizeof(SOCKADDR_IN));  
    if(SOCKET_ERROR == Result)  
    {  
        return false;  
    }  
    return true;  
}  
  
int MyRecvFrom(SOCKET s, char FAR * buf,int len,char *addr,USHORT port)  
{  
      
    //���÷������ݵ��� �׽��ּ���ַ�ṹ  
    int addrlen;  
    SOCKADDR_IN   addrFrom;  
    addrFrom.sin_addr.S_un.S_addr=inet_addr(addr);  
    addrFrom.sin_family=AF_INET;  
    addrFrom.sin_port=htons(port);  
    addrlen = sizeof(SOCKADDR_IN);  
    return recvfrom( s, buf, len, 0, (SOCKADDR *)&addrFrom, &addrlen);  
}  
  
int  ChName(char *fname,char *tname);//����ת��  
bool SetDNSHead(char *name,char *buf)  
{  
    DnsHead *DnsH;  
    DnsQuery *DnsQ;  
    int NameLen;  
    memset(buf,0,sizeof(DnsHead));  
      
    //����ͷ��  
    DnsH = (DnsHead *)buf;  
    DnsH->ID = (USHORT)1;  
    DnsH->tag = htons(0x0100);  
    DnsH->numQ = htons(1);  
    DnsH->numA = 0;  
      
    DnsQ =(DnsQuery *) ( buf+ sizeof(DnsHead) );  
    NameLen = ChName(name,(char *)DnsQ);  
      
    //���ò�ѯ��Ϣ  
    DnsQ = (DnsQuery *)( (char *)DnsQ + NameLen );  
    DnsQ->classes = htons(1);  
    DnsQ->type = htons(1);  
    return true;  
}  
  
//��ʾ������Ϣ  
void displayErrWSA(char *str)  
{  
    printf("\n%s,err = %d\n",str,WSAGetLastError());  
    getchar();  
}  
  
//����ת��  
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
