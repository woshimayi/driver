# code=utf-8
# 
# 
#    根据坐标点位置计算方位角（python实现）
#    根据坐标点位置计算方位角是在GPS轨迹处理和数据挖掘中很常见的得到车头朝向的方式。
#    网上的大部分代码都有些许错误，这里总结如下。（x1，y1）为当前GPS点坐标，（x2，y2）为下一个点的GPS坐标：

import math


def calc_angle(x1,y1,x2,y2): 
    angle=0
    dy= y2-y1
    dx= x2-x1
    if dx==0 and dy>0:
        angle = 0
    if dx==0 and dy<0:
        angle = 180
    if dy==0 and dx>0:
        angle = 90
    if dy==0 and dx<0:
        angle = 270
    if dx>0 and dy>0:
       angle = math.atan(dx/dy)*180/math.pi
    elif dx<0 and dy>0:
       angle = 360 + math.atan(dx/dy)*180/math.pi
    elif dx<0 and dy<0:
       angle = 180 + math.atan(dx/dy)*180/math.pi
    elif dx>0 and dy<0:
       angle = 180 + math.atan(dx/dy)*180/math.pi
    return angle



lat1 = 31.24916171
lng1 = 121.48789949

# lat2 = 31.248405
# lng2 = 121.542576

lat2 = 31.235067;
lng2 = 121.531078


print(calc_angle(lat1,lng1,lat2,lng2))


