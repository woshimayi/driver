#if 0
	shc Version 4.0.3, Generic Shell Script Compiler
	GNU GPL Version 3 Md Jahidul Hamid <jahidulhamid@yahoo.com>

	shc -f shell_test.sh 
#endif

static  char data [] = 
#define      tst2_z	19
#define      tst2	((&data[2]))
	"\255\064\071\375\133\203\267\221\244\043\262\256\101\061\364\141"
	"\366\372\067\003\326\362\067\316\017"
#define      tst1_z	22
#define      tst1	((&data[26]))
	"\201\030\006\056\014\210\366\040\131\065\046\256\370\112\041\063"
	"\153\170\012\377\307\065\353\352\364\204\015\143"
#define      chk1_z	22
#define      chk1	((&data[58]))
	"\142\206\072\377\062\342\371\120\367\260\035\327\324\377\363\312"
	"\103\206\172\047\233\224\146\354\264\066\055\107\335\143"
#define      msg1_z	65
#define      msg1	((&data[88]))
	"\236\105\362\323\070\071\331\244\224\353\157\304\113\231\041\215"
	"\321\126\376\250\107\067\052\370\161\240\021\165\330\240\370\311"
	"\103\201\302\063\141\160\254\035\240\375\172\004\145\046\262\236"
	"\322\132\203\057\153\063\277\337\323\363\304\027\210\167\216\163"
	"\005\125\223\047\077\227\051\241\107\262"
#define      chk2_z	19
#define      chk2	((&data[158]))
	"\320\221\034\120\137\014\155\302\120\017\253\142\233\352\100\006"
	"\201\101\245\061\060"
#define      rlax_z	1
#define      rlax	((&data[178]))
	"\250"
#define      inlo_z	3
#define      inlo	((&data[179]))
	"\233\261\072"
#define      opts_z	1
#define      opts	((&data[182]))
	"\165"
#define      text_z	531
#define      text	((&data[252]))
	"\212\351\306\334\056\145\042\041\070\132\113\332\242\375\277\336"
	"\060\217\017\351\155\243\064\073\312\341\217\054\216\025\325\030"
	"\377\233\364\055\000\026\117\071\161\232\023\023\227\322\362\310"
	"\142\001\261\320\245\346\013\160\307\232\234\125\257\161\156\256"
	"\015\143\334\016\171\367\244\371\043\031\133\000\173\345\265\246"
	"\157\136\215\061\136\134\302\127\302\242\233\002\213\150\111\302"
	"\272\046\027\031\207\311\250\254\371\112\272\000\256\231\135\262"
	"\127\206\252\335\217\016\353\147\042\101\201\051\263\160\324\121"
	"\366\225\235\361\263\206\257\332\131\130\211\324\030\252\250\305"
	"\115\170\275\013\227\322\324\061\253\257\140\127\131\063\133\034"
	"\324\234\045\101\245\024\151\140\122\171\156\264\175\306\243\234"
	"\123\333\307\107\200\222\144\175\103\262\260\011\124\067\237\230"
	"\072\060\236\373\341\120\303\122\054\053\070\161\047\031\215\056"
	"\226\235\142\100\363\073\206\016\317\062\234\057\124\370\142\070"
	"\250\047\371\073\214\146\302\176\012\050\141\152\254\326\242\043"
	"\151\125\236\035\100\021\113\206\015\332\347\226\364\210\354\025"
	"\233\350\006\245\273\202\256\022\222\362\106\117\031\242\075\047"
	"\201\237\225\273\014\255\175\306\160\041\326\231\314\072\016\367"
	"\133\154\125\164\111\164\374\040\316\035\151\244\131\144\207\230"
	"\131\141\054\214\216\324\163\357\367\152\171\270\300\167\206\227"
	"\253\043\057\223\100\055\012\247\350\171\326\345\255\363\333\127"
	"\007\140\320\351\264\325\053\277\047\377\026\107\355\021\114\000"
	"\311\244\004\155\057\361\026\004\004\031\022\033\273\246\024\352"
	"\253\203\126\057\030\321\272\244\042\257\312\025\212\375\266\272"
	"\211\253\351\156\027\264\016\317\001\375\215\035\173\120\320\011"
	"\334\351\121\033\137\103\377\000\103\342\022\213\374\241\274\372"
	"\051\100\062\050\340\202\032\243\136\241\243\354\255\234\050\143"
	"\113\134\066\032\033\251\372\165\046\060\302\156\076\216\363\357"
	"\135\226\157\103\313\317\140\326\354\103\001\236\167\230\357\325"
	"\224\330\136\250\050\212\144\357\206\322\221\333\221\135\342\036"
	"\047\345\031\266\166\317\271\307\113\206\360\256\013\271\133\241"
	"\207\363\150\002\122\072\111\361\115\013\226\046\053\334\221\216"
	"\112\124\104\061\264\141\237\051\076\374\046\253\151\011\022\256"
	"\376\236\006\271\035\132\145\365\323\363\237\036\270\170\315\073"
	"\225\112\343\273\322\372\046\142\066\136\326\016\320\167\204\152"
	"\034\032\206\012\176\004\206\176\037\235\216\117\264\260\376\266"
	"\063\272\167\110\307\012\223\054\215\005\247\253\123\076\317\054"
	"\231\174\106\006\143\141\240\076\053\107\353\306\132\376\135\055"
	"\360\046\220\362\330\140\227\276\154\007\205\006\244\333\266\025"
	"\111\145\043\254\101\061\046\155\170\021\063\323\020\221\000\001"
	"\267\221\363\220\361\213\116\135\222\323\144\067\256\032\114\370"
	"\177\160\244\301\241\312\057\032\334\143\355\354\365\356\355\255"
	"\177\341\075\161\154\213\316\377\137\062\066\016\115\203\006"
