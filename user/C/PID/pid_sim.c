#if 0

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

// PID 控制器结构体
typedef struct
{
    double kp;         // 比例系数
    double ki;         // 积分系数
    double kd;         // 微分系数
    double setpoint;   // 设定值
    double integral;   // 积分项
    double prev_error; // 上一次误差
    double output;     // 输出值
    double output_min; // 输出最小值
    double output_max; // 输出最大值
} PIDController;

// 被控对象模型（一阶惯性系统）
typedef struct
{
    double gain;          // 系统增益
    double time_constant; // 时间常数
    double state;         // 系统状态
} PlantModel;

// 初始化 PID 控制器
void pid_init(PIDController *pid, double kp, double ki, double kd,
              double setpoint, double min, double max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = setpoint;
    pid->integral = 0.0;
    pid->prev_error = 0.0;
    pid->output = 0.0;
    pid->output_min = min;
    pid->output_max = max;
}

// PID 计算函数
double pid_compute(PIDController *pid, double measurement, double dt)
{
    double error = pid->setpoint - measurement;

    // 比例项
    double proportional = pid->kp * error;

    // 积分项（带抗饱和）
    // pid->integral += error * dt;
    double integral = 0;
    // double integral = pid->ki * pid->integral;

    // 微分项（使用后向差分）
    double derivative = 0.0;
    // if (dt > 0)
    // {
    //     derivative = pid->kd * (error - pid->prev_error) / dt;
    // }

    // 计算输出
    pid->output = proportional + integral + derivative;

    // 输出限幅
    if (pid->output > pid->output_max)
    {
        pid->output = pid->output_max;
        // 抗饱和：停止积分累积
        pid->integral -= error * dt;
    }
    else if (pid->output < pid->output_min)
    {
        pid->output = pid->output_min;
        // 抗饱和：停止积分累积
        pid->integral -= error * dt;
    }

    pid->prev_error = error;
    return pid->output;
}

// 初始化被控对象
void plant_init(PlantModel *plant, double gain, double time_constant)
{
    plant->gain = gain;
    plant->time_constant = time_constant;
    plant->state = 0.0;
}

// 被控对象模型更新（使用欧拉法离散化）
double plant_update(PlantModel *plant, double input, double dt)
{
    // 连续系统：dy/dt = (K*u - y) / T
    // 离散化：y[k+1] = y[k] + dt * (K*u - y[k]) / T
    plant->state = plant->state + dt * (plant->gain * input - plant->state) / plant->time_constant;
    return plant->state;
}

// 添加噪声（模拟传感器噪声）
double add_noise(double signal, double noise_level)
{
    return signal + noise_level * ((double)rand() / RAND_MAX - 0.5) * 2.0;
}

// 记录数据到文件
void log_data(FILE *file, double time, double setpoint,
              double measurement, double output, double error)
{
    fprintf(file, "%.3f,%.3f,%.3f,%.3f,%.3f\n",
            time, setpoint, measurement, output, error);
}

int main()
{
    // 初始化随机数种子
    srand(time(NULL));

    // 创建 PID 控制器
    PIDController pid;
    pid_init(&pid, 10.0, 0.8, 0.1, 10.0, -20.0, 20.0); // 目标值 10.0

    // 创建被控对象
    PlantModel plant;
    plant_init(&plant, 1.2, 2.0); // 增益 1.2，时间常数 2.0

    // 仿真参数
    double dt = 0.01;              // 采样时间 10ms
    double simulation_time = 30.0; // 总仿真时间 30秒
    int steps = (int)(simulation_time / dt);

    // 创建数据记录文件
    FILE *log_file = fopen("pid_simulation.csv", "w");
    if (!log_file)
    {
        perror("无法创建日志文件");
        return 1;
    }
    fprintf(log_file, "Time,Setpoint,Measurement,Output,Error\n");

    printf("开始 PID 仿真...\n");
    printf("目标值: %.1f, 仿真时间: %.1f秒, 步长: %.3f秒\n",
           pid.setpoint, simulation_time, dt);

    // 主仿真循环
    for (int i = 0; i < steps; i++)
    {
        double current_time = i * dt;

        // 在特定时间改变设定值，测试动态响应
        // if (current_time > 15.0)
        // {
        //     pid.setpoint = 5.0;
        // }

        // 读取测量值（添加噪声）
        double measurement = add_noise(plant.state, 0.05);

        // PID 计算
        double output = pid_compute(&pid, measurement, dt);

        // 更新被控对象
        plant_update(&plant, output, dt);

        // 记录数据
        log_data(log_file, current_time, pid.setpoint,
                 measurement, output, pid.prev_error);

        // 每秒钟打印一次状态
        if (i % (int)(1.0 / dt) == 0)
        {
            printf("时间: %5.1fs, 目标: %5.1f, 测量: %5.3f, 输出: %5.3f, 误差: %5.3f\n",
                   current_time, pid.setpoint, measurement, output, pid.prev_error);
        }

        // 模拟实时延迟
        usleep(dt * 1000);
    }

    fclose(log_file);
    printf("仿真完成！数据已保存到 pid_simulation.csv\n");

    return 0;
}
#else

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

