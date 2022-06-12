#include <WinSock2.h>
#include <string>
#include <iostream>
using namespace std;

#include "main.h"

#define BUF_SIZE 200

#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll

class TSocket {

public:
	WSADATA wsaData;
	SOCKET sock;			//服务器端套接字
	sockaddr_in ser_Addr;	//服务器端套接字地址

	SOCKET clntSock;	//客户端套接字
	SOCKADDR clntAddr;	//客户端套接字地址
	int nSize;
	char bufSend[BUF_SIZE];	// 发送缓冲区
	char bufRecv[BUF_SIZE];	// 接收缓冲区

	TSocket();//构造函数
	~TSocket();//析构函数

	void Send(struct Info& send_info);//发送结构体
	void Recv(struct Info& recv_info);//接收结构体
	void Close();//断开连接
};


extern TSocket server_sock;

