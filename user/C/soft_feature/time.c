/*
 * @*************************************:
 * @FilePath     : /user/C/soft_feature/time.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-07-19 14:54:51
 * @LastEditors  : dof
 * @LastEditTime : 2025-12-26 17:42:24
 * @Descripttion : time format transform
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void getTheCurrentTime(char *time_now)
{
	time_t now;
	struct tm *timenow;

	time(&now);
	timenow = localtime(&now);
	snprintf(time_now, 32, "%d%02d%02d%02d%02d%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday,
			 timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
}

int convert_cst_to_iso(const char *input, char *output, size_t output_size)
{
	if (input == NULL || output == NULL || output_size < 20)
	{
		return -1; // 参数检查
	}

	struct tm tm = {0};
	const char *format = "%b %d %H:%M:%S CST %Y";

	// 解析输入字符串
	if (strptime(input, format, &tm) == NULL)
	{
		// 尝试其他可能的格式变体
		if (strptime(input, "%b %d %H:%M:%S UTC %Y", &tm) == NULL)
		{
			return -1; // 解析失败
		}
	}

	// 格式化为目标字符串
	if (strftime(output, output_size, "%Y-%m-%d %H:%M.%S", &tm) == 0)
	{
		return -1; // 格式化失败
	}

	return 0; // 成功
}

char *convert_cst_to_iso_alloc(const char *input)
{
	if (input == NULL)
	{
		return NULL;
	}

	// 分配足够的内存（YYYY-MM-DD HH:MM.SS + null终止符 = 20字节）
	char *result = (char *)malloc(20 * sizeof(char));
	if (result == NULL)
	{
		return NULL; // 内存分配失败
	}

	if (convert_cst_to_iso(input, result, 20) != 0)
	{
		free(result);
		return NULL; // 转换失败
	}

	return result;
}


#define HI_IPC_SCT_INIT          ".hi_ipc_init"
#define HI_IPC_SCT_EXIT          ".hi_ipc_exit"
#define HI_IPC4(ipc_fun, type, data, size)                              \
    hi_int32 hi_##ipc_fun##_ipc( type, hi_uint32);                      \
    hi_int32 hi_##ipc_fun##_call(hi_void *pv_data, hi_uint32 ui_size)   \
    {                                                                   \
        if (pv_data == NULL || ui_size == 0)                         \
            return HI_IPC_ERR_PARAMETER_E;                              \
        return hi_##ipc_fun##_ipc(pv_data, ui_size);                    \
    }                                                                   \
    hi_int32 ipc_fun(type pst_data, hi_uint32 ui_size)                  \
    {                                                                   \
        return hi_ipc_call("hi_"#ipc_fun"_call", pst_data, ui_size );   \
    }                                                                   \
    hi_int32 __attribute__((section(HI_IPC_SCT_INIT)))                  \
    hi_##ipc_fun##_init(hi_void)                                        \
    {                                                                   \
        hi_ipc_call_node_s st_node = {"hi_"#ipc_fun"_call",             \
                                      hi_##ipc_fun##_call};             \
        return hi_ipc_reg(&st_node);                                    \
    }                                                                   \
    hi_int32 __attribute__((section(HI_IPC_SCT_EXIT)))                  \
    hi_##ipc_fun##_exit(hi_void)                                        \
    {                                                                   \
        return hi_ipc_unreg("hi_"#ipc_fun"_call");                      \
    }                                                                   \
    hi_int32 hi_##ipc_fun##_ipc(type data, hi_uint32 size)

#define HI_IPC3(ipc_fun, type, data)                                    \
    hi_int32 hi_##ipc_fun##_ipc(type);                                  \
    hi_int32 hi_##ipc_fun##_call(hi_void *pv_data, hi_uint32 ui_size)   \
    {                                                                   \
        type temp;                                                      \
        if (pv_data == NULL || ui_size != sizeof(*temp))             \
            return HI_IPC_ERR_PARAMETER_E;                              \
        return hi_##ipc_fun##_ipc(pv_data);                             \
    }                                                                   \
    hi_int32 ipc_fun(type pst_data)                                     \
    {                                                                   \
        return hi_ipc_call("hi_"#ipc_fun"_call", pst_data, sizeof(*pst_data)); \
    }                                                                   \
    hi_int32 __attribute__((section(HI_IPC_SCT_INIT)))                  \
    hi_##ipc_fun##_init()                                               \
    {                                                                   \
        hi_ipc_call_node_s st_node = {"hi_"#ipc_fun"_call",             \
                                      hi_##ipc_fun##_call};             \
        return hi_ipc_reg(&st_node);                                    \
    }                                                                   \
    hi_int32 __attribute__((section(HI_IPC_SCT_EXIT)))                  \
    hi_##ipc_fun##_exit()                                               \
    {                                                                   \
        return hi_ipc_unreg("hi_"#ipc_fun"_call");                      \
    }                                                                   \
    hi_int32 hi_##ipc_fun##_ipc(type data)

#define HI_IPC1(ipc_fun)                                                \
    hi_int32 hi_##ipc_fun##_ipc(hi_void);                               \
    hi_int32 hi_##ipc_fun##_call(hi_void *pv_data, hi_uint32 ui_size)   \
    {                                                                   \
        if (pv_data != NULL || ui_size != 0)                         \
            return HI_IPC_ERR_PARAMETER_E;                              \
        return hi_##ipc_fun##_ipc();                                    \
    }                                                                   \
    hi_int32 ipc_fun(hi_void)                                           \
    {                                                                   \
        return hi_ipc_call("hi_"#ipc_fun"_call", NULL, 0);           \
    }                                                                   \
    hi_int32 __attribute__((section(HI_IPC_SCT_INIT)))                  \
    hi_##ipc_fun##_init(hi_void)                                        \
    {                                                                   \
        hi_ipc_call_node_s st_node = {"hi_"#ipc_fun"_call",             \
                                      hi_##ipc_fun##_call};             \
        return hi_ipc_reg(&st_node);                                    \
    }                                                                   \
    hi_int32 __attribute__((section(HI_IPC_SCT_EXIT)))                  \
    hi_##ipc_fun##_exit(hi_void)                                        \
    {                                                                   \
        return hi_ipc_unreg("hi_"#ipc_fun"_call");                      \
    }                                                                   \
    hi_int32 hi_##ipc_fun##_ipc(hi_void)

#define HI_IPC_IMPL2(count, ...) HI_IPC##count( __VA_ARGS__ )
#define HI_IPC_IMPL(count, ...)  HI_IPC_IMPL2(count, __VA_ARGS__ )
#define HI_DEF_IPC(...)          /*lint -save -e550*/HI_IPC_IMPL(HI_VA_NARGS(__VA_ARGS__), __VA_ARGS__ ) /*lint -restore*/


