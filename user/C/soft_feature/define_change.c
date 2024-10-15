/*
 * @*************************************: 
 * @FilePath: /user/C/soft_feature/define_change.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2024-03-21 11:50:47
 * @Descripttion: 宏定义 常规用例
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int fun(void)
{
	return 0 == access("new_qos", F_OK);
}

#define HG_CM_NEW_QOS fun()

// int fun(void)
// {
// 	return 0;
// }

// int fun1(char *conse)
// {
// 	printf("2222222222222  %s\n", conse);
// 	return 0;
// }


// #define fun() fun1(__FUNCTION__)


char *ip4defaultClassifications[] =
{
	"-p udp --dport 53",    /* DNS probe */
	"-p 2",                 /* IGMP */
	"-p udp --sport 161",   /* SNMP */
	"-p udp --sport 520",   /* RIP */
	"-p udp --dport 67:68", /* DHCP relay */
	"-p tcp --sport 30005"  /* TR69 */
	/* "-p tcp --sport 80",      httpd */
	/* "-p tcp --sport 443"      https */
};

char *ip6defaultClassifications[] =
{
	"-p tcp -m tcp --sport 30005",
	"-p udp -m udp --sport 22456:32456",
	"-p icmpv6 -m icmp6 --icmpv6-type 129",
	"-p udp -m udp --dport 53",
	"-p udp -m udp --sport 521",
	"-p udp -m udp --dport 547",
	"-p icmpv6 -m icmp6 --icmpv6-type 135",
	"-p icmpv6 -m icmp6 --icmpv6-type 136"
};

#define QOS_QUEUE_NBR 8
#define DEFINECLASSIFICATIONS(x) ip##x##defaultClassifications

#define joiner(param1, param2) param1##param2
#define tostring(param) #param
#define nested(param) tostring(param)


int hi_ipc_call(char *pc_call_name)
{
	printf("pc_call_name = %s\n", pc_call_name);
	return 0;
}


#define HI_IPC_CALL(ipc_fun)                       \
    hi_ipc_call("hi_" ipc_fun "_call")




// void defaultClassifications(int ipmode)
// {
// 	int i = 0;
// 	int prio = 1;
// 	int queueMark = 0;
// 	char *defaultclass[] = DEFINECLASSIFICATIONS(4);

// 	/*qObj->classQueue: [1,8]; priority range: [7,0]*/
// 	// prio = QOS_QUEUE_NBR - 1;
// 	// queueMark = SKBMARK_SET_Q(queueMark, prio);
// 	// printf("sssss\n");
	
// 	for (i = 0; i < sizeof(DEFINECLASSIFICATIONS(4))/sizeof(char *); i++)
// 	{
// 		// rut_hg_add_chain_rule(handle, "-t mangle -D OUTPUT -j MARK --or-mark 0x%x %s",
// 		// 		XTM_QOS_LEVELS-1, ip6_defaultClassifications[i]);
// 		printf("[%d:%s]\n", i, DEFINECLASSIFICATIONS(4)[i]);
// 	}
// }

#define unlink(s)                                      \
	do                                                 \
	{                                                  \
		printf("[%s:%d] rm %s\n", __func__, __LINE__, s); \
		unlink(s);                                       \
	} while (0)

int main()
{
	// fun();
// if(HG_CM_NEW_QOS)
// {
// 	printf("sssssssss %d\n", HG_CM_NEW_QOS);
// }
// else
// {
// 	printf("aaaaaaaaa %d\n", HG_CM_NEW_QOS);
// }

	// defaultClassifications("ip4");
	// printf("ipmode = %s\n", DEFINECLASSIFICATIONS("ip4", "defaultClassifications"));

    /*数字直接拼接结果为int*/
    printf("1. joiner result is int: %d \n", joiner(1, 2));
    /*没有使用#预编译字符串转换符的时候，嵌套宏按照从里到外执行*/
    printf("2. nested joiner result is string: %s \n", nested(joiner(1, 2)));
	printf("2. nested joiner result is string: %s \n", nested(sss));
    /*#预编译字符串转换符直接处理参数，即使参数也为宏，原样转换，而不是先执行子嵌套*/
    printf("3. tostring joiner result is string: %s \n", tostring(joiner(1, 2)));
	printf("3. tostring joiner result is string: %s \n", nested(joiner(ttt, yyy)));
	printf("3. tostring joiner result is string: %s \n", tostring(ffffffff));


	printf("[%s]\n", DEFINECLASSIFICATIONS(6)[0]);

	// defaultClassifications(4);

	// HI_IPC_CALL("zzzzzzz");

	unlink("123");

	return 0;
}


