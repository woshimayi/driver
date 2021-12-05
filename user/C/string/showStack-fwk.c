/*
 * @*************************************: 
 * @FilePath: \undefinedc:\Users\ZS-offic\Documents\C\showStack.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-18 18:06:12
 * @LastEditors: dof
 * @LastEditTime: 2021-10-18 18:12:09
 * @Descripttion: 
 * @**************************************: 
 */

/*
 * gcc -g -rdynamic showStack.c 
 * addr2line 0x4009b7 -e a.out -f -C -s
 *
 *
 * */
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <stdlib.h>
#ifdef DESKTOP_LINUX
#include <execinfo.h>
#endif


#define FWK_BACKTRACE_DEPTH  (30)
#define ABS(s)  ((s) < 0? -(s) : (s))


/* this is in standard Intel libc, we implement it locally for MIPS */
int FWK_btBacktrace(void **buffer, int size, ucontext_t *uc)
{
#ifdef DESKTOP_LINUX
	return backtrace(buffer, size);
#else
	unsigned long *addr;
	unsigned long *ra;
	unsigned long *sp;
	size_t ra_offset;
	size_t stack_size;
	int depth = 0;

	if (!size)
		return 0;
	if (!buffer || size < 0)
		return -1;

	ra = (unsigned long *)(unsigned long)uc->uc_mcontext.pc;
	sp = (unsigned long *)(unsigned long)uc->uc_mcontext.gregs[29];

	/* Repeat backward scanning */
	for (depth = 0; depth < size && ra; ++depth) {
		buffer[depth] = ra;
		ra_offset = 0;
		stack_size = 0;

		for (addr = ra; !ra_offset || !stack_size; --addr) {
			switch (*addr & 0xffff0000) {
			case 0x27bd0000: /* addiu sp, sp, -stack_size */
				if (ra_offset == 0)
				{
					/*
					* Try to deal with possible multiple expansion of stack
					* in a single function.  If this doesn't work, just do
					* return depth+1;
					*/
					short stack_size_x;
					stack_size_x = (short)(*addr & 0xffff);
					if (stack_size_x < 0)
						sp = (unsigned long *)((unsigned long)sp - stack_size_x);
				}
				else
				{
					stack_size = ABS((short)(*addr & 0xffff));
				}
				break;
			case 0xafbf0000: /* sw ra, offset */
				ra_offset = (short)(*addr & 0xffff);
				break;
#if 0
			case 0x3c1c0000: /* lui gp, constant */
				return depth + 1;
#endif
			case 0x03e00000:
				if (*addr == 0x03e00008)  /* jr ra */
				{
					return depth + 1;
				}
				break;

			default:
				break;
			}
		}
		ra = *(unsigned long **)((unsigned long)sp + ra_offset);
		sp = (unsigned long *)((unsigned long)sp + stack_size);
	}

	return depth;
#endif
}


void FWK_btPrintBacktrace(void **buffer, int size)
{
#ifdef DESKTOP_LINUX
	backtrace_symbols_fd(buffer, size, 1);
#else
	FILE *fp;
	int i;

	fp = fopen("/proc/self/maps","r");

	for (i = 0; i < size && 0 < (unsigned long) buffer[i]; i++) {
		char line[1024];
		int found = 0;

		rewind(fp);
		while (fgets(line, sizeof(line), fp)) {
			char lib[1024];
			void *start, *end;
			unsigned int offset;
			int n = sscanf(line, "%p-%p %*s %x %*s %*d %s",
					   &start, &end, &offset, lib);
			if (n == 4 && buffer[i] >= start && buffer[i] < end) {
				if (buffer[i] < (void*)0x10000000)
					offset = (unsigned int)buffer[i];
				else
					offset += buffer[i] - start;

				fprintf(stderr, "#%02d  [%08x] in %s\n", i, offset, lib);
				found = 1;
				break;
			}
		}
		if (!found)
			fprintf(stderr, "#%02d  [%08x]\n", i, (unsigned)buffer[i]);
	}
	fclose(fp);
#endif
}


void FWK_btSigHandler(int sig, siginfo_t *info, void *secret)
{
	void *traceBuf[FWK_BACKTRACE_DEPTH] = {NULL};
	int traceBufSize = 0;

	if (SIGUSR2 != sig)
	{
		fprintf(stderr, "\nFatal Error!Process exit due to received signal %d, dump backtrace\n", sig);
	}
	else
	{
		fprintf(stderr, "\nReceived signal %d, dump backtrace\n", sig);
	}

	traceBufSize = FWK_btBacktrace(traceBuf, sizeof(traceBuf) / sizeof(traceBuf[0]), (ucontext_t *)secret);
	FWK_btPrintBacktrace(traceBuf, traceBufSize);
	fflush(stderr);

	if (SIGUSR2 != sig)
	{
		exit(0);
	}
}
 

static void fwk_btRegSigAction(int sig, int resetHand)
{
	struct sigaction action;

	action.sa_sigaction = (void *)FWK_btSigHandler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, sig);
	action.sa_flags = SA_NODEFER | SA_SIGINFO | (resetHand ? SA_RESETHAND : 0);
	sigaction(sig, &action, NULL);
}


void FWK_btInit(int spySig)
{
	fwk_btRegSigAction(SIGSEGV, 1);
	fwk_btRegSigAction(SIGABRT, 1);
	fwk_btRegSigAction(SIGBUS, 1);
	fwk_btRegSigAction(spySig > 0 ? spySig : SIGUSR2, 0);
}


void fun3()
{
	printf("this is fun3\n");
	*(char *)0 = 1;
}

void fun2()
{
	printf("this is fun3\n");
	fun3();
}

void fun1()
{
	printf("this is fun3\n");
	fun2();
}

int main(int argc, char const *argv[])
{
	FWK_btInit(0);
	fun1();
	return 0;
}
