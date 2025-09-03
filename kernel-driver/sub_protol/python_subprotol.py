'''
*************************************: 
FilePath     : \kernel-driver\sub_protol\python_subprotol.py
version      : 
Author       : dof
Date         : 2025-08-27 17:49:14
LastEditors  : dof
LastEditTime : 2025-08-27 17:49:14
Descripttion :  
compile      :  
**************************************: 
'''
from scapy.all import *
import socket
from random import randint

# 自定义协议号
MY_PROTO_NUMBER = 253


# 自定义协议头（只有一个 type 字段）
class MyProto(Packet):
    name = "MyProto"
    fields_desc = [ByteField("type", 1)]


# 注册协议到 scapy
bind_layers(IP, MyProto, proto=MY_PROTO_NUMBER)


def send_myproto_request(dst_ip):
    pkt = IP(dst=dst_ip, proto=MY_PROTO_NUMBER) / MyProto(type=1)
    send(pkt)
    print(f"Sent MyProto request to {dst_ip}")


def sniff_reply(src_ip):
    def filter_fn(pkt):
        return (IP in pkt and pkt[IP].src == src_ip and
                pkt[IP].proto == MY_PROTO_NUMBER and
                MyProto in pkt and pkt[MyProto].type == 2)

    print("Waiting for reply...")
    pkt = sniff(lfilter=filter_fn, timeout=5, count=1)
    if pkt:
        print("Received reply:", pkt[0].summary())
    else:
        print("No reply received.")


if __name__ == "__main__":
    # 修改为你的目标 IP
    # dst_ip = "192.168.1.1"
    # send_myproto_request(dst_ip)
    # sniff_reply(dst_ip)
    id_ip = randint(1, 65535)  # 随机产生IP ID位
    id_ping = randint(1, 65535)  # 随机产生ping ID位
    seq_ping = randint(1, 65535)  # 随机产生ping序列号位
    # ping指令会使用ICMP传输协议,ICMP报文中要封装IP头部
    packet = IP(src='192.168.1.100', dst='192.168.1.1', tos=4, ttl=64, id=id_ip, proto=253) / ICMP(id=id_ping, seq=seq_ping) / b'welcome'
    # while True:
    res = sr(packet, iface='lan', timeout=0, verbose=False)
    if res:
        print('[*] ' + '192.168.1.1' + ' is active')
    time.sleep(1)
