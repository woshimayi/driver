/*
 * @*************************************:
 * @FilePath     : /user/C/progress_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-08-28 19:31:09
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-28 19:33:28
 * @Descripttion : progress bar test
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct _progress
{
    int cur_size;
    int sum_size;
} progress_t;

void progress_bar(progress_t *progress_data)
{
    int percentage = 0;
    int cnt = 0;
    char proc[102];

    memset(proc, '\0', sizeof(proc));

    percentage = (int)(progress_data->cur_size * 100 / progress_data->sum_size);
    printf("percentage = %d %%\n", percentage);

    if (percentage <= 100)
    {
        while (cnt <= percentage)
        {
            printf("[%-100s] [%d%%]\r", proc, cnt);
            fflush(stdout);
            proc[cnt] = '#';
            usleep(100000);
            cnt++;
        }
    }
    printf("\n");
}

int main(int arc, char *argv[])
{
    progress_t progress_test = {0};

    progress_test.cur_size = 65;
    progress_test.sum_size = 100;
    progress_bar(&progress_test);

    return 0;
}