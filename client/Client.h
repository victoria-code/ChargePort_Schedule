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

	//用户登录
	int logIN();
	int sendLogInRequest(string usrname, string password);  //发送登录请求报文

	//用户注册
	int signUp();
	int sendSignUpRequest(string usrname, string password, int type); //发送注册请求报文

	//用户注销
	//virtual int deleteAccount() { return 0; }    //由子类实现
	//int sendDeleteRequest(string usrname);    //发送注销请求报文

	//退出系统
	void close();
};
