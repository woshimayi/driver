#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/sem.h>

typedef unsigned long int 	UINT64;
typedef signed   long int 	SINT64;
typedef unsigned int    	UINT32;
typedef signed   int    	SINT32;
typedef unsigned short  	UINT16;
typedef signed   short  	SINT16;
typedef unsigned char   	UINT8;
typedef signed   char    	SINT8;

typedef struct
{
	int sec;   /**< Number of seconds since some arbitrary point. */
	int nsec;  /**< Number of nanoseconds since some arbitrary point. */
} CmsTimestamp;

struct timespec
{
	long tv_sec;
	long tv_nsec;
};

int oal_lock(const UINT32 *timeoutMs)
{
	struct sembuf lockOp[2];
	SINT32 rc = -1;
	UINT32 timeRemainingMs = 0;
	CmsTimestamp startTms, stopTms;
	int ret = -1;
	UBOOL8 extraDebug = FALSE;

	lockOp[0].sem_num = semIndex;
	lockOp[0].sem_op = 0; /* wait for zero: block until write count goes to 0. */
	lockOp[0].sem_flg = 0;

	lockOp[1].sem_num = semIndex;
	lockOp[1].sem_op = 1; /* incr sem count by 1 */
	lockOp[1].sem_flg = SEM_UNDO; /* automatically undo this op if process terminates. */

	if (mdmShmCtx->locked)
	{
		cmsLog_debug("lock currently held by pid=%d func=%s", mdmShmCtx->lockOwner, mdmShmCtx->callerFuncName);
		extraDebug = TRUE;
	}

	if (timeoutMs != NULL)
	{
		timeRemainingMs = *timeoutMs;
	}

	while (TRUE)
	{
		/*
		 * If user specified a timeout, initialize pTimeout and pass it to semtimedop.
		 * If fourth arg to semtimedop is NULL, then it blocks indefinately.
		 */
		if (timeoutMs != NULL)
		{
			struct timespec timeout;

			cmsTms_get(&startTms);
			timeout.tv_sec = timeRemainingMs / MSECS_IN_SEC;
			timeout.tv_nsec = (timeRemainingMs % MSECS_IN_SEC) * NSECS_IN_MSEC;
			rc = semtimedop(semid, lockOp, sizeof(lockOp) / sizeof(struct sembuf), &timeout);
		}
		else
		{
			rc = semop(semid, lockOp, sizeof(lockOp) / sizeof(struct sembuf));
		}

		/*
		 * Our new 2.6.21 MIPS kernel returns the errno in the rc, but my Fedora 7
		 * with 2.6.22 kernel still returns -1 and sets the errno.  So check for both.
		 */
		if ((rc == -1 && errno == EINTR) ||
		        (rc > 0 && rc == EINTR))
		{
			/*
			 * Our semaphore operation was interrupted by a signal or something,
			 * go back to the top of while loop and keep trying.
			 * But if user has specified a timeout, we have to calculate how long
			 * we have waited already, and how much longer we need to wait.
			 */
			if (timeoutMs != NULL)
			{
				UINT32 elapsedMs;

				cmsTms_get(&stopTms);
				elapsedMs = cmsTms_deltaInMilliSeconds(&stopTms, &startTms);

				if (elapsedMs >= timeRemainingMs)
				{
					/* even though we woke up because of EINTR, we have waited long enough */
					rc = EAGAIN;
					break;
				}
				else
				{
					/* subtract the time we already waited and wait some more */
					timeRemainingMs -= elapsedMs;
				}
			}
		}
		else
		{
			/* If we get any error other than EINTR, break out of the loop */
			break;
		}
	}

	if (extraDebug)
	{
		cmsLog_debug("lock grab result, rc=%d errno=%d", rc, errno);
	}

	if (rc != 0)
	{
		/*
		 * most likely cause of error is caught signal, we could also
		 * get EIDRM if someone deletes the semphore while we are waiting
		 * for it (that indicates programming error.)
		 */
		if (errno == EINTR || rc == EINTR)
		{
			cmsLog_notice("lock interrupted by signal");
			ret = CMSRET_OP_INTR;
		}
		else if (errno == EAGAIN || rc == EAGAIN)
		{
			/* the new 2.6.21 kernel seems to return the errno in the rc */
			cmsLog_debug("timed out, errno=%d rc=%d", errno, rc);
			return CMSRET_TIMED_OUT;
		}
		else
		{
			cmsLog_error("lock failed, errno=%d rc=%d", errno, rc);
			ret = CMSRET_INTERNAL_ERROR;
		}
	}
	else
	{
		/* I got the lock! */

		/*
		 * Because of the SEM_UNDO feature, when I acquire a lock,
		 * if I notice that my mdmShmCtx does not have the same info,
		 * then that means the previous owner died suddenly and did not clean up.
		 * Update my mdmShmCtx structure to reflect reality.
		 */
		if ((mdmShmCtx->locked) || (mdmShmCtx->lockOwner != CMS_INVALID_PID))
		{
			cmsLog_notice("correcting stale lock data from pid %d", mdmShmCtx->lockOwner);
			mdmShmCtx->locked = FALSE;
			mdmShmCtx->lockOwner = CMS_INVALID_PID;
		}

		/* update my lock tracking variables */
		mdmLibCtx.locked = TRUE;
		mdmShmCtx->locked = TRUE;
		mdmShmCtx->lockOwner = cmsEid_getPid();
	}

	return ret;
}



/** OS dependent timestamp functions go in this file.
 */
void oalTms_get(CmsTimestamp *tms)
{
	struct timespec ts;
	int rc;

	if (tms == NULL)
	{
		return;
	}

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (rc == 0)
	{
		tms->sec = ts.tv_sec;
		tms->nsec = ts.tv_nsec;
	}
	else
	{
		printf("clock_gettime failed, set timestamp to 0");
		tms->sec = 0;
		tms->nsec = 0;
	}
}


void cmsTms_get(CmsTimestamp *tms)
{
	oalTms_get(tms);
}


int cmsLck_acquireLockWithTimeoutTraced(const char *callerFuncName, int timeoutMilliSeconds)
{
	int to = timeoutMilliSeconds;
	int ret;
	CmsTimestamp cmsTimestamp;

	if ((ret = oal_lock(&to)) == 0)
	{
		//save acquired timestamp and caller function name;
		cmsTms_get(&cmsTimestamp);
		//strncpy(mdmShmCtx->callerFuncName, callerFuncName, sizeof(mdmShmCtx->callerFuncName)-1);
		printf("acquired lock. callerFuncName %s; timeout %d milliseconds\n", callerFuncName, timeoutMilliSeconds);
	}
	return ret;
}



int main()
{

	return 0;
}