#define      pswd_z	256
#define      pswd	((&data[931]))
	"\217\225\165\276\257\122\042\235\076\027\214\054\304\013\016\001"
	"\174\173\214\113\173\353\176\261\371\313\065\000\231\051\253\050"
	"\277\041\346\157\163\011\014\262\040\230\337\345\244\356\346\041"
	"\151\163\155\344\137\353\226\131\267\314\131\120\366\306\135\306"
	"\274\245\314\163\167\067\233\333\255\346\214\214\107\050\105\101"
	"\311\335\233\113\112\361\171\175\245\347\176\057\255\333\365\151"
	"\201\302\335\370\371\170\323\246\137\137\063\247\207\171\350\121"
	"\126\203\235\241\165\026\036\032\376\235\112\253\171\100\025\372"
	"\002\362\362\374\153\305\242\312\045\326\161\255\120\131\377\247"
	"\335\234\110\122\263\147\155\261\004\267\135\175\367\163\170\372"
	"\146\152\366\321\060\231\233\126\157\014\003\300\146\002\147\103"
	"\237\260\226\122\027\003\004\034\273\141\232\263\324\022\255\073"
	"\175\243\014\256\074\247\004\253\264\010\154\032\013\323\136\252"
	"\203\364\375\233\370\001\270\264\143\123\147\070\146\024\163\344"
	"\267\177\222\363\047\226\237\333\236\013\366\251\337\125\124\143"
	"\112\121\377\103\123\270\367\266\013\136\356\162\162\142\126\051"
	"\341\350\035\011\177\274\345\036\310\334\307\247\061\034\012\173"
	"\155\012\276\301\302\265\167\315\023\146\100\206\310\226\257\252"
	"\177\314\264\376\211\231\034\121\165\344\370\247\000\003\042\156"
	"\015\341\057\317\227\247\235\253\015\335\061\326\164\005\170\265"
	"\046\137\044\232\150\061\114\210\312\054\155\157\032\124\220\203"
	"\307\375\150\047\351\376\200\240\313\332\360\301\337\150\167\005"
	"\307\234\240\057\315\355\270\230"
#define      date_z	1
#define      date	((&data[1230]))
	"\077"
#define      shll_z	10
#define      shll	((&data[1232]))
	"\270\116\371\036\154\354\162\047\257\062\350\103"
#define      msg2_z	19
#define      msg2	((&data[1243]))
	"\215\126\204\320\340\007\163\275\033\241\204\126\120\132\000\376"
	"\143\164\365\176\037"
#define      xecc_z	15
#define      xecc	((&data[1264]))
	"\262\207\313\116\023\274\044\166\064\337\164\214\217\125\217\305"
	"\017\254"
#define      lsto_z	1
#define      lsto	((&data[1282]))
	"\362"/* End of data[] */;
