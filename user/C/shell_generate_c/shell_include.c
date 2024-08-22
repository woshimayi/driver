/*
 * @*************************************:
 * @FilePath     : /user/C/shell_generate_c/shell_include.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-22 11:07:51
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-22 17:35:01
 * @Descripttion : shell generate c header file
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>



int shell_to_c(char * shell)
{
    if (NULL == shell)
    {
        return 0;
    }

    creat("shell_include.h", 0777);
    FILE *fw = fopen("shell_include.h", "w");

    FILE *fr = fopen(shell, "r");
    
    char cmd[1024] = {0};
    fprintf(fw, "#ifndef __SHELL_INCLUDE__\n");
    fprintf(fw, "#define __SHELL_INCLUDE__\n");
    fprintf(fw, "void generate_file()\n");
    fprintf(fw, "{\n");
    fprintf(fw, "creat(\"/tmp/shell_include.sh\", 0777);\n");
    fprintf(fw, "FILE *fp = fopen(\"/tmp/shell_include.sh\", \"w\");\n");
    fprintf(fw, "if (!fp) {\n");
    fprintf(fw, "	perror(\"fopen\");\n");
    fprintf(fw, "	exit(EXIT_FAILURE);\n");
    fprintf(fw, "}\n");

    while (fgets(cmd, sizeof(cmd), fr))
    {
        cmd[strlen(cmd)-1] = '\0';
        char output[1024] = {0};
        int j = 0;
        for (int i = 0; cmd[i] != '\0' ; i++)
        {
            if (cmd[i] == '"')
            {
                output[j++] = '\\';
                output[j++] = '"';
            }
            else if (cmd[i] == '\\')
            {
                output[j++] = '\\';
                output[j++] = '\\';
            }
            else
            {
                output[j++] = cmd[i];
            }
        }
        // printf("output = %s\n", output);
        if (strlen(output))
        {
            fprintf(fw, "fprintf(fp, \"%s\");\n", output);
            fprintf(fw, "fprintf(fp, \"\\n\");\n");
        }
        else
        {
            fprintf(fw, "fprintf(fp, \"\\t\\n\");\n");
        }
        memset(cmd, 0, sizeof(cmd));
        memset(output, 0, sizeof(output));
    }
    // fprintf(fw, "fprintf(fp, \"\\n\");\n");
    fprintf(fw, "fclose(fp);\n");
    fprintf(fw, "}\n");

    fprintf(fw, "#endif\n");

    fclose(fw);
    fclose(fr);

    return 0;
}

int main(int argc, char const *argv[])
{
    shell_to_c("cppcheck_self.sh");
    printf("zzzzzzzzzzzzzzzzzzzzzzz\n");
    return 0;
}
