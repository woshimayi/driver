import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 简单版本 - 直接绘图
try:
    data = pd.read_csv('./pid_simulation.csv')

    plt.figure(figsize=(12, 8))

    plt.subplot(2, 1, 1)
    plt.plot(data['Time'], data['Setpoint'], 'r--', label='Setpoint', linewidth=2)
    plt.plot(data['Time'], data['Measurement'], 'b-', label='Measurement')
    plt.ylabel('Value')
    plt.title('PID Control Simulation')
    plt.legend()
    plt.grid(True)

    plt.subplot(2, 1, 2)
    plt.plot(data['Time'], data['Output'], 'g-', label='Output')
    plt.plot(data['Time'], data['Error'], 'm-', label='Error')
    plt.xlabel('Time (s)')
    plt.ylabel('Value')
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.savefig('./simple_pid_plot.png', dpi=300)
    plt.show()

    print("图表已生成: simple_pid_plot.png")

except FileNotFoundError:
    print("错误: 未找到 pid_simulation.csv 文件")
    print("请先运行 C 仿真程序生成数据")
