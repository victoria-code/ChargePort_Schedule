#include<iostream>
#include<string>
#include<math.h>    //floor()

#include "TSocket.h"
#include "main.h"
#include "User.h"
#include "client.h"
using namespace std;


mutex mutexUsr;	//修改用户信息时用到的互斥锁
mutex mutexSock;	//使用client_socket时的互斥锁
struct Info send_info, recv_info;

int main()
{
	while (1)
	{
		cout << "\n**************************************************" << endl;
		cout << "***************  欢迎使用充电桩系统  *************" << endl;
		cout << "*****************  1.退出系统  *******************" << endl;
		cout << "*****************  2.用户注册  *******************" << endl;
		cout << "*****************  3.用户登录  *******************" << endl;
		cout << "**************************************************" << endl;

		cout << "请输入您的选择：";
		string choice;
		while(getline(cin, choice), !isLegalChoice(choice, 3))
			cout << "输入无效，请重新选择: ";
		
		Client *client = new Client;
		switch (choice[0] - '0')
		{
		case 1:
			break;
		case 2:
			client->signUp();
		case 3:
			client->logIN();
			break;
		default:
			cout << "无效选项，请重新输入" << endl;
			break;
		}

	}
	return 0;
}