HI_DEF_IPC(hi_sal_pon_reginfo_get, hi_sal_pon_reg_info_s *, pst_info)
{
    if (up_mode == HI_WAN_ACCESS_TYPE_GE) {
        pst_info->em_olt_auth = HI_SAL_PON_REG_SUCC_E;
        return HI_RET_SUCC;
    }
    PP("pid = %d", getpid());
    pst_info->em_olt_auth = __sal_pon_regstatus_get();
    return HI_RET_SUCC;
}

int main()
{
	// char *str = NULL;
	// getTheCurrentTime(str);
	//
	// time_t t;
	// char buf[1024];
	// time(&t);
	// printf("%d\n", t.tm_sec);

	// ctime_r(&t, buf);
	// printf("%s\n", buf);

	// const char *input = "Aug 28 11:46:45 CST 2025";
	// char output[20] = {0}; // 存储结果：YYYY-MM-DD HH:MM.SS
	// convert_cst_to_iso(input, output, sizeof(output));

	// printf("输入: %s\n", input);
	// printf("输出: %s\n", output);


	// int cpurate = 0;
	// for (int i=0; i<3; i++)
	// {
	// 	 if (1)
	// 	 {
	// 		 cpurate++;
	// 	 }
	// 	 sleep(1);
	// }
	// printf("cpurate = %d", cpurate);

	// printf("%s\n", str);
	return 0;

}
