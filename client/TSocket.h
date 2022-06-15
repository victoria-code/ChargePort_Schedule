#ifndef TSOCKET_H
#define TSOCKET_H

#include <WinSock.h>
#include <string>
#include <iostream>
using namespace std;

#include "main.h"

#define BUF_SIZE 3000

#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll



class TSocket {

public:
	WSADATA wsaData;
	SOCKET sock;
	sockaddr_in cli_Addr;
	char bufSend[BUF_SIZE];	// ���ͻ�����
	char bufRecv[BUF_SIZE];	// ���ջ�����

	TSocket();
	~TSocket();

	void Send(struct Info& send_info);
	int Recv(struct Info& recv_info);

	void Close();	//�Ͽ�����
};


//�ͻ���Ϊclient_sock  �������˸�Ϊserver_sock����
//extern TSocket client_sock; 

#endif