#define      hide_z	4096
#define SETUID 0	/* Define as 1 to call setuid(0) at start of script */
#define DEBUGEXEC	0	/* Define as 1 to debug execvp calls */
#define TRACEABLE	1	/* Define as 1 to enable ptrace the executable */
#define HARDENING	0	/* Define as 1 to disable ptrace/dump the executable */
#define BUSYBOXON	0	/* Define as 1 to enable work with busybox */

#if HARDENING
static const char * shc_x[] = {
"/*",
" * Copyright 2019 - Intika <intika@librefox.org>",
" * Replace ******** with secret read from fd 21",
" * Also change arguments location of sub commands (sh script commands)",
" * gcc -Wall -fpic -shared -o shc_secret.so shc_secret.c -ldl",
" */",
"",
"#define _GNU_SOURCE /* needed to get RTLD_NEXT defined in dlfcn.h */",
"#define PLACEHOLDER \"********\"",
"#include <dlfcn.h>",
"#include <stdlib.h>",
"#include <string.h>",
"#include <unistd.h>",
"#include <stdio.h>",
"#include <signal.h>",
"",
"static char secret[128000]; //max size",
"typedef int (*pfi)(int, char **, char **);",
"static pfi real_main;",
"",
"// copy argv to new location",
"char **copyargs(int argc, char** argv){",
"    char **newargv = malloc((argc+1)*sizeof(*argv));",
"    char *from,*to;",
"    int i,len;",
"",
"    for(i = 0; i<argc; i++){",
"        from = argv[i];",
"        len = strlen(from)+1;",
"        to = malloc(len);",
"        memcpy(to,from,len);",
"        // zap old argv space",
"        memset(from,'\\0',len);",
"        newargv[i] = to;",
"        argv[i] = 0;",
"    }",
"    newargv[argc] = 0;",
"    return newargv;",
"}",
"",
"static int mymain(int argc, char** argv, char** env) {",
"    //fprintf(stderr, \"Inject main argc = %d\\n\", argc);",
"    return real_main(argc, copyargs(argc,argv), env);",
"}",
"",
"int __libc_start_main(int (*main) (int, char**, char**),",
"                      int argc,",
"                      char **argv,",
"                      void (*init) (void),",
"                      void (*fini)(void),",
"                      void (*rtld_fini)(void),",
"                      void (*stack_end)){",
"    static int (*real___libc_start_main)() = NULL;",
"    int n;",
"",
"    if (!real___libc_start_main) {",
"        real___libc_start_main = dlsym(RTLD_NEXT, \"__libc_start_main\");",
"        if (!real___libc_start_main) abort();",
"    }",
"",
"    n = read(21, secret, sizeof(secret));",
"    if (n > 0) {",
"      int i;",
"",
"    if (secret[n - 1] == '\\n') secret[--n] = '\\0';",
"    for (i = 1; i < argc; i++)",
"        if (strcmp(argv[i], PLACEHOLDER) == 0)",
"          argv[i] = secret;",
"    }",
"",
"    real_main = main;",
"",
"    return real___libc_start_main(mymain, argc, argv, init, fini, rtld_fini, stack_end);",
"}",
"",
0};
#endif /* HARDENING */

/* rtc.c */

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* 'Alleged RC4' */

static unsigned char stte[256], indx, jndx, kndx;

/*
 * Reset arc4 stte. 
 */
void stte_0(void)
{
	indx = jndx = kndx = 0;
	do {
		stte[indx] = indx;
	} while (++indx);
}

/*
 * Set key. Can be used more than once. 
 */
void key(void * str, int len)
{
	unsigned char tmp, * ptr = (unsigned char *)str;
	while (len > 0) {
		do {
			tmp = stte[indx];
			kndx += tmp;
			kndx += ptr[(int)indx % len];
			stte[indx] = stte[kndx];
			stte[kndx] = tmp;
		} while (++indx);
		ptr += 256;
		len -= 256;
	}
}

/*
 * Crypt data. 
 */
void arc4(void * str, int len)
{
	unsigned char tmp, * ptr = (unsigned char *)str;
	while (len > 0) {
		indx++;
		tmp = stte[indx];
		jndx += tmp;
		stte[indx] = stte[jndx];
		stte[jndx] = tmp;
		tmp += stte[indx];
		*ptr ^= stte[tmp];
		ptr++;
		len--;
	}
}

