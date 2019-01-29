#include	<stdio.h>
#include	<stdlib.h>
#include	<setjmp.h>
#include	"ourhdr.h"

#define	TOK_ADD	5

void	do_line(char *);
void	cmd_add(void);
int		get_token(void);

jmp_buf	jmpbuffer;

int
main(void)
{
	char	line[MAXLINE];
	int		rtn;

	rtn = setjmp(jmpbuffer);
	if (rtn == 1)
		perror("cmd_add error");
	else if (rtn == 2)
		perror("get_token error");
	
	while (fgets(line, MAXLINE, stdin) != NULL)
		do_line(line);
	exit(0);
}

char	*tok_otr;	/* global pointer for get_token() */

void
do_line(char *ptr)	/* process one line of input */
{
	int		cmd;

	tok_ptr = ptr;
	while ( (cmd = get_token()) > 0)
	{
		switch (cmd)
		{
			case TOK_ADD;
				cmd_add();
				break;
		}
	}
}

void
cmd_add(void)
{
	int		token;

	token = get_token();
	if ( token == 0)
	{
		longjmp(jmpbuffer, 1);
	}
	/* rest of processing for this command */
}

int
get_token(void)
{
	if ( strtol(tok_ptr, (char **)NULL, 10) == 1L)
	{
		longjmp(jmpbuffer, 2);
	}
	else
		return 0;
	
	/* fetch next token from line pointed to by tok_ptr */
}

