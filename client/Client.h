#pragma once
#include <string>
#include <iostream>

#include "User.h"
#include "main.h"
#include "TSocket.h"
#include "Admin.h"
using namespace std;

class Client
{
public:
	User* cur_usr = NULL;

	//�û���¼
	int logIN();
	int sendLogInRequest(string usrname, string password);  //���͵�¼������

	//�û�ע��
	int signUp();
	int sendSignUpRequest(string usrname, string password, int type); //����ע��������

	//�û�ע��
	//virtual int deleteAccount() { return 0; }    //������ʵ��
	//int sendDeleteRequest(string usrname);    //����ע��������

	//�˳�ϵͳ
	void close();
};
