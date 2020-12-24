/**
 * 计算 IMU 数据
 * 要注意的的是，四元数算法输出的是三个量 Pitch、Roll 和 Yaw，运算量很大。而像平衡小车这样的例子只需要一个角（Pitch 或 Roll ）就可以满足工作要求，个人觉得做平衡小车最好不用四元数法
 */

#include<math.h>
// #include "stm32f10x.h"
//---------------------------------------------------------------------------------------------------


// 变量定义
#define Kp 100.0f                               // 比例增益支配率收敛到加速度计/磁强计
#define Ki 0.002f                               // 积分增益支配率的陀螺仪偏见的衔接
#define halfT 0.001f                            // 采样周期的一半
float q0 = 1, q1 = 0, q2 = 0, q3 = 0;           // 四元数的元素，代表估计方向
float exInt = 0, eyInt = 0, ezInt = 0;          // 按比例缩小积分误差
float Yaw,Pitch,Roll;                           //偏航角，俯仰角，翻滚角


/**
 * [IMUupdate description]
 * @param gx [description]
 * @param gy [description]
 * @param gz [description]
 * @param ax [description]
 * @param ay [description]
 * @param az [description]
 */
void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
        float norm;
        float vx, vy, vz;
        float ex, ey, ez;  
        // 测量正常化
        norm = sqrt(ax*ax + ay*ay + az*az);      
        ax = ax / norm;                   //单位化
        ay = ay / norm;
        az = az / norm;

        // 估计方向的重力
        vx = 2*(q1*q3 - q0*q2);
        vy = 2*(q0*q1 + q2*q3);
        vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

        // 错误的领域和方向传感器测量参考方向之间的交叉乘积的总和
        ex = (ay*vz - az*vy);
        ey = (az*vx - ax*vz);
        ez = (ax*vy - ay*vx);

        // 积分误差比例积分增益
        exInt = exInt + ex*Ki;
        eyInt = eyInt + ey*Ki;
        ezInt = ezInt + ez*Ki;

        // 调整后的陀螺仪测量
        gx = gx + Kp*ex + exInt;
        gy = gy + Kp*ey + eyInt;
        gz = gz + Kp*ez + ezInt;

        // 整合四元数率和正常化
        q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
        q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
        q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
        q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;  

        // 正常化四元
        norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
        q0 = q0 / norm;
        q1 = q1 / norm;
        q2 = q2 / norm;
        q3 = q3 / norm;

        Pitch  = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3; // pitch ,转换为度数
        Roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // rollv
        //Yaw = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;                //此处没有价值，注掉
}




/**
 * MPU6050 可以输出三轴的加速度和角速度。
 * 通过加速度和角速度都可以得到 Pitch 和 Roll 角（加速度不能得到 Yaw 角），
 * 就是说有两组 Pitch、Roll 角，到底应该选哪组呢？
 * 别急，先分析一下。MPU6050 的加速度计和陀螺仪各有优缺点，
 * 三轴的加速度值没有累积误差，且通过算 tan()  可以得到倾角，
 * 但是它包含的噪声太多（因为待测物运动时会产生加速度，电机运行时振动会产生加速度等），不能直接使用；
 * 陀螺仪对外界振动影响小，精度高，通过对角速度积分可以得到倾角，但是会产生累积误差。
 * 所以，不能单独使用 MPU6050 的加速度计或陀螺仪来得到倾角，需要互补。
 * 一阶互补算法的思想就是给加速度和陀螺仪不同的权值，把它们结合到一起，进行修正。得到 Pitch 角的程序如下：
 */

//一阶互补滤波

float K1 =0.1; // 对加速度计取值的权重
float dt=0.001;//注意：dt的取值为滤波器采样时间
float angle;
 
angle_ax=atan(ax/az)*57.3;     //加速度得到的角度
gy=(float)gyo[1]/7510.0;       //陀螺仪得到的角速度
Pitch = yijiehubu(angle_ax,gy);

