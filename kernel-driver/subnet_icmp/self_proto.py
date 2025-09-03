'''
@author: caopeng
@license: (C) Copyright 2013-2049, Node Supply Chain Manager Corporation Limited. 
@contact: woshidamayi@gmail.com
@software: dof
@file: self_proto.py
@time: 2025/08/24 15:17
@desc: 
'''

from scapy.all import *
from scapy.packet import Packet, bind_layers
from scapy.fields import ByteField, ShortField, IntField, StrLenField, XShortField
import struct

# !/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
自定义协议测试程序 - 使用Scapy发送0x8888协议数据包
"""

from scapy.all import *
import time
import sys


def create_custom_protocol_packet(src_ip, dst_ip, msg_type, seq_num=1, data_len=0):
    """
    创建自定义协议数据包

    Args:
        src_ip: 源IP地址
        dst_ip: 目的IP地址
        msg_type: 消息类型 (0x0001=心跳, 0x0002=数据, 0x0003=控制)
        seq_num: 序列号
        data_len: 数据长度
    """

    # 自定义协议头部 (20字节)
    class CustomProtocol(Packet):
        name = "CustomProtocol"
        fields_desc = [
            IPField("src_addr", src_ip),  # 源地址 (4字节)
            IPField("dst_addr", dst_ip),  # 目的地址 (4字节)
            ShortField("seq_num", seq_num),  # 序列号 (2字节)
            ShortField("msg_type", msg_type),  # 消息类型 (2字节)
            IntField("data_len", data_len),  # 数据长度 (4字节)
        ]

    # 注册自定义协议
    bind_layers(Ether, CustomProtocol, type=0x8888)

    # 构建数据包
    # 以太网层
    eth = Ether(
        dst="00:11:22:33:44:55",  # 目标MAC地址 (请替换为实际地址)
        src="f4:6b:8c:5e:bb:1e",  # 源MAC地址 (请替换为实际地址)
        type=0x8888  # 自定义协议类型
    )

    # 自定义协议层
    custom = CustomProtocol(
        src_addr=src_ip,
        dst_addr=dst_ip,
        seq_num=seq_num,
        msg_type=msg_type,
        data_len=data_len
    )

    # 组装数据包
    packet = eth / custom

    return packet


def send_test_packets(interface="eth0", target_mac="00:11:22:33:44:55"):
    """
    发送测试数据包

    Args:
        interface: 网络接口名称
        target_mac: 目标MAC地址
    """

    print(f"开始发送自定义协议测试数据包...")
    print(f"接口: {interface}")
    print(f"目标MAC: {target_mac}")
    print("-" * 50)

    # 测试1: 心跳包
    print("发送心跳包 (消息类型: 0x0001)")
    heartbeat_pkt = create_custom_protocol_packet(
        src_ip="172.16.36.35",
        dst_ip="172.16.26.224",
        msg_type=0x0001,
        seq_num=1
    )

    # 更新目标MAC地址
    heartbeat_pkt[Ether].dst = target_mac

    try:
        sendp(heartbeat_pkt, iface=interface, verbose=False)
        print("✓ 心跳包发送成功")
    except Exception as e:
        print(f"✗ 心跳包发送失败: {e}")

    time.sleep(1)

    # 测试2: 数据包
    print("发送数据包 (消息类型: 0x0002)")
    data_pkt = create_custom_protocol_packet(
        src_ip="172.16.36.35",
        dst_ip="172.16.26.224",
        msg_type=0x0002,
        seq_num=2,
        data_len=64
    )

    data_pkt[Ether].dst = target_mac

    try:
        sendp(data_pkt, iface=interface, verbose=False)
        print("✓ 数据包发送成功")
    except Exception as e:
        print(f"✗ 数据包发送失败: {e}")

    time.sleep(1)

    # 测试3: 控制包
    print("发送控制包 (消息类型: 0x0003)")
    control_pkt = create_custom_protocol_packet(
        src_ip="172.16.36.35",
        dst_ip="172.16.26.224",
        msg_type=0x0003,
        seq_num=3,
        data_len=32
    )

    control_pkt[Ether].dst = target_mac

    try:
        sendp(control_pkt, iface=interface, verbose=False)
        print("✓ 控制包发送成功")
    except Exception as e:
        print(f"✗ 控制包发送失败: {e}")

    print("-" * 50)
    print("所有测试数据包发送完成!")


def send_single_packet(interface="eth0", target_mac="00:11:22:33:44:55",
                       msg_type=0x0001, seq_num=1, src_ip="172.16.36.35",
                       dst_ip="172.16.26.224"):
    """
    发送单个数据包

    Args:
        interface: 网络接口名称
        target_mac: 目标MAC地址
        msg_type: 消息类型
        seq_num: 序列号
        src_ip: 源IP地址
        dst_ip: 目的IP地址
    """

    print(f"发送单个数据包...")
    print(f"接口: {interface}")
    print(f"目标MAC: {target_mac}")
    print(f"消息类型: 0x{msg_type:04x}")
    print(f"序列号: {seq_num}")
    print(f"源IP: {src_ip}")
    print(f"目的IP: {dst_ip}")

    packet = create_custom_protocol_packet(
        src_ip=src_ip,
        dst_ip=dst_ip,
        msg_type=msg_type,
        seq_num=seq_num
    )

    packet[Ether].dst = target_mac

    try:
        sendp(packet, iface=interface, verbose=False)
        print("✓ 数据包发送成功")
    except Exception as e:
        print(f"✗ 数据包发送失败: {e}")


def main():
    """主函数"""

    print("自定义协议测试程序 (Scapy)")
    print("=" * 50)

    # 获取命令行参数
    if len(sys.argv) < 2:
        print("使用方法:")
        print("  python3 test_custom_protocol.py <interface> [target_mac]")
        print("  例如: python3 test_custom_protocol.py eth0 00:11:22:33:44:55")
        print()
        print("或者使用默认参数:")
        print("  python3 test_custom_protocol.py")
        print()

        # 使用默认参数
        interface = "lan"
        target_mac = "A4:A5:28:32:08:04"

        print(f"使用默认参数:")
        print(f"  接口: {interface}")
        print(f"  目标MAC: {target_mac}")
        print()

        choice = 'y'
        if choice != 'y':
            return

    else:
        interface = sys.argv[1]
        target_mac = sys.argv[2] if len(sys.argv) > 2 else "f4:6b:8c:5e:bb:1e"

    try:
        # 发送测试数据包
        send_test_packets(interface, target_mac)

        # 询问是否发送单个数据包
        print()
        choice = 'y'
        if choice == 'y':
            print()
            msg_type = int(input("输入消息类型 (1=心跳, 2=数据, 3=控制): "))
            seq_num = int(input("输入序列号: "))
            # src_ip = input("输入源IP (默认172.16.36.35): ") or "172.16.36.35"
            # dst_ip = input("输入目的IP (默认172.16.26.224): ") or "172.16.26.224"

            src_ip = "172.16.36.35"
            dst_ip = "172.16.26.224"

            send_single_packet(interface, target_mac, msg_type, seq_num, src_ip, dst_ip)

    except KeyboardInterrupt:
        print("\n程序被用户中断")
    except Exception as e:
        print(f"程序执行出错: {e}")


if __name__ == "__main__":
    main()
