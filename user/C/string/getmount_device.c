/*
 * @*************************************:
 * @FilePath: /user/C/string/getmount_device.c
 * @version:
 * @Author: dof
 * @Date: 2022-12-09 10:35:19
 * @LastEditors: dof
 * @LastEditTime: 2022-12-09 10:35:24
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <mntent.h>

int main(void)
{
	FILE *mtab;
	struct mntent *mnt_p;

	mtab = setmntent("/etc/mtab", "r");

	while ((mnt_p = getmntent(mtab)) != NULL)
	{
		if (strcmp(mnt_p->mnt_dir, ""))
		{
			if (!strcmp(mnt_p->mnt_fsname, "/dev/sda1"))
			{
				printf(" sda1 Mount OK \r\n");
			}
			// printf(" mnt_fsname [%s],mnt_dir [%s],mnt_type [%s],mnt_opts [%s], \
            //         mnt_freq [%d],mnt_passno [%d].. \r\n",
			// 	   mnt_p->mnt_fsname, mnt_p->mnt_dir, mnt_p->mnt_type,
			// 	   mnt_p->mnt_opts, mnt_p->mnt_freq, mnt_p->mnt_passno);
		}
	}
	(void)endmntent(mtab);

	return 0;
}