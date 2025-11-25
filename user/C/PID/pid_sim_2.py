'''
@author: caopeng
@license: (C) Copyright 2013-2049, Node Supply Chain Manager Corporation Limited. 
@contact: woshidamayi@gmail.com
@software: dof
@file: pid_sim_2.py
@time: 2025/10/17 19:07
@desc: 
'''

import numpy as np
import matplotlib.pyplot as plt

plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'KaiTi', 'SongTi SC']  # Windows/安卓常用


class AdaptivePID:
    def __init__(self, Kp=1.0, Ki=0.1, Kd=0.01, setpoint=0, dt=0.1):
        # 基础PID参数
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.setpoint = setpoint
        self.dt = dt

        # 自适应系数
        self.alpha_p = 0.02  # 比例增益自适应系数
        self.alpha_i = 0.0005  # 积分增益自适应系数
        self.alpha_d = 0.001  # 微分增益自适应系数

        # 状态变量
        self.prev_error = 0
        self.integral = 0
        self.output_history = []
        self.error_history = []

    def update(self, measured_value):
        # 计算误差
        error = self.setpoint - measured_value

        # 积分项（抗饱和处理）
        self.integral += error * self.dt
        if self.integral > 100:
            self.integral = 100
        elif self.integral < -100:
            self.integral = -100

        # 微分项
        derivative = (error - self.prev_error) / self.dt

        # 计算控制输出
        output = self.Kp * error + self.Ki * self.integral + self.Kd * derivative

        # 参数自适应调整
        self.Kp += self.alpha_p * error
        self.Ki += self.alpha_i * self.integral
        self.Kd += self.alpha_d * derivative

        # 更新状态
        self.prev_error = error
        self.output_history.append(output)
        self.error_history.append(error)

        return output


# 被控对象模拟（二阶系统）
def simulated_system(control_signal, prev_state, dt=0.1):
    # 状态方程: dy/dt = -0.5*y + 2*u
    new_state = prev_state + dt * (-0.5 * prev_state + 2 * control_signal)
    return new_state


# 初始化控制器（目标温度50℃）
pid = AdaptivePID(setpoint=50, Kp=2.0, Ki=0.5, Kd=0.1)
# 仿真参数
total_time = 10  # 秒
dt = 0.1
steps = int(total_time / dt)
time = np.arange(0, total_time, dt)
# 状态记录
process_values = [30]  # 初始温度30℃
control_signals = []
# 运行仿真
for t in range(steps):
    # 获取控制量
    control = pid.update(process_values[-1])
    control_signals.append(control)

    # 更新系统状态（加入随机扰动）
    if t == 30:  # 第3秒时加入负载扰动
        process_values[-1] += 10
    new_state = simulated_system(control, process_values[-1], dt)
    process_values.append(new_state)

# 可视化结果
plt.figure(figsize=(12, 8))
plt.subplot(2, 1, 1)
plt.plot(time, process_values[:-1], 'b-', label="实际值")
plt.axhline(50, color='r', linestyle='--', label="目标值")
plt.axvline(x=3, color='gray', linestyle=':', label="负载扰动")
plt.ylabel("温度 (℃)")
plt.title("自适应PID控制响应")
plt.legend()
plt.subplot(2, 1, 2)
plt.plot(time, control_signals, 'g-')
plt.ylabel("控制信号")
plt.xlabel("时间 (秒)")
plt.tight_layout()
plt.show()
# 输出性能指标
settling_time = np.argmax(np.abs(np.array(pid.error_history)) < 0.5) * dt
print(f"调节时间: {settling_time:.2f}秒 | 超调量: {max(process_values) - 50:.1f}℃")
