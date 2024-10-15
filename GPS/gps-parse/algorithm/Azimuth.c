/*
 * @FilePath: \algorithm\Azimuth.c
 * @version: 
 * @Author: dof
 * @Date: 2020-12-24 20:04:11
 * @LastEditors: dof
 * @LastEditTime: 2021-02-26 10:04:19
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


/**
 * [getDistanceByGps 距离]
 * @param  lat1 [纬度]
 * @param  lng1 [经度]
 * @param  lat2 [纬度]
 * @param  lng2 [经度]
 * @return      [距离，单位/m(米)]
 */
double getDistanceByGps(double lat1, double lng1, double lat2, double lng2)
{
    double PI = 3.14159265358979323846;
    double EarthRadius = 6378137;
    double Rad = PI / 180.0;

    double radlat1 = lat1 * Rad;
    double radlat2 = lat2 * Rad;
    double a = radlat2 - radlat1;
    double b = (lng2 - lng1) * Rad;
    double s = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radlat1) * cos(radlat2) * pow(sin(b / 2), 2)));
    s = s * EarthRadius;
    s = round(s * 10000) / 10000;
    return s;
}



/**
 * [getAngleByGps 计算方位角]
 * @param  {[type]} double lat1          [纬度]
 * @param  {[type]} double lng1          [经度]
 * @param  {[type]} double lat2          [纬度]
 * @param  {[type]} double lng2          [经度]
 * @return {[type]}        				[返回 角度]
 */
double getAngleByGps(double lat1, double lng1, double lat2, double lng2)
{
    double PI = 3.14159265358979323846;
    double x = sin(lng2 - lng1) * cos(lat2);
    double y = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lng2 - lng1);
    double angle = atan2(x, y) * 180 / PI;
    return angle > 0 ? angle : angle + 360;
}




int main(int argc, char const *argv[])
{
    double lat1 = 31.24916171;
    double lng1 = 121.48789949;

    // double lat2 = 31.248405;
    // double lng2 = 121.542576;


    double lat2 = 31.235067;
    double lng2 = 121.531078;

    double distance = getDistanceByGps(lat1, lng1, lat2, lng2);
    printf("距离 distance = %lf\n", distance);

    int angle = getAngleByGps(lat1, lng1, lat2, lng2);
    printf("方位角  angle = %d\n", angle);

    return 0;
}