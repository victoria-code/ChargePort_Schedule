#include "TSocket.h"

//�û��˼�client_sock  �������˸�Ϊsever_sock����
TSocket client_sock;


TSocket::TSocket()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	memset(&cli_Addr, 0, sizeof(cli_Addr));  //ÿ���ֽڶ���0���
	cli_Addr.sin_family = PF_INET;
	cli_Addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cli_Addr.sin_port = htons(1234);

	char bufSend[BUF_SIZE] = { 0 };	// ���ͻ�����
	char bufRecv[BUF_SIZE] = { 0 };	// ���ջ�����

	memset(bufSend, 0, BUF_SIZE);  //���û�����
	memset(bufRecv, 0, BUF_SIZE);  //���û�����

	//��������
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(sock, (SOCKADDR*)&cli_Addr, sizeof(SOCKADDR)) == 1)
		cout << "connect succeed!" << endl;

}


void TSocket::Send(struct Info& send_info)
{
	// ���ṹ��ֱ��ת����ַ������
	send(sock, (char*)&send_info, sizeof(send_info), 0);

	// ���ͺ���սṹ��
	memset(&send_info, 0, sizeof(send_info));
}


int TSocket::Recv(struct Info& recv_info)
{
	memset(bufRecv, 0, BUF_SIZE);		// ��ջ�����
	memset(&recv_info, 0, sizeof(recv_info));	//��սṹ��

	int strLen = recv(client_sock.sock, bufRecv, BUF_SIZE, 0);
	if (strLen == 0)
	{
		cout << "Receive from Server Error!" << endl;
	}

	memcpy(&recv_info, bufRecv, sizeof(recv_info));	//�ַ������Ƶ��ṹ��

	//������յ��Ľṹ��
	//cout << endl << "Info type:" << recv_info.info_type << endl
	   // << "Name:" << recv_info.name << endl
	   // << "Password:" << recv_info.password << endl ;

	return strLen;
}


TSocket::~TSocket()
{

}


//�Ͽ�����
void TSocket::Close()
{
	send_info.cmd = CLOSE;
	client_sock.Send(send_info);

	if (client_sock.Recv(recv_info))
	{
		closesocket(sock);
		WSACleanup();
	}
}

