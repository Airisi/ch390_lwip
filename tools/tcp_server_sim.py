#!/usr/bin/env python3
"""
模拟 STM32 CH390D 继电器控制板
- TCP Server (端口 23000): 接收 8 字节控制指令，解析并回显
- UDP Discovery (端口 8888): 响应上位机的 DISCOVER 广播，返回设备 IP

协议格式: [0xFE, 通道(1-40), 开/关, 保留, 保留, 保留, 保留, 0xFF]

用法: python3 tcp_server_sim.py [tcp端口]
"""

import socket
import sys
import threading
from datetime import datetime


def get_local_ip() -> str:
    """获取本机局域网 IP 地址"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"

# 通道号 → GPIO 引脚映射表
CHANNEL_MAP = {
    1: "PC13",  2: "PC12",  3: "PC11",  4: "PC10",
    5: "PB9",   6: "PB8",   7: "PB5",   8: "PB4",
    9: "PB3",  10: "PD4",  11: "PD3",  12: "PD2",
   13: "PD1",  14: "PD0",  15: "PC9",  16: "PC8",
   17: "PA15", 18: "PA12", 19: "PA11", 20: "PA10",
   21: "PD9",  22: "PD8",  23: "PC7",  24: "PC6",
   25: "PA9",  26: "PA8",  27: "PB15", 28: "PB14",
   29: "PB13", 30: "PB12", 31: "PB11", 32: "PB10",
   33: "PB2",  34: "PB1",  35: "PB0",  36: "PC5",
   37: "PC4",  38: "PA7",  39: "PA3",  40: "PA5",
}

# 继电器状态（模拟）
relay_state = {i: False for i in range(1, 41)}


def hex_dump(data: bytes) -> str:
    """将字节数据格式化为十六进制字符串"""
    return " ".join(f"{b:02X}" for b in data)


def parse_and_execute(data: bytes) -> str:
    """
    解析 8 字节控制指令并模拟执行
    返回可读的操作描述
    """
    if len(data) != 8:
        return f"长度错误: {len(data)} 字节 (需要 8)"

    if data[0] != 0xFE:
        return f"帧头错误: 0x{data[0]:02X} (需要 0xFE)"

    if data[7] != 0xFF:
        return f"帧尾错误: 0x{data[7]:02X} (需要 0xFF)"

    channel = data[1]
    action = data[2]

    if channel < 1 or channel > 40:
        return f"通道号无效: {channel} (范围 1-40)"

    gpio = CHANNEL_MAP.get(channel, "???")
    state_str = "开" if action == 1 else "关"
    relay_state[channel] = (action == 1)

    return f"继电器 #{channel:2d} ({gpio}) → {state_str}"


def handle_client(conn: socket.socket, addr: tuple):
    """处理单个客户端连接"""
    print(f"[{addr[0]}:{addr[1]}] 已连接")

    try:
        while True:
            data = conn.recv(1024)
            if not data:
                print(f"[{addr[0]}:{addr[1]}] 断开连接")
                break

            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            print(f"[{timestamp}] 收到 {len(data)} 字节: {hex_dump(data)}")

            # 处理可能粘包的情况：按 8 字节切分
            for i in range(0, len(data), 8):
                chunk = data[i:i+8]
                result = parse_and_execute(chunk)
                print(f"  → {result}")

            # 原样回显（与设备行为一致）
            conn.sendall(data)

    except ConnectionResetError:
        print(f"[{addr[0]}:{addr[1]}] 连接被重置")
    except Exception as e:
        print(f"[{addr[0]}:{addr[1]}] 错误: {e}")
    finally:
        conn.close()


def udp_discovery_server(port: int = 8888):
    """
    UDP 广播发现服务
    监听端口 8888，收到 "DISCOVER" 广播后回复设备 IP
    """
    server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    server.bind(("0.0.0.0", port))

    print(f"  UDP Discovery: 监听端口 {port}")

    while True:
        try:
            data, addr = server.recvfrom(1024)
            message = data.decode("utf-8", errors="ignore").strip()
            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]

            if message == "DISCOVER":
                # 上位机检查回复是否包含 "DISCOVERHOST"，包含则认为发现了设备
                response = "DISCOVERHOST".encode("utf-8")
                server.sendto(response, addr)
                print(f"[{timestamp}] UDP DISCOVER 来自 {addr[0]}:{addr[1]} → 回复 DISCOVERHOST")
            else:
                print(f"[{timestamp}] UDP 收到未知消息: {message} (来自 {addr[0]})")

        except Exception as e:
            print(f"UDP Discovery 错误: {e}")


def print_status():
    """打印当前所有继电器状态"""
    print("\n=== 继电器状态 ===")
    for ch in range(1, 41):
        state = "开" if relay_state[ch] else "关"
        gpio = CHANNEL_MAP[ch]
        end = "\n" if ch % 8 == 0 else "  "
        print(f"  #{ch:2d}({gpio}):{state}", end=end)
    print()


def main():
    tcp_port = int(sys.argv[1]) if len(sys.argv) > 1 else 23000
    local_ip = get_local_ip()

    # 启动 UDP Discovery 线程
    udp_thread = threading.Thread(target=udp_discovery_server, args=(8888,), daemon=True)
    udp_thread.start()

    # 启动 TCP Server
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", tcp_port))
    server.listen(5)

    print(f"========================================")
    print(f"  继电器控制板 模拟器")
    print(f"  本机 IP: {local_ip}")
    print(f"  TCP 监听端口: {tcp_port}")
    print(f"  UDP Discovery 端口: 8888")
    print(f"  协议: 8 字节 [FE 通道 开/关 ... FF]")
    print(f"========================================")
    print(f"等待连接...\n")

    try:
        while True:
            conn, addr = server.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.daemon = True
            thread.start()
    except KeyboardInterrupt:
        print("\n\n服务器已停止")
        print_status()
    finally:
        server.close()


if __name__ == "__main__":
    main()
