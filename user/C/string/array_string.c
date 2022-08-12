/*
 * @*************************************: 
 * @FilePath: /user/C/string/struct_spec.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-25 14:04:14
 * @LastEditors: dof
 * @LastEditTime: 2022-07-25 14:06:22
 * @Descripttion: 字符串数组的应用
 * @**************************************: 
 */

#define NF_BR_PRE_ROUTING	0
/* If the packet is destined for this box. */
#define NF_BR_LOCAL_IN		1
/* If the packet is destined for another interface. */
#define NF_BR_FORWARD		2
/* Packets coming from a local process. */
#define NF_BR_LOCAL_OUT		3
/* Packets about to hit the wire. */
#define NF_BR_POST_ROUTING	4
/* Not really a hook, but used for the ebtables broute table */
#define NF_BR_BROUTING		5
#define NF_BR_NUMHOOKS		6


const char *ebt_hooknames[NF_BR_NUMHOOKS] =
{
	[NF_BR_PRE_ROUTING]"PREROUTING",
	[NF_BR_LOCAL_IN]"INPUT",
	[NF_BR_FORWARD]"FORWARD",
	// [NF_BR_LOCAL_OUT]"OUTPUT",
	[NF_BR_POST_ROUTING]"POSTROUTING",
	[NF_BR_BROUTING]"BROUTING"
};


int main(int argc, char const *argv[])
{
	printf("%s\n", ebt_hooknames[2]);
	return 0;
}