// 增量式 PID 控制器结构体
typedef struct
{
    double kp;          // 比例系数
    double ki;          // 积分系数
    double kd;          // 微分系数
    double setpoint;    // 设定值
    double prev_error;  // 上一次误差 e(k-1)
    double prev_error2; // 上上次误差 e(k-2)
    double output;      // 当前输出值
    double output_min;  // 输出最小值
    double output_max;  // 输出最大值
} IncrementalPID;

// 被控对象模型（一阶惯性系统）
typedef struct
{
    double gain;          // 系统增益
    double time_constant; // 时间常数
    double state;         // 系统状态
} PlantModel;

// 初始化增量式 PID 控制器
void pid_init(IncrementalPID *pid, double kp, double ki, double kd,
              double setpoint, double min, double max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = setpoint;
    pid->prev_error = 0.0;
    pid->prev_error2 = 0.0;
    pid->output = 0.0;
    pid->output_min = min;
    pid->output_max = max;
}

// 增量式 PID 计算函数
/**
 * @brief 
 * 
 * @param pid PID 控制器结构体
 * @param measurement 当前测量值
 * @param dt  采样时间
 * @return double 当前输出值
 */
double pid_compute(IncrementalPID *pid, double measurement, double dt)
{
    double error = pid->setpoint - measurement; // 当前误差 e(k)

    // 增量式 PID 公式:
    // Δu(k) = Kp * [e(k) - e(k-1)] + Ki * e(k) + Kd * [e(k) - 2e(k-1) + e(k-2)]
    // u(k) = u(k-1) + Δu(k)

    double delta_output = 0.0;

    // 比例项增量
    double proportional = pid->kp * (error - pid->prev_error);

    // 积分项增量 (使用当前误差)
    double integral = pid->ki * error * dt; // 注意这里要乘以dt

    // 微分项增量
    double derivative = 0.0;
    if (dt > 0)
    {
        derivative = pid->kd * (error - 2 * pid->prev_error + pid->prev_error2) / dt;
    }

    // 计算输出增量
    delta_output = proportional + integral + derivative;

    // 更新输出
    pid->output += delta_output;

    // 输出限幅
    if (pid->output > pid->output_max)
    {
        pid->output = pid->output_max;
    }
    else if (pid->output < pid->output_min)
    {
        pid->output = pid->output_min;
    }

    // 更新误差历史
    pid->prev_error2 = pid->prev_error;
    pid->prev_error = error;

    return pid->output;
}

