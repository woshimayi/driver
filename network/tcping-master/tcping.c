#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef BRCM_CMS_BUILD
	#include "mdm.h"
	#include "rut_util.h"
	#include "cms_obj.h"
	#include "cms_log.h"


	void *msgHandle = NULL;
	CmsMsgHeader *msg = NULL;
	int shmId = 0;
#endif
int result = 0;

#define abs(x) ((x) < 0 ? -(x) : (x))

static volatile int stop = 0;

void usage(void)
{
	fprintf(stderr, "tcping  ");
	fprintf(stderr, "hostname (e.g. localhost) [option]\n\n");
	fprintf(stderr, "[Options]\n");
	fprintf(stderr, "   -p portnr	portnumber (e.g. 80)\n");
	fprintf(stderr, "   -c count	how many times to connect\n");
	fprintf(stderr, "   -i interval	delay between each connect\n");
	fprintf(stderr, "   -f flood connect (no delays)\n");
	fprintf(stderr, "   -q quiet, only returncode\n\n");
}

void handler(int sig __attribute__((unused)))
{
	stop = 1;
}

int lookup(char *host, char *portnr, struct addrinfo **res)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_protocol = 0;

	return getaddrinfo(host, portnr, &hints, res);
}

int connect_to(struct addrinfo *addr, struct timeval *rtt)
{
	int fd;
	struct timeval start;
	int connect_result;
	const int on = 1;
	/* int flags; */
	int rv = 0;

	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	/* try to connect for each of the entries: */
	while (addr != NULL)
	{
		/* create socket */
		if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
			goto next_addr0;

		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
			goto next_addr1;

		if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
			goto next_addr1;

		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
			goto next_addr1;


#if 0

		if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
			goto next_addr1;
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
			goto next_addr1;
#endif
		if (gettimeofday(&start, NULL) == -1)
			goto next_addr1;

		/* connect to peer */
		if ((connect_result = connect(fd, addr->ai_addr, addr->ai_addrlen)) == 0)
		{
			if (gettimeofday(rtt, NULL) == -1)
				goto next_addr1;
			rtt->tv_sec = rtt->tv_sec - start.tv_sec;
			rtt->tv_usec = rtt->tv_usec - start.tv_usec;
			close(fd);
			return 0;
		}

next_addr1:
		close(fd);
next_addr0:
		addr = addr->ai_next;
	}

	rv = rv ? rv : -errno;
	return rv;
}

