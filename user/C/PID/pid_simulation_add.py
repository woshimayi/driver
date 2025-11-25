import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


def calculate_rise_time(data):
    """计算上升时间（达到设定值90%的时间）"""
    target = data['Setpoint'].iloc[0]
    threshold = 0.9 * target

    for i, row in data.iterrows():
        if row['Measurement'] >= threshold:
            return row['Time']
    return None


def analyze_incremental_pid():
    """分析增量式PID性能"""

    # 读取数据
    try:
        data = pd.read_csv('incremental_pid_simulation.csv')
    except FileNotFoundError:
        print("Error: incremental_pid_simulation.csv not found!")
        print("Generating sample data for demonstration...")
        data = generate_sample_data()

    # 创建专业图表
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))

    # 1. 系统响应曲线
    ax1.plot(data['Time'], data['Setpoint'], 'r--', label='Setpoint', linewidth=2, alpha=0.8)
    ax1.plot(data['Time'], data['Measurement'], 'b-', label='System Response', linewidth=1.5)
    ax1.set_ylabel('Amplitude')
    ax1.set_title('System Response')
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # 2. 控制器输出
    ax2.plot(data['Time'], data['Output'], 'g-', label='Controller Output', linewidth=1.5)
    ax2.set_ylabel('Control Signal')
    ax2.set_xlabel('Time (s)')
    ax2.set_title('Controller Output')
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    # 3. 输出增量
    ax3.plot(data['Time'], data['DeltaOutput'], 'c-', label='Output Increment', linewidth=1.5)
    ax3.axhline(y=0, color='r', linestyle='--', alpha=0.5)
    ax3.set_ylabel('Increment')
    ax3.set_xlabel('Time (s)')
    ax3.set_title('Output Increment (Δu)')
    ax3.legend()
    ax3.grid(True, alpha=0.3)

    # 4. 控制误差
    ax4.plot(data['Time'], data['Error'], 'm-', label='Control Error', linewidth=1.5)
    ax4.axhline(y=0, color='r', linestyle='--', alpha=0.5)
    ax4.set_ylabel('Error')
    ax4.set_xlabel('Time (s)')
    ax4.set_title('Control Error')
    ax4.legend()
    ax4.grid(True, alpha=0.3)

    plt.suptitle('Incremental PID Control Analysis', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.savefig('incremental_pid_analysis.png', dpi=300, bbox_inches='tight')
    plt.show()

    # 性能指标计算
    print("\n" + "=" * 50)
    print("INCREMENTAL PID PERFORMANCE ANALYSIS")
    print("=" * 50)

    steady_state_error = data['Error'].iloc[-100:].mean()
    max_overshoot = max(0, (data['Measurement'].max() - data['Setpoint'].iloc[0]) / data['Setpoint'].iloc[0] * 100)
    settling_time = calculate_settling_time(data)
    rise_time = calculate_rise_time(data)

    print(f"Steady-state error: {steady_state_error:.4f}")
    print(f"Maximum overshoot: {max_overshoot:.2f}%")
    print(f"Rise time: {rise_time:.2f} s" if rise_time else "Rise time: Not reached")
    print(f"Settling time: {settling_time:.2f} s" if settling_time else "Settling time: Not reached")
    print(f"Maximum output increment: {data['DeltaOutput'].abs().max():.4f}")

    # 计算性能指标
    ise = np.trapz(data['Error'] ** 2, data['Time'])
    iae = np.trapz(np.abs(data['Error']), data['Time'])
    print(f"Integral Square Error (ISE): {ise:.4f}")
    print(f"Integral Absolute Error (IAE): {iae:.4f}")


def calculate_settling_time(data, tolerance=0.02):
    """计算调节时间（进入±2%误差带）"""
    target = data['Setpoint'].iloc[0]
    tolerance_band = tolerance * target

    for i, row in data.iterrows():
        if abs(row['Error']) <= tolerance_band:
            # 检查是否持续在误差带内
            subsequent_data = data.iloc[i:i + 100]  # 检查后续100个点
            if all(abs(subsequent_data['Error']) <= tolerance_band):
                return row['Time']
    return None


def generate_sample_data():
    """生成示例数据（如果数据文件不存在）"""
    print("Generating sample incremental PID data...")

    time = np.arange(0, 30, 0.01)
    setpoint = np.ones_like(time) * 10.0
    setpoint[time > 15] = 5.0

    # 模拟增量式PID响应
    measurement = np.zeros_like(time)
    output = np.zeros_like(time)
    error = np.zeros_like(time)
    delta_output = np.zeros_like(time)

    # 模拟参数
    Kp, Ki, Kd = 2.5, 0.8, 0.1
    prev_error = 0.0
    prev_error2 = 0.0
    system_state = 0.0

    for i, t in enumerate(time):
        error[i] = setpoint[i] - system_state

        # 增量式PID计算
        proportional = Kp * (error[i] - prev_error)
        integral = Ki * error[i] * 0.01
        derivative = Kd * (error[i] - 2 * prev_error + prev_error2) / 0.01 if i > 1 else 0

        delta_output[i] = proportional + integral + derivative
        output[i] = output[i - 1] + delta_output[i] if i > 0 else delta_output[i]

        # 输出限幅
        output[i] = np.clip(output[i], -20, 20)

        # 系统更新
        system_state = system_state + 0.01 * (1.2 * output[i] - system_state) / 2.0
        measurement[i] = system_state + np.random.normal(0, 0.05)

        prev_error2 = prev_error
        prev_error = error[i]

    data = pd.DataFrame({
        'Time': time,
        'Setpoint': setpoint,
        'Measurement': measurement,
        'Output': output,
        'Error': error,
        'DeltaOutput': delta_output
    })

    data.to_csv('incremental_pid_simulation.csv', index=False)
    print("Sample data generated: incremental_pid_simulation.csv")
    return data


if __name__ == "__main__":
    analyze_incremental_pid()
