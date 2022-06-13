#include "TSocket_server.h"

TSocket server_sock;

TSocket::TSocket()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//创建套接字
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//绑定套接字
	memset(&ser_Addr, 0, sizeof(ser_Addr));  //每个字节都用0填充
	ser_Addr.sin_family = PF_INET;
	ser_Addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ser_Addr.sin_port = htons(1234);
	bind(sock, (SOCKADDR*)&ser_Addr, sizeof(SOCKADDR));

	//进入监听状态
	listen(sock, 20);
	cout << "监听中..." << endl;

	char bufSend[BUF_SIZE] = { 0 };	// 发送缓冲区
	char bufRecv[BUF_SIZE] = { 0 };	// 接收缓冲区

	memset(bufSend, 0, BUF_SIZE);  //重置缓冲区
	memset(bufRecv, 0, BUF_SIZE);  //重置缓冲区

	//接收请求
	nSize = sizeof(SOCKADDR);
	clntSock = accept(sock, (SOCKADDR*)&clntAddr, &nSize);
	if (clntSock != -1)
		cout << "accept succeed!" << endl;
	else
	{
		cout << "accept error!" << endl;
	}

}


void TSocket::Send(struct Info& send_info)
{
	send(clntSock, (char*)&send_info, sizeof(send_info), 0);
	memset(&send_info, 0, sizeof(send_info));	//发送后清空结构体
}


// 将接收缓冲区复制到msg_recv
void TSocket::Recv(struct Info& recv_info)
{
	memset(bufRecv, 0, BUF_SIZE);	//接收之前重置缓冲区！
	memset(&recv_info, 0, sizeof(recv_info));	//清空结构体

	int strLen = recv(clntSock, bufRecv, BUF_SIZE, 0);
	if (strLen <= 0)
	{
		cout << "Receive from Client Error!" << endl;
	}

	memcpy(&recv_info, bufRecv, sizeof(recv_info));	//拷贝

	return;
}


TSocket::~TSocket()
{

}


void TSocket::Close()
{
	server_sock.Send(send_info);

	closesocket(sock);
	closesocket(clntSock);
	WSACleanup();

	cout << endl << "连接已断开..." << endl;
}

