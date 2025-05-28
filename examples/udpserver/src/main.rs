//! Simple UDP server.
//!
//! This server listens for incoming UDP datagrams and sends a fixed response back
//! to the sender.
#![cfg_attr(feature = "axstd", no_std)]
#![cfg_attr(feature = "axstd", no_main)]

#[macro_use]
#[cfg(feature = "axstd")]
extern crate axstd as std;

use std::io::{self, prelude::*};
use std::net::{UdpSocket, SocketAddr};
use std::vec;
use std::vec::Vec;
use std::sync::Arc; // 引入 Arc 用于多线程共享所有权
use std::thread;    // 引入 thread 模块用于创建新线程

const LOCAL_IP: &str = "0.0.0.0";
const LOCAL_PORT: u16 = 5555;

const UDP_RESPONSE_CONTENT: &str = "Hello from ArceOS UDP Server!";

macro_rules! info {
    ($($arg:tt)*) => {
        match option_env!("LOG") {
            Some("info") | Some("debug") | Some("trace") => {
                print!("[INFO] {}\n", format_args!($($arg)*));
            }
            _ => {}
        }
    };
}

// 处理单个 UDP 数据报的函数
// 现在它接收一个 Arc<UdpSocket>，这样它就可以在自己的线程中安全地使用 socket 发送响应
// received_data 也需要是 Vec<u8>，因为原始 buf 会被主线程复用
fn handle_udp_request(socket: Arc<UdpSocket>, received_data: Vec<u8>, src_addr: SocketAddr) -> io::Result<()> {
    info!("Received {} bytes from {}: {:?}", received_data.len(), src_addr, received_data);

    // 使用 Arc 包装的 socket 发送响应
    socket.send_to(UDP_RESPONSE_CONTENT.as_bytes(), src_addr)?;
    info!("Sent response to {}", src_addr);
    Ok(())
}

fn udp_loop() -> io::Result<()> {
    // 绑定 UDP 套接字
    let socket = UdpSocket::bind((LOCAL_IP, LOCAL_PORT))?;
    println!("UDP listen on: {}", socket.local_addr().unwrap());

    // 将 UdpSocket 包装在 Arc 中，以便在多个线程中共享
    let arc_socket = Arc::new(socket);

    // 用于接收数据报的缓冲区
    let mut buf = [0u8; 1024];
    let mut i = 0;

    loop {
        // 在主线程中接收数据报
        // recv_from 仍然在原始的 arc_socket 上调用
        let (len, src_addr) = arc_socket.recv_from(&mut buf)?;
        info!("new client (packet) {}: {}", i, src_addr);

        // 复制接收到的数据，因为 buf 会在下一次循环中被复用
        let received_data_owned = buf[..len].to_vec();

        // 克隆 Arc，将所有权传递给新线程
        let socket_for_thread = Arc::clone(&arc_socket);

        // 为每个数据报创建一个新线程来处理
        thread::spawn(move || {
            // 在新线程中调用处理函数
            match handle_udp_request(socket_for_thread, received_data_owned, src_addr) {
                Err(e) => info!("UDP packet handling error for {}: {:?}", src_addr, e),
                Ok(()) => info!("UDP packet {} from {} handled successfully", i, src_addr),
            }
        });
        i += 1;
    }
}

#[cfg_attr(feature = "axstd", unsafe(no_mangle))]
fn main() {
    println!("Hello, ArceOS UDP server!");
    udp_loop().expect("test UDP server failed");
}