/**
 * [yijiehubu 采集后计算的角度和角加速度]
 * @param  angle_m [description]
 * @param  gyro_m  [description]
 * @return         [description]
 */
float yijiehubu(float angle_m, float gyro_m)
{
     angle = K1 * angle_m + (1-K1) * (angle + gyro_m * dt);
     return angle;
}
// 互补算法只能得到一个倾角，这在平衡车项目中够用了，而在四轴飞行器设计中还需要 Roll 和 Yaw，就需要两个 互补算法，我是这样写的，注意变量不要搞混：
 
//一阶互补滤波
float K1 =0.1;          // 对加速度计取值的权重
float dt=0.001;         //注意：dt的取值为滤波器采样时间
float angle_P,angle_R;

/**
 * [yijiehubu_P description]
 * @param  angle_m [description]
 * @param  gyro_m  [description]
 * @return         [description]
 */
float yijiehubu_P(float angle_m, float gyro_m)//采集后计算的角度和角加速度
{
     angle_P = K1 * angle_m + (1-K1) * (angle_P + gyro_m * dt);
         return angle_P;
}
 
/**
 * [yijiehubu_R description]
 * @param  angle_m [description]
 * @param  gyro_m  [description]
 * @return         [description]
 */
float yijiehubu_R(float angle_m, float gyro_m)//采集后计算的角度和角加速度
{
     angle_R = K1 * angle_m + (1-K1) * (angle_R + gyro_m * dt);
         return angle_R;
}

// 单靠 MPU6050 无法准确得到 Yaw 角，需要和地磁传感器结合使用。


/**
 * 三、卡尔曼滤波
   其实卡尔曼滤波和一阶互补有些相似，输入也是一样的。卡尔曼原理以及什么5个公式等等的，我也不太懂，就不写了，
   感兴趣的话可以上网查。在此给出具体程序，和一阶互补算法一样，每次卡尔曼滤波只能得到一个方向的角度。
 */
 
#include<math.h>
#include "stm32f10x.h"
#include "Kalman_Filter.h"

//卡尔曼滤波参数与函数
float dt=0.001;//注意：dt的取值为kalman滤波器采样时间
float angle, angle_dot;//角度和角速度
float P[2][2] = {{ 1, 0 },
                 { 0, 1 }};
float Pdot[4] ={ 0,0,0,0};
float Q_angle=0.001, Q_gyro=0.005; //角度数据置信度,角速度数据置信度
float R_angle=0.5 ,C_0 = 1;
float q_bias, angle_err, PCt_0, PCt_1, E, K_0, K_1, t_0, t_1;
 
//卡尔曼滤波
float Kalman_Filter(float angle_m, float gyro_m)//angleAx 和 gyroGy
{
        angle+=(gyro_m-q_bias) * dt;
        angle_err = angle_m - angle;

        Pdot[0]=Q_angle - P[0][1] - P[1][0];
        Pdot[1]=- P[1][1];
        Pdot[2]=- P[1][1];
        Pdot[3]=Q_gyro;

        P[0][0] += Pdot[0] * dt;
        P[0][1] += Pdot[1] * dt;
        P[1][0] += Pdot[2] * dt;
        P[1][1] += Pdot[3] * dt;

        PCt_0 = C_0 * P[0][0];
        PCt_1 = C_0 * P[1][0];
        E = R_angle + C_0 * PCt_0;

        K_0 = PCt_0 / E;
        K_1 = PCt_1 / E;
        t_0 = PCt_0;
        t_1 = C_0 * P[0][1];

        P[0][0] -= K_0 * t_0;
        P[0][1] -= K_0 * t_1;
        P[1][0] -= K_1 * t_0;
        P[1][1] -= K_1 * t_1;

        angle += K_0 * angle_err; //最优角度
        q_bias += K_1 * angle_err;
        angle_dot = gyro_m-q_bias;//最优角速度
 
        return angle；
}