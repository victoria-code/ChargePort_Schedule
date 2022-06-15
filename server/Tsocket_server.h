#include <Winsock.h>
#include <string>
#include <iostream>
using namespace std;
#pragma warning(disable : 4996)

#include "main.h"

#define BUF_SIZE 3000

#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll

class TSocket {

public:
	WSADATA wsaData;
	SOCKET sock;			//���������׽���
	sockaddr_in ser_Addr;	//���������׽��ֵ�ַ

	SOCKET clntSock;	//�ͻ����׽���
	SOCKADDR clntAddr;	//�ͻ����׽��ֵ�ַ
	int nSize;
	char bufSend[BUF_SIZE];	// ���ͻ�����
	char bufRecv[BUF_SIZE];	// ���ջ�����

	TSocket();//���캯��
	~TSocket();//��������

	void Send(struct Info& send_info);//���ͽṹ��
	void Recv(struct Info& recv_info);//���սṹ��
	void Close();//�Ͽ�����
};


extern TSocket server_sock;

