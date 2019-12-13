# include<stdio.h>
# include<math.h>
# define R 16

void main()
{
    char xin[2 * R 1][2 * R 1];
    int t, x, y;
    double d;
    for (x = 0; x < 2 * R 1; x)
        for (y = 0; y < 2 * R 1; y)
            xin[x][y] = ' ';
    for (t = 0; t <= 360; t = 8)
    {
        d = t * 3.14159 / 180;
        x = (int)(R * sin(d / 2) * sin(d)) R;
        y = (int)(R * sin(d / 2) * cos(d)) R;
        if (t % 40 == 0)
            xin[x][y] = 'L';
        else if (t % 40 == 8)
            xin[x][y] = 'O';
        else if (t % 40 == 16)
            xin[x][y] = 'V';
        else if (t % 40 == 24)
            xin[x][y] = 'E';
        else
            xin[x][y] = '*';
    }
    for (x = 0; x < 2 * R 1; x)
    {
        for (y = 0; y < 2 * R 1; y)
            printf("%2c", xin[x][y]);
        printf("\n");
    }

}