/* End of ARC4 */

#if HARDENING

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>
#define PR_SET_PTRACER 0x59616d61

/* Seccomp Sandboxing Init */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/audit.h>

#define ArchField offsetof(struct seccomp_data, arch)

#define Allow(syscall) \
    BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, SYS_##syscall, 0, 1), \
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

struct sock_filter filter[] = {
    /* validate arch */
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, ArchField),
    BPF_JUMP( BPF_JMP+BPF_JEQ+BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL),

    /* load syscall */
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, offsetof(struct seccomp_data, nr)),

    /* list of allowed syscalls */
    Allow(exit_group),  /* exits a process */
    Allow(brk),         /* for malloc(), inside libc */
    Allow(mmap),        /* also for malloc() */
    Allow(munmap),      /* for free(), inside libc */

    /* and if we don't match above, die */
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL),
};
struct sock_fprog filterprog = {
    .len = sizeof(filter)/sizeof(filter[0]),
    .filter = filter
};

/* Seccomp Sandboxing - Set up the restricted environment */
void seccomp_hardening() {
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("Could not start seccomp:");
        exit(1);
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filterprog) == -1) {
        perror("Could not start seccomp:");
        exit(1);
    }
} 
/* End Seccomp Sandboxing Init */

void shc_x_file() {
    FILE *fp;
    int line = 0;

    if ((fp = fopen("/tmp/shc_x.c", "w")) == NULL ) {exit(1); exit(1);}
    for (line = 0; shc_x[line]; line++)	fprintf(fp, "%s\n", shc_x[line]);
    fflush(fp);fclose(fp);
}

int make() {
	char * cc, * cflags, * ldflags;
    char cmd[4096];

	cc = getenv("CC");
	if (!cc) cc = "cc";

	sprintf(cmd, "%s %s -o %s %s", cc, "-Wall -fpic -shared", "/tmp/shc_x.so", "/tmp/shc_x.c -ldl");
	if (system(cmd)) {remove("/tmp/shc_x.c"); return -1;}
	remove("/tmp/shc_x.c"); return 0;
}

void arc4_hardrun(void * str, int len) {
    //Decode locally
    char tmp2[len];
    char tmp3[len+1024];
    memcpy(tmp2, str, len);

	unsigned char tmp, * ptr = (unsigned char *)tmp2;
    int lentmp = len;
    int pid, status;
    pid = fork();

    shc_x_file();
    if (make()) {exit(1);}

    setenv("LD_PRELOAD","/tmp/shc_x.so",1);

    if(pid==0) {

        //Start tracing to protect from dump & trace
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            kill(getpid(), SIGKILL);
            _exit(1);
        }

        //Decode Bash
        while (len > 0) {
            indx++;
            tmp = stte[indx];
            jndx += tmp;
            stte[indx] = stte[jndx];
            stte[jndx] = tmp;
            tmp += stte[indx];
            *ptr ^= stte[tmp];
            ptr++;
            len--;
        }

        //Do the magic
        sprintf(tmp3, "%s %s", "'********' 21<<<", tmp2);

        //Exec bash script //fork execl with 'sh -c'
        system(tmp2);

        //Empty script variable
        memcpy(tmp2, str, lentmp);

        //Clean temp
        remove("/tmp/shc_x.so");

        //Sinal to detach ptrace
        ptrace(PTRACE_DETACH, 0, 0, 0);
        exit(0);
    }
    else {wait(&status);}

    /* Seccomp Sandboxing - Start */
    seccomp_hardening();

    exit(0);
}
#endif /* HARDENING */

/*
 * Key with file invariants. 
 */
int key_with_file(char * file)
{
	struct stat statf[1];
	struct stat control[1];

	if (stat(file, statf) < 0)
		return -1;

	/* Turn on stable fields */
	memset(control, 0, sizeof(control));
	control->st_ino = statf->st_ino;
	control->st_dev = statf->st_dev;
	control->st_rdev = statf->st_rdev;
	control->st_uid = statf->st_uid;
	control->st_gid = statf->st_gid;
	control->st_size = statf->st_size;
	control->st_mtime = statf->st_mtime;
	control->st_ctime = statf->st_ctime;
	key(control, sizeof(control));
	return 0;
}

