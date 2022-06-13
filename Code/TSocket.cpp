#include "TSocket.h"

//用户端即client_sock  服务器端改为sever_sock即可
TSocket client_sock;


 TSocket::TSocket()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	memset(&cli_Addr, 0, sizeof(cli_Addr));  //每个字节都用0填充
	cli_Addr.sin_family = PF_INET;
// 	cli_Addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, "127.0.0.1", &cli_Addr.sin_addr.s_addr);
	cli_Addr.sin_port = htons(1234);

	char bufSend[BUF_SIZE] = { 0 };	// 发送缓冲区
	char bufRecv[BUF_SIZE] = { 0 };	// 接收缓冲区

	memset(bufSend, 0, BUF_SIZE);  //重置缓冲区
	memset(bufRecv, 0, BUF_SIZE);  //重置缓冲区

	//建立连接
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(sock, (SOCKADDR*)&cli_Addr, sizeof(SOCKADDR)) == 1)
		cout << "connect succeed!" << endl;

}


 void TSocket::Send(struct Info& send_info)
 {
	 // 将结构体直接转变成字符串输出
	 send(sock, (char*)&send_info, sizeof(send_info), 0);

	 // 发送后清空结构体
	 memset(&send_info, 0, sizeof(send_info));	
 }


 int TSocket::Recv(struct Info& recv_info)
 {
	 memset(bufRecv, 0, BUF_SIZE);		// 清空缓冲区
	 memset(&recv_info, 0, sizeof(recv_info));	//清空结构体

	 int strLen = recv(client_sock.sock, bufRecv, BUF_SIZE, 0);
	 if (strLen == 0)
	 {
		 cout << "Receive from Server Error!" << endl;
	 }

	 memcpy(&recv_info, bufRecv, sizeof(recv_info));	//字符串复制到结构体

	 //输出接收到的结构体
	 //cout << endl << "Info type:" << recv_info.info_type << endl
		// << "Name:" << recv_info.name << endl
		// << "Password:" << recv_info.password << endl ;

	 return strLen;
 }


 TSocket::~TSocket()
{

}


 //断开连接
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

