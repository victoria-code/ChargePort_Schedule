#include<iostream>
#include<string>
#include<math.h>    //floor()

#include "TSocket.h"
#include "main.h"
#include "User.h"
#include "client.h"
using namespace std;


mutex mutexUsr;	//�޸��û���Ϣʱ�õ��Ļ�����
mutex mutexSock;	//ʹ��client_socketʱ�Ļ�����
struct Info send_info, recv_info;

int main()
{
	while (1)
	{
		cout << "\n**************************************************" << endl;
		cout << "***************  ��ӭʹ�ó��׮ϵͳ  *************" << endl;
		cout << "*****************  1.�˳�ϵͳ  *******************" << endl;
		cout << "*****************  2.�û�ע��  *******************" << endl;
		cout << "*****************  3.�û���¼  *******************" << endl;
		cout << "**************************************************" << endl;

		cout << "����������ѡ��";
		string choice;
		while (getline(cin, choice), !isLegalChoice(choice, 3))
			cout << "������Ч��������ѡ��: ";

		Client* client = new Client;
		switch (choice[0] - '0')
		{
		case 1:
			client->close();
			break;
		case 2:
			client->signUp();
			break;
		case 3:
			client->logIN();
			break;
		default:
			cout << "��Чѡ�����������" << endl;
			break;
		}

	}
	return 0;
}