#if DEBUGEXEC
void debugexec(char * sh11, int argc, char ** argv)
{
	int i;
	fprintf(stderr, "shll=%s\n", sh11 ? sh11 : "<null>");
	fprintf(stderr, "argc=%d\n", argc);
	if (!argv) {
		fprintf(stderr, "argv=<null>\n");
	} else { 
		for (i = 0; i <= argc ; i++)
			fprintf(stderr, "argv[%d]=%.60s\n", i, argv[i] ? argv[i] : "<null>");
	}
}
#endif /* DEBUGEXEC */

void rmarg(char ** argv, char * arg)
{
	for (; argv && *argv && *argv != arg; argv++);
	for (; argv && *argv; argv++)
		*argv = argv[1];
}

void chkenv_end(void);

int chkenv(int argc)
{
	char buff[512];
	unsigned long mask, m;
	int l, a, c;
	char * string;
	extern char ** environ;

	mask = (unsigned long)getpid();
	stte_0();
	 key(&chkenv, (void*)&chkenv_end - (void*)&chkenv);
	 key(&data, sizeof(data));
	 key(&mask, sizeof(mask));
	arc4(&mask, sizeof(mask));
	sprintf(buff, "x%lx", mask);
	string = getenv(buff);
#if DEBUGEXEC
	fprintf(stderr, "getenv(%s)=%s\n", buff, string ? string : "<null>");
#endif
	l = strlen(buff);
	if (!string) {
		/* 1st */
		sprintf(&buff[l], "=%lu %d", mask, argc);
		putenv(strdup(buff));
		return 0;
	}
	c = sscanf(string, "%lu %d%c", &m, &a, buff);
	if (c == 2 && m == mask) {
		/* 3rd */
		rmarg(environ, &string[-l - 1]);
		return 1 + (argc - a);
	}
	return -1;
}

void chkenv_end(void){}

#if HARDENING

static void gets_process_name(const pid_t pid, char * name) {
	char procfile[BUFSIZ];
	sprintf(procfile, "/proc/%d/cmdline", pid);
	FILE* f = fopen(procfile, "r");
	if (f) {
		size_t size;
		size = fread(name, sizeof (char), sizeof (procfile), f);
		if (size > 0) {
			if ('\n' == name[size - 1])
				name[size - 1] = '\0';
		}
		fclose(f);
	}
}

void hardening() {
    prctl(PR_SET_DUMPABLE, 0);
    prctl(PR_SET_PTRACER, -1);

    int pid = getppid();
    char name[256] = {0};
    gets_process_name(pid, name);

    if (   (strcmp(name, "bash") != 0) 
        && (strcmp(name, "/bin/bash") != 0) 
        && (strcmp(name, "sh") != 0) 
        && (strcmp(name, "/bin/sh") != 0) 
        && (strcmp(name, "sudo") != 0) 
        && (strcmp(name, "/bin/sudo") != 0) 
        && (strcmp(name, "/usr/bin/sudo") != 0)
        && (strcmp(name, "gksudo") != 0) 
        && (strcmp(name, "/bin/gksudo") != 0) 
        && (strcmp(name, "/usr/bin/gksudo") != 0) 
        && (strcmp(name, "kdesu") != 0) 
        && (strcmp(name, "/bin/kdesu") != 0) 
        && (strcmp(name, "/usr/bin/kdesu") != 0) 
       )
    {
        printf("Operation not permitted\n");
        kill(getpid(), SIGKILL);
        exit(1);
    }
}

#endif /* HARDENING */

#if !TRACEABLE

#define _LINUX_SOURCE_COMPAT
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#if !defined(PT_ATTACHEXC) /* New replacement for PT_ATTACH */
   #if !defined(PTRACE_ATTACH) && defined(PT_ATTACH)
       #define PT_ATTACHEXC	PT_ATTACH
   #elif defined(PTRACE_ATTACH)
       #define PT_ATTACHEXC PTRACE_ATTACH
   #endif
#endif

