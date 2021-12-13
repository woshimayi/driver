/*
* 根据经纬度算方位角
*
*
*
*
*
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415926

#define rad(x) (x * PI / 180.0)

/**
 * [gps2d description]
 * @param  lata [a 点 经度]
 * @param  lona [a 点 纬度]
 * @param  latb [b 点 经度]
 * @param  lonb [b 点 纬度]
 * @return      [description]
 */
double gps2d(double lata, double lona, double latb, double lonb)
{
	double d  = 0;
	double lta = lata * PI / 180; // 算出弧度
	double lga = lona * PI / 180;
	double ltb = latb * PI / 180;
	double lgb = lonb * PI / 180;

	d = sin(lta) * sin(ltb) + cos(lta) * cos(ltb) * cos(lgb - lga);
	d = sqrt(1 - d * d);
	d = cos(ltb) * sin(lgb - lga) / d;
	d = asin(d) * 180.0 / PI;

	printf("1 d = %lf\n", d);
	return d;
}


/**
 * [gps12d description]
 * @param  lata [a 点 经度]
 * @param  lona [a 点 纬度]
 * @param  latb [b 点 经度]
 * @param  lonb [b 点 纬度]
 * @return      [description]
 */
double gps12d(double lata, double lona, double latb, double lonb)
{
	double d  = 0;
	double radlta = rad(lata);
	double radlna = rad(lona);
	double radltb = rad(latb);
	double radlnb = rad(lonb);

	double dlon = radlnb - radlna;
	double y = sin(dlon) * cos(radlta);
	double x = cos(radlta) * sin(radltb) - sin(radlta) * cos(radltb) * cos(dlon);
	//    d = atan2(y,x) * 180.0 / PI;
	if (y > 0)
	{
		if (x > 0)
			d = atan2(y, x);
		else if (x == 0)
			d = 90;
		else
			d = 180 - atan2(-y, x);
	}
	else if (y == 0)
	{
		if (x > 0)
			d = 0;
		else if (x == 0)
			d = 0;
		else
			d = 180;
	}
	else
	{
		if (x > 0)
			d = -atan2(-y, x);
		else if (x == 0)
			d = 270;
		else
			d = atan2(y, x) - 180;
	}

	d = d * 180.0 / PI;
	//    d = (int)(d + 360)%360;

	printf("2 d = %lf\n", d);
	return d;
}


int main(int argc, char **argv)
{
	double d  = 0;
	d = gps2d(29538171, 1066030353, 295381761, 1066039656);
	d = gps12d(29538171, 1066030353, 295381761, 1066039656);
	d = atan2((29.538176 - 29.538176), (106.6030353 - 106.6039656)) / PI * 180.0;
	printf("3 d = %lf\n", d);

	return 0;
}

