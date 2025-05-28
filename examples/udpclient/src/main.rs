#![cfg_attr(feature = "axstd", no_std)]
#![cfg_attr(feature = "axstd", no_main)]

#[macro_use]
#[cfg(feature = "axstd")]
extern crate axstd as std;

use std::io::{self, prelude::*}; 
use std::net::{UdpSocket, ToSocketAddrs}; 

#[cfg(feature = "dns")]
// 假设有一个在 localhost:12345 上运行的 UDP 回显服务器
const DEST: &str = "localhost:12345";
#[cfg(not(feature = "dns"))]
// 如果没有 DNS 功能，直接使用 IP 地址
const DEST: &str = "127.0.0.1:12345";

// UDP 不发送 HTTP 请求，而是发送任意字节数据。
// 这里我们发送一个简单的字符串作为消息。
const MESSAGE: &str = "Hello from Rust UDP client!";

fn client() -> io::Result<()> {
    // 1. 解析目标地址
    // 对于 UDP，to_socket_addrs 仍然有用，因为它能将 "localhost:12345" 解析为 SocketAddr
    // 但 UdpSocket::send_to 也可以直接接受字符串地址
    for addr in DEST.to_socket_addrs()? {
        println!("dest: {} ({})", DEST, addr);
    }

    // 2. 创建 UDP Socket
    // UDP 客户端通常需要绑定到一个本地地址和端口，以便发送和接收数据。
    // "0.0.0.0:0" 表示绑定到所有可用网络接口的任意可用端口（由操作系统分配）。
    let socket = UdpSocket::bind("0.0.0.0:0")?;
    println!("UDP client bound to: {}", socket.local_addr()?);

    // 4. 发送数据
    // UdpSocket::send_to 方法用于向指定地址发送数据报。
    println!("Sending message: '{}' to {}", MESSAGE, DEST);
    socket.send_to(MESSAGE.as_bytes(), DEST)?;

    // 5. 接收响应
    // UdpSocket::recv_from 方法用于接收数据报，并返回实际读取的字节数和发送方的地址。
    let mut buf = [0; 2048]; // 缓冲区大小
    println!("Waiting for response...");
    let (n, src_addr) = socket.recv_from(&mut buf)?; // n 是读取的字节数，src_addr 是发送方地址

    let response = core::str::from_utf8(&buf[..n]).unwrap();
    println!("Received response from {}: '{}'", src_addr, response);

    Ok(())
}

#[cfg_attr(feature = "axstd", unsafe(no_mangle))]
fn main() {
    println!("Hello, simple UDP client!");
    client().expect("UDP client failed");
}
