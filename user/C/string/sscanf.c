/*
 * @*************************************:
 * @FilePath: /user/C/string/sscanf.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2022-06-23 11:59:18
 * @Descripttion:
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
* @param:
* @param:
* @param:
*/
int main(int argc, char *argv[])
{
	char str[128] = "sdfsd_yes_no=2&asda_flag=3";
	int yes = 0;
	int flag = 0;

	// 过滤 分割 &
	sscanf(str, "sdfsd_yes_no=%d&asda_flag=%d", &yes, &flag);
	printf("yes=%d falg=%d\n", yes, flag);

	//  过滤数字
	int sport = 0;
	int eport = 0;

	sscanf("1001-8080", "%d-%d", &sport, &eport);
	printf("%d\n%d\n", sport, eport);

	// 过滤字符串
	char startport[16] = {0};
	char endport[16] = {0};

	sscanf("asd-sdf", "%[a-z]-%[a-z]", startport, endport);
	printf("%s\n%s\n", startport, endport);

	// 拷贝字符串
	char str1[64] = {0};
	sscanf("123456", "%s", str1);
	printf("str = %s\n", str1);

	// 正则表达式的百分号后面多了一个3，这告诉sscanf只拷贝3个字符给str，然后把第4个字符设为null字符
	sscanf("123456", "%3s", str1);
	printf("str = %s\n", str1);

	// 括号里面的a-z就是一个正则表达式，它可以表示从a到z的任意字符，
	// 在继续讨论之前，我们先来看看百分号表示什么意思，%表示选择 ，%后面的是条件，比如实验1的"%s"，s是一个条件，表示任意字符，"%s"的意思是：只要输入的东西是一个字符，就把它拷贝给str。实验2的"%3s"又多了一个条件：只拷贝3个字符。实验3的“%[a-z]”的条件稍微严格一些，输入的东西不但是字符，还得是一个小写字母的字符，所以实验3只拷贝了小写字母"aaa"给str，别忘了加上null字符。
	sscanf("aaaAAA", "%[a-z]", str1);
	printf("str = %s\n", str1);

	// 对于所有字符，只要不是小写字母，都满足"^a-z"正则表达式，符号^表示逻辑非。前3个字符都不是小写字符，所以将其拷贝给str，但最后3个字符也不是小写字母，为什么不拷贝给str呢？这是因为当碰到不满足条件的字符后，sscanf就会停止执行，不再扫描之后的字符
	sscanf("AAAaaaBBB", "%[^a-z]", str1);
	printf("str = %s\n", str1);

	// 这个实验出现了一个新的符号：%*，与%相反，%*表示过滤 满足条件的字符，在这个实验中，%*[A-Z]过滤了所有大写字母，然后再使用%[a-z]把之后的小写字母拷贝给str。如果只有%*，没有%的话，sscanf不会拷贝任何字符到str，这时sscanf的作用仅仅是过滤字符串
	sscanf("AAAaaaBBB", "%*[A-Z]%[a-z]", str1);
	printf("str z = %s\n", str1);

	// 做完前面几个实验后，我们都知道sscanf拷贝完成后，还会在str的后面加上一个null字符，但如果没有一个字符满足条件，sscanf不会在str的后面加null字符，str的值依然是10个惊叹号。这个实验也说明了，如果不使用%*过滤掉前面不需要的字符，你永远别想取得中间的字符
	memset(str1, '\0', sizeof(str1));
	sscanf("AAAaaaBBB", "%[a-z]", str1);
	printf("str z = %s\n", str1);

	// 注意1：%只能使用一次，但%*可以使用多次，比如在这个实验里面，先用%*[A-Z]过滤大写字母，然后用%*[a-z]过滤小写字母。
	// 注意2：^后面可以带多个条件，且这些条件都受^的作用，比如^a-z=表示^a-z且^=(既不是小写字母，也不是等于号)
	memset(str1, '!', sizeof(str1));
	sscanf("AAAaaaBC=", "%*[A-Z]%*[a-z]%[^a-z=]", str1) ;
	printf("str z = %s\n", str1);

	int k = 0;
	int j = 0;
	sscanf("AAA123BBB456", "%*[^0-9]%i%*[A-Z]%d", &k, &j);
	printf("str z = %d j = %d\n", k, j);

	memset(str1, '!', sizeof(str1));
	sscanf("123334abcd123", "%[0-9]*", str1);
	printf("str d = %s\n", str1);


	memset(str1, '!', sizeof(str1));
	char resp_ms[3][32] = {0};
	char ip[32] = {0};
	// sscanf("192.168.1.1| 123.4 345.6 157.5", "%s%[|] %s %s %s", ip, resp_ms[0], resp_ms[1], resp_ms[2]);
	sscanf(" |,*,*,*", "%s%[|,]%s%[^,]%s%[^,]%s", ip, resp_ms[0], resp_ms[1], resp_ms[2]);
	printf("str d = %s %s %s %s\n", ip, resp_ms[0], resp_ms[1], resp_ms[2]);

	return 0;
}
