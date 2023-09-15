/*
 * @*************************************: 
 * @FilePath: /user/C/string/array_test_copy.c
 * @version: 
 * @Author: dof
 * @Date: 2023-09-15 11:14:44
 * @LastEditors: dof
 * @LastEditTime: 2023-09-15 11:51:48
 * @Descripttion:  状态机 结构体示例
 * @**************************************: 
 */


typedef enum {
	BOB_TYPE_25L95,
	BOB_TYPE_UX3320,
	BOB_TYPE_I7525,
	BOB_TYPE_28L95,
	BOB_TYPE_28L97B,
	BOB_TYPE_HLC,
	BOB_TYPE_MINGYI,
	BOB_TYPE_MAX,

	HG_BOB_INVALID       = 0xff  //   "INVALID"
}BobType;

typedef enum
{
  PON_TYPE_EPON   = 0,
  PON_TYPE_GPON   = 1,
  PON_TYPE_NONE   = 2,
}PonType;


typedef int (*FuncAddChainRules)(void * handle);

int hg_ipt_flt_input(void * handle)
{
	printf("zzzzz %s\n", (char *)handle);
}

typedef struct array_test
{
	BobType chain;
	char *chain_name;
	char * table_name;
	FuncAddChainRules add_chain_rules;
}array_test;


array_test chipId[] = {
	{BOB_TYPE_25L95,    "EPON_25L95", "GPON_25L95_SPT", hg_ipt_flt_input},
	{BOB_TYPE_UX3320,   "EPON_3320", "GPON_3320_SPT", hg_ipt_flt_input},
	{BOB_TYPE_I7525,    "EPON_I7525", "GPON_I7525", hg_ipt_flt_input},
	{BOB_TYPE_28L95,    "EPON_28L95", "GPON_28L95", hg_ipt_flt_input},
	{BOB_TYPE_28L97B,   "EPON_28L97B", "GPON_28L97B", hg_ipt_flt_input},
	{BOB_TYPE_HLC,      "EPON_HLC", "GPON_HLC", hg_ipt_flt_input},
	{BOB_TYPE_MINGYI,   "EPON_MINGYI", "GPON_MINGYI", hg_ipt_flt_input},
};


int main(int argc, char const *argv[])
{
	for (int i = 0; i < sizeof(chipId) / sizeof(chipId[0]); i++)
	{
		if (BOB_TYPE_28L95 == chipId[i].chain)
		{
			printf("%s %10s\n", chipId[i].chain_name, chipId[i].table_name, chipId[i].add_chain_rules(chipId[i].chain_name));
		}
	}
	// printf("%s\n", chipId[BOB_TYPE_28L95].chain_name);
	return 0;
}
