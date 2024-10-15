/*
 * @FilePath: \algorithm\posture_c.c
 * @version: 
 * @Author: dof
 * @Date: 2020-12-24 20:05:18
 * @LastEditors: dof
 * @LastEditTime: 2021-02-26 10:07:40
 * @Descripttion: 
 */
/**
 * 计算两个GPS 坐标点的距离 和方位角
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <math.h>


#define M_PI 3.14159265358979323846
#define EARTH_RADIUS 6371.0

// DistanceBetweenLonAndLat();
// ~DistanceBetweenLonAndLat();


// double HaverSin(double thera);
// double ConvertDegreesToRadians(double degrees);
// double ConvertRadiansToDegrees(double radian);
// double get_distance(double lon1, double lat1, double lon2, double lat2);
// int get_angle(double lon1, double lat1, double lon2, double lat2);
// int AngleSpecification(int angle);



// DistanceBetweenLonAndLat()
// {
// }
 
 
// ~DistanceBetweenLonAndLat()
// {
// }
 
double HaverSin(double thera)
{
	double v = sin(thera / 2);
	return v * v;
}
 
//角度转换为弧度
double ConvertDegreesToRadians(double degrees)
{
	return degrees * M_PI / 180;
}
 
//弧度转换为角度
double ConvertRadiansToDegrees(double radian)
{
	return radian * 180.0 / M_PI;
}
 
//计算两个经纬度坐标间的距离(单位：米)
//参数：lon1为经度1；lat1为纬度1；lon2为经度2；lat2为纬度2
double get_distance(double lon1, double lat1, double lon2, double lat2)
{
	lat1 = ConvertDegreesToRadians(lat1);
	lon1 = ConvertDegreesToRadians(lon1);
	lat2 = ConvertDegreesToRadians(lat2);
	lon2 = ConvertDegreesToRadians(lon2);
 
	double vLon = fabs(lon2 - lon1);
	double vLat = fabs(lat2 - lat1);
	double h = HaverSin(vLat) + cos(lat1) * cos(lat2) * HaverSin(vLon);
	double distance = 2 * EARTH_RADIUS * asin(sqrt(h));
 
	return distance * 1000;
}
 
//两个经纬度间连线与正北方向的夹角
//参数：lon1为经度1；lat1为纬度1；lon2为经度2；lat2为纬度
double get_angle(double lon1, double lat1, double lon2, double lat2)
{
	double x = lat2 - lat1;
	double y = lon2 - lon1;
	double angle = -1;
	if (y == 0 && x > 0)
		angle = 0;
	if (y == 0 && x < 0)
		angle = 180;
	if (x == 0 && y > 0)
		angle = 90;
	if (x == 0 && y < 0)
		angle = 270;
	if (angle == -1)
	{
		double dislon = get_distance(lon1, lat2, lon2, lat2);
		double dislat = get_distance(lon2, lat1, lon2, lat2);
		if (x > 0 && y > 0)
			angle = atan2(dislon, dislat) / M_PI * 180;
		if (x < 0 && y > 0)
			angle = atan2(dislat, dislon) / M_PI * 180 + 90;
		if (x < 0 && y < 0)
			angle = atan2(dislon, dislat) / M_PI * 180 + 180;
		if (x > 0 && y < 0)
			angle = atan2(dislat, dislon) / M_PI * 180 + 270;
	}
	return angle;
}
 
//将角度限制在[0,360),0--北，90--东，180--南，270--西；与指南针一致
// double AngleSpecification(double angle)
// {
// 	double curAngle;
// 	if (angle < 0)
// 	{
// 		curAngle = (angle*100000 + 36000000) % 36000000;
// 	}
// 	else {
// 		curAngle = angle*100000 % 36000000;
// 	}
// 	return curAngle;
// }


int main(int argc, char const *argv[])
{
	double lat1 = 31.24916171;
    double lng1 = 121.48789949;

    // double lat2 = 31.248405;
    // double lng2 = 121.542576;


    double lat2 = 31.235067;
    double lng2 = 121.531078;


    double distance = get_distance(lng1, lat1, lng2, lat2);
    printf("距离 distance = %lf\n", distance);

    double angle = get_angle(lng1, lat1, lng2, lat2);
    printf("方位角  angle = %lf\n", angle);


	return 0;
}