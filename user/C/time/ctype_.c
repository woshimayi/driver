/*
 * @*************************************:
 * @FilePath: /user/C/time/ctype_.c
 * @version:
 * @Author: dof
 * @Date: 2022-01-22 13:45:08
 * @LastEditors: dof
 * @LastEditTime: 2022-01-22 13:55:06
 * @Descripttion:  数据类型长度定义
 * @**************************************:
 */

#ifdef __COMMDEF_H
#define __COMMDEF_H

typedef unsigned char boolean;	  /* Boolean value type. */
typedef unsigned long int uint32; /* Unsigned 32 bit value */
typedef unsigned short uint16;	  /* Unsigned 16 bit value */
typedef unsigned char uint8;	  /* Unsigned 8 bit value */
typedef signed long int int32;	  /* Signed 32 bit value */
typedef signed short int16;		  /* Signed 16 bit value */
typedef signed char int8;		  /* Signed 8 bit value */

// 得到指定地址上的一个字节或字
#define MEM_B(x) (*((byte *)(x)))2 #defineMEM_W(x)(*((word *)(x)))

// 求最大值和最小值
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// 得到一个field在结构体(struct)中的偏移量
#define FPOS(type, field) \
	/*lint -e545 */ ((dword) & ((type *)0)->field) /*lint +e545 */

// 得到一个结构体中field所占用的字节数
#define FSIZ(type, field) sizeof(((type *)0)->field)

// 按照LSB格式把两个字节转化为一个Word
#define FLIPW(ray) ((((word)(ray)[0]) * 256) + (ray)[1])

//按照LSB格式把一个Word转化为两个字节
#define FLOPW(ray, val)       \
	(ray)[0] = ((val) / 256); \
	(ray)[1] = ((val)&0xFF)

// 得到一个变量的地址(word宽度)
#define B_PTR(var) ((byte *)(void *)&(var))
#define W_PTR(var) ((word *)(void *)&(var))

//  得到一个字的高位和低位字节
#define WORD_LO(xxx) ((byte)((word)(xxx)&255))
#define WORD_HI(xxx) ((byte)((word)(xxx) >> 8))

// 返回一个比X大的最接近的8的倍数
#define UPCASE(c) (((c) >= 'a' && (c) <= 'z') ? ((c)-0x20) : (c))

// 13. 判断字符是不是10进制的数字
#define DECCHK(c) ((c) >= '0' && (c) <= '9')

// 14. 判断字符是不是16进制的数字
#define HEXCHK(c) (((c) >= '0' && (c) <= '9') || \
				   ((c) >= 'A' && (c) <= 'F') || \
				   ((c) >= 'a' && (c) <= 'f'))

// 15. 防止溢出的一个方法
#define INC_SAT(val) (val = ((val) + 1 > (val)) ? (val) + 1 : (val))

// 16. 返回数组元素的个数
#define ARR_SIZE(a) (sizeof((a)) / sizeof((a[0])))

// 17. 返回一个无符号数n尾的值MOD_BY_POWER_OF_TWO(X,n)=X%(2^n)
#define MOD_BY_POWER_OF_TWO(val, mod_by) \
	((dword)(val) & (dword)((mod_by)-1))

// 18. 对于IO空间映射在存储空间的结构，输入输出处理
#define inp(port) (*((volatile byte *)(port)))
#define inpw(port) (*((volatile word *)(port)))
#define inpdw(port) (*((volatile dword *)(port)))
#define outp(port, val) (*((volatile byte *)(port)) = ((byte)(val)))
#define outpw(port, val) (*((volatile word *)(port)) = ((word)(val)))
#define outpdw(port, val) (*((volatile dword *)(port)) = ((dword)(val)))

// 19. 使用一些宏跟踪调试ANSI标准说明了五个预定义的宏名。它们是：
/*
_LINE_
_FILE_
_DATE_
_TIME_
_STDC_

如果编译不是标准的，则可能仅支持以上宏名中的几个，或根本不支持。记住编译程序也许还提供其它预定义的宏名。

_LINE_及_FILE_宏指令在有关#line的部分中已讨论，这里讨论其余的宏名。

_DATE_ 宏指令含有形式为月/日/年的串，表示源文件被翻译到代码时的日期。

源代码翻译到目标代码的时间作为串包含在_TIME_中。串形式为时：分：秒。

如果实现是标准的，则宏_STDC_含有十进制常量1。如果它含有任何其它数，则实现是非标准的。

可以定义宏，例如: 当定义了_DEBUG，输出数据信息和所在文件所在行
*/

#ifdef _DEBUG
#define DEBUGMSG(msg, date) \
	printf(msg);            \
	printf(“% d % d % d”, date, _LINE_, _FILE_)
#else
#define DEBUGMSG(msg, date)
#endif

// 20. 宏定义防止使用时错误用小括号包含。例如：
#difne DO(a, b) do {a + b; a++; } while (0)

#endif
