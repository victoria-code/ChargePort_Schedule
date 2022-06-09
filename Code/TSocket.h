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
	SOCKET sock;
	sockaddr_in cli_Addr;
	char bufSend[BUF_SIZE];	// 发送缓冲区
	char bufRecv[BUF_SIZE];	// 接收缓冲区

	TSocket();
	~TSocket();

	void Send(struct Info& send_info);
	int Recv(struct Info& recv_info);

	void Close();	//断开连接
};


//客户端为client_sock  服务器端改为server_sock即可
//extern TSocket client_sock; 

