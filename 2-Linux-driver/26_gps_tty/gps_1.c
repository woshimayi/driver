/*
 * @*************************************: 
 * @FilePath: /driver/2-Linux-driver/26_gps_tty/gps_1.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-24 22:10:44
 * @LastEditors: dof
 * @LastEditTime: 2022-07-24 22:16:50
 * @Descripttion: 命令输入参数
 * @**************************************: 
 */






int main(int argc, char const *argv[])
{
	
	while((c = getopt(argc, argv, "h:t::?")) != -1)
    {
        switch(c)
        {
            case 't':
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

	return 0;
}