int main(int argc, char *argv[])
{
	char *hostname = NULL;
	char *portnr = "80";
	int c;
	int count = -1, curncount = 0;
	int wait = 1, quiet = 0;
	int ok = 0, err = 0;
	double min = 5000, avg = 0.0, max = 0.0;
	struct addrinfo *resolved;
	int errcode;
	int seen_addrnotavail = 0;

#ifdef BRCM_CMS_BUILD
	int ret = 0;
	cmsLog_init(EID_MDM_TCPING);
	if ((ret = (int)cmsMsg_init(EID_MDM_TCPING, &msgHandle)) != 0)
	{
		cmsLog_cleanup();
		fprintf(stdout, "%d", ret);
		return ret;
	}

	if ((ret = (int)cmsMdm_init(EID_MDM_TCPING, msgHandle, &shmId)) != 0)
	{
		cmsMsg_cleanup(&msgHandle);
		cmsLog_cleanup();
		fprintf(stdout, "%d", ret);
		return ret;
	}

	mdmLibCtx.eid = EID_MDM_TCPING;
#endif

	while ((c = getopt(argc, argv, "h:p:c:i:fq?")) != -1)
	{
		switch (c)
		{
			case 'p':
				portnr = optarg;
				break;

			case 'c':
				count = atoi(optarg);
				break;

			case 'i':
				wait = atoi(optarg);
				break;

			case 'f':
				wait = 0;
				break;

			case 'q':
				quiet = 1;
				break;

			case '?':
			default:
				usage();
				return 0;
		}
	}

	if (optind >= argc)
	{
		fprintf(stderr, "No hostname given\n");
		usage();
		return 3;
	}
	hostname = argv[optind];

	signal(SIGINT, handler);
	signal(SIGTERM, handler);

	if ((errcode = lookup(hostname, portnr, &resolved)) != 0)
	{
		fprintf(stderr, "%s\n", gai_strerror(errcode)); /*用以释放调用getaddrinfo 没有释放的内存*/
		return 2;
	}

	if (!quiet)
		printf("PING %s:%s\n", hostname, portnr);

	while ((curncount < count || count == -1) && stop == 0)
	{
		double ms;
		struct timeval rtt;

		if ((errcode = connect_to(resolved, &rtt)) != 0)
		{
			if (errcode != -EADDRNOTAVAIL)
			{
//				break;
				printf("error connecting to host (%s):%s: -- seq=%d %s\n", hostname, portnr, curncount, strerror(-errcode));
				err++;
			}
			else
			{
				if (seen_addrnotavail)
				{
					printf(".");
					fflush(stdout);
				}
				else
				{
					printf("error connecting to host (%s):%s: ++ seq=%d %s\n", hostname, portnr, curncount, strerror(-errcode));
					result = 3;
				}
				seen_addrnotavail = 1;
			}
		}
		else
		{
			seen_addrnotavail = 0;
			ok++;

			ms = ((double)rtt.tv_sec * 1000.0) + ((double)rtt.tv_usec / 1000.0);
			avg += ms;
			min = min > ms ? ms : min;
			max = max < ms ? ms : max;

			printf("response from %s:%s, seq=%d time=%.3f ms\n", hostname, portnr, curncount, ms);
			if (ms > 500)
				break; /* Stop the test on the first long connect() */
		}

		curncount++;

		if (curncount != count)
			sleep(wait);
	}

	if (!quiet)
	{
		if (errcode != 0)
		{
			printf("--- %s:%s ping statistics ---\n", hostname, portnr);
			printf("%d packets transmitted, %d packets reveived, %.0f%% packet loss\n", curncount, ok,
			       (((double)err) / abs(((double)count)) * 100.0));
			if (100 != ok)
			{
				printf("round-trip min/avg/max = %.3f/%.3f/%.3f ms\n", min, avg / (double)ok, max);
			}

			fprintf(stdout, "unreachable\n");
			result = 4;
		}
		else
		{
			printf("--- %s:%s ping statistics ---\n", hostname, portnr);
			printf("%d packets transmitted, %d packets reveived, %.0f%% packet loss\n", curncount, ok,
			       (((double)err) / abs(((double)count)) * 100.0));
			if (100 != ok)
			{
				printf("round-trip min/avg/max = %.3f/%.3f/%.3f ms\n", min, avg / (double)ok, max);
			}

		}
#ifdef BRCM_CMS_BUILD
		CmsRet ret = CMSRET_SUCCESS;
		_HgTcpingObject *tcpingObj = NULL;
		InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;

		if ((ret = cmsObj_get(MDMOID_HG_TCPING, &iidStack, 0, (void **)&tcpingObj)) != CMSRET_SUCCESS)
		{
			cmsLog_error("Failed to get <MDMOID_HG_TCPING>, ret=%d", ret);
			return ret;
		}

		tcpingObj->result = result;
		tcpingObj->lost = (((double)err) / abs(((double)count)) * 100.0);
		tcpingObj->rtt = (int)(((avg / ok) < 1) ? 1 : (avg / ok));

		ret = cmsObj_set(tcpingObj, &iidStack);
		if (ret != CMSRET_SUCCESS)
		{
			cmsLog_error("cmsObj_set MDMOID_HG_TCPING failed ret=%d", ret);
		}

		cmsObj_free((void **)&tcpingObj);
#endif
	}

	freeaddrinfo(resolved);


#ifdef BRCM_CMS_BUILD
	mdmLibCtx.eid = EID_MDM_TCPING;
	cmsLog_cleanup();
	cmsMdm_cleanup();
	cmsMsg_cleanup(&msgHandle);
#endif

	if (ok)
		return 0;
	else
		return 127;
}

