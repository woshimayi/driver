/*
 * @*************************************: 
 * @FilePath: /user/C/string/RGB_test.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-28 13:50:59
 * @LastEditors: dof
 * @LastEditTime: 2022-09-28 14:03:04
 * @Descripttion:  rgb 颜色控制
 * @**************************************: 
 */

void rgb(int rgb)
{
	int r = 0, g = 0, b = 0;
	r = (rgb & 0xff0000) >> 16;
	b = (rgb & 0x00ff00) >> 8;
	g = (rgb & 0x0000ff);
	printf("rgb = 0x%6x %3d %3d %3d\n", rgb, r, g, b);
}

int main(int argc, char const *argv[])
{
	for (int i = 0; i < 0xff00ff; i++)
	{
		rgb(i);
	}
	return 0;
}