// 带积分分离的增量式 PID（改进版本）
double pid_compute_with_integral_separation(IncrementalPID *pid, double measurement, double dt, double integral_threshold)
{
    double error = pid->setpoint - measurement;

    double delta_output = 0.0;

    // 比例项增量
    double proportional = pid->kp * (error - pid->prev_error);

    // 积分项增量（积分分离）
    double integral = 0.0;
    if (fabs(error) < integral_threshold)
    {
        // 当误差较小时才进行积分，避免超调
        integral = pid->ki * error * dt;
    }

    // 微分项增量
    double derivative = 0.0;
    if (dt > 0)
    {
        derivative = pid->kd * (error - 2 * pid->prev_error + pid->prev_error2) / dt;
    }

    // 计算输出增量
    delta_output = proportional + integral + derivative;

    // 更新输出
    pid->output += delta_output;

    // 输出限幅
    if (pid->output > pid->output_max)
    {
        pid->output = pid->output_max;
    }
    else if (pid->output < pid->output_min)
    {
        pid->output = pid->output_min;
    }

    // 更新误差历史
    pid->prev_error2 = pid->prev_error;
    pid->prev_error = error;

    return pid->output;
}

// 初始化被控对象
void plant_init(PlantModel *plant, double gain, double time_constant)
{
    plant->gain = gain;
    plant->time_constant = time_constant;
    plant->state = 0.0;
}

// 被控对象模型更新（使用欧拉法离散化）
double plant_update(PlantModel *plant, double input, double dt)
{
    // 连续系统：dy/dt = (K*u - y) / T
    // 离散化：y[k+1] = y[k] + dt * (K*u - y[k]) / T
    plant->state = plant->state + dt * (plant->gain * input - plant->state) / plant->time_constant;
    return plant->state;
}

// 添加噪声（模拟传感器噪声）
double add_noise(double signal, double noise_level)
{
    return signal + noise_level * ((double)rand() / RAND_MAX - 0.5) * 2.0;
}

// 记录数据到文件
void log_data(FILE *file, double time, double setpoint,
              double measurement, double output, double error, double delta_output)
{
    fprintf(file, "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
            time, setpoint, measurement, output, error, delta_output);
}

int main()
{
    // 初始化随机数种子
    srand(time(NULL));

    // 创建增量式 PID 控制器
    IncrementalPID pid;
    pid_init(&pid, 1.0, 0.8, 0.1, 10.0, -20.0, 20.0); // 目标值 10.0

    // 创建被控对象
    PlantModel plant;
    plant_init(&plant, 1.2, 2.0); // 增益 1.2，时间常数 2.0

    // 仿真参数
    double dt = 0.01;              // 采样时间 10ms
    double simulation_time = 30.0; // 总仿真时间 30秒
    int steps = (int)(simulation_time / dt);

    // 创建数据记录文件
    FILE *log_file = fopen("incremental_pid_simulation.csv", "w");
    if (!log_file)
    {
        perror("无法创建日志文件");
        return 1;
    }
    fprintf(log_file, "Time,Setpoint,Measurement,Output,Error,DeltaOutput\n");

    printf("开始增量式 PID 仿真...\n");
    printf("目标值: %.1f, 仿真时间: %.1f秒, 步长: %.3f秒\n",
           pid.setpoint, simulation_time, dt);

    double prev_output = 0.0;

    // 主仿真循环
    for (int i = 0; i < steps; i++)
    {
        double current_time = i * dt;

        // 在特定时间改变设定值，测试动态响应
        // if (current_time > 15.0)
        // {
        //     pid.setpoint = 5.0;
        // }

        // 读取测量值（添加噪声）
        double measurement = add_noise(plant.state, 0.05);

        // 增量式 PID 计算
        double output = pid_compute(&pid, measurement, dt);
        double delta_output = output - prev_output;
        prev_output = output;

        // 更新被控对象
        plant_update(&plant, output, dt);

        // 记录数据（包含输出增量）
        log_data(log_file, current_time, pid.setpoint,
                 measurement, output, pid.prev_error, delta_output);

        // 每秒钟打印一次状态
        // if (i % (int)(1.0 / dt) == 0)
        {
            printf("时间: %.1fs, 目标: %.1f, 测量: %.3f, 输出: %.3f, 误差: %.3f, 输出增量: %.3f\n",
                   current_time, pid.setpoint, measurement, output,
                   pid.prev_error, delta_output);
        }

        // 模拟实时延迟
        usleep(dt * 1000000);
    }

    fclose(log_file);
    printf("仿真完成！数据已保存到 incremental_pid_simulation.csv\n");

    return 0;
}

#endif