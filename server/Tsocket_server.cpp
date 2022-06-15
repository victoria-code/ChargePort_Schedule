#include "TSocket_server.h"

TSocket server_sock;

TSocket::TSocket()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//�����׽���
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//���׽���
	memset(&ser_Addr, 0, sizeof(ser_Addr));  //ÿ���ֽڶ���0���
	ser_Addr.sin_family = PF_INET;
	ser_Addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ser_Addr.sin_port = htons(1234);
	bind(sock, (SOCKADDR*)&ser_Addr, sizeof(SOCKADDR));

	//�������״̬
	listen(sock, 20);
	cout << "������..." << endl;

	char bufSend[BUF_SIZE] = { 0 };	// ���ͻ�����
	char bufRecv[BUF_SIZE] = { 0 };	// ���ջ�����

	memset(bufSend, 0, BUF_SIZE);  //���û�����
	memset(bufRecv, 0, BUF_SIZE);  //���û�����

	//��������
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
	memset(&send_info, 0, sizeof(send_info));	//���ͺ���սṹ��
}


// �����ջ��������Ƶ�msg_recv
void TSocket::Recv(struct Info& recv_info)
{
	memset(bufRecv, 0, BUF_SIZE);	//����֮ǰ���û�������
	memset(&recv_info, 0, sizeof(recv_info));	//��սṹ��

	int strLen = recv(clntSock, bufRecv, BUF_SIZE, 0);
	if (strLen <= 0)
	{
		cout << "Receive from Client Error!" << endl;
	}

	memcpy(&recv_info, bufRecv, sizeof(recv_info));	//����

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

	cout << endl << "�����ѶϿ�..." << endl;
}