void untraceable(char * argv0)
{
	char proc[80];
	int pid, mine;

	switch(pid = fork()) {
	case  0:
		pid = getppid();
		/* For problematic SunOS ptrace */
#if defined(__FreeBSD__)
		sprintf(proc, "/proc/%d/mem", (int)pid);
#else
		sprintf(proc, "/proc/%d/as",  (int)pid);
#endif
		close(0);
		mine = !open(proc, O_RDWR|O_EXCL);
		if (!mine && errno != EBUSY)
			mine = !ptrace(PT_ATTACHEXC, pid, 0, 0);
		if (mine) {
			kill(pid, SIGCONT);
		} else {
			perror(argv0);
			kill(pid, SIGKILL);
		}
		_exit(mine);
	case -1:
		break;
	default:
		if (pid == waitpid(pid, 0, 0))
			return;
	}
	perror(argv0);
	_exit(1);
}
#endif /* !TRACEABLE */

char * xsh(int argc, char ** argv)
{
	char * scrpt;
	int ret, i, j;
	char ** varg;
	char * me = argv[0];
	if (me == NULL) { me = getenv("_"); }
	if (me == 0) { fprintf(stderr, "E: neither argv[0] nor $_ works."); exit(1); }

	ret = chkenv(argc);
	stte_0();
	 key(pswd, pswd_z);
	arc4(msg1, msg1_z);
	arc4(date, date_z);
	if (date[0] && (atoll(date)<time(NULL)))
		return msg1;
	arc4(shll, shll_z);
	arc4(inlo, inlo_z);
	arc4(xecc, xecc_z);
	arc4(lsto, lsto_z);
	arc4(tst1, tst1_z);
	 key(tst1, tst1_z);
	arc4(chk1, chk1_z);
	if ((chk1_z != tst1_z) || memcmp(tst1, chk1, tst1_z))
		return tst1;
	arc4(msg2, msg2_z);
	if (ret < 0)
		return msg2;
	varg = (char **)calloc(argc + 10, sizeof(char *));
	if (!varg)
		return 0;
	if (ret) {
		arc4(rlax, rlax_z);
		if (!rlax[0] && key_with_file(shll))
			return shll;
		arc4(opts, opts_z);
#if HARDENING
	    arc4_hardrun(text, text_z);
	    exit(0);
       /* Seccomp Sandboxing - Start */
       seccomp_hardening();
#endif
		arc4(text, text_z);
		arc4(tst2, tst2_z);
		 key(tst2, tst2_z);
		arc4(chk2, chk2_z);
		if ((chk2_z != tst2_z) || memcmp(tst2, chk2, tst2_z))
			return tst2;
		/* Prepend hide_z spaces to script text to hide it. */
		scrpt = malloc(hide_z + text_z);
		if (!scrpt)
			return 0;
		memset(scrpt, (int) ' ', hide_z);
		memcpy(&scrpt[hide_z], text, text_z);
	} else {			/* Reexecute */
		if (*xecc) {
			scrpt = malloc(512);
			if (!scrpt)
				return 0;
			sprintf(scrpt, xecc, me);
		} else {
			scrpt = me;
		}
	}
	j = 0;
#if BUSYBOXON
	varg[j++] = "busybox";
	varg[j++] = "sh";
#else
	varg[j++] = argv[0];		/* My own name at execution */
#endif
	if (ret && *opts)
		varg[j++] = opts;	/* Options on 1st line of code */
	if (*inlo)
		varg[j++] = inlo;	/* Option introducing inline code */
	varg[j++] = scrpt;		/* The script itself */
	if (*lsto)
		varg[j++] = lsto;	/* Option meaning last option */
	i = (ret > 1) ? ret : 0;	/* Args numbering correction */
	while (i < argc)
		varg[j++] = argv[i++];	/* Main run-time arguments */
	varg[j] = 0;			/* NULL terminated array */
#if DEBUGEXEC
	debugexec(shll, j, varg);
#endif
	execvp(shll, varg);
	return shll;
}

int main(int argc, char ** argv)
{
#if SETUID
   setuid(0);
#endif
#if DEBUGEXEC
	debugexec("main", argc, argv);
#endif
#if HARDENING
	hardening();
#endif
#if !TRACEABLE
	untraceable(argv[0]);
#endif
	argv[1] = xsh(argc, argv);
	fprintf(stderr, "%s%s%s: %s\n", argv[0],
		errno ? ": " : "",
		errno ? strerror(errno) : "",
		argv[1] ? argv[1] : "<null>"
	);
	return 1;
}
