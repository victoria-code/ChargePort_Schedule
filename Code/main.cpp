#include<iostream>
#include<string>
#include<math.h>    //floor()
#include "main.h"
#include "User.h"
#include "client.h"
#include "TSocket.h"
using namespace std;

struct Info send_info, recv_info;

int main()
{
	while (1)
	{
		cout << "\n**************************************************" << endl;
		cout << "***************  欢迎使用充电桩系统  ****************" << endl;
		cout << "*****************  0.退出系统  *******************" << endl;
		cout << "*****************  1.用户注册  *******************" << endl;
		cout << "*****************  2.用户登录  *******************" << endl;
		cout << "**************************************************" << endl;

		cout << "请输入您的选择：(整数）" << endl;
		int choice = -1;
		cin >> choice;
		while (cin.fail())
		{
			cin.clear();	//清屏
			cin.ignore();	//忽略缓冲区内容
			cout << "您输入的不是整数！请输入一个整数！" << endl;
			cin >> choice;
		}
		
		Client *client = new Client;
		switch (choice)
		{
		case 0:
			break;
		case 1:
			client->signUp();
		case 2:
			client->logIN();
			break;
		default:
			cout << "无效选项，请重新输入" << endl;
			break;
		}

	}
	return 0;
}
