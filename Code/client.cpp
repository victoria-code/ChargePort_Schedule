#include "client.h"


extern TSocket client_sock;
extern mutex mutexSock;

int Client::logIN()
{

	int suc = 0;
	cout << "==========登录界面=========" << endl;

	//获取用户输入的用户名和密码
	string usrname, password;
	cout << "请输入用户名：";
	while (getline(cin, usrname), usrname == "")
		;
	cout << "请输入密码：";
	while (getline(cin, password), password == "")
		;

	mutexSock.lock();
	//向服务器发送登录请求
	suc = sendLogInRequest(usrname, password);
	mutexSock.unlock();

	//提示登录成功/失败信息
	// suc=0表示成功，-1表示用户名错误，-2表示密码错误
	if (suc != 0)
	{
		if (suc == -1)
			cout << "登录失败，请检查用户名是否正确" << endl;
		else if (suc == -2)
			cout << "登录失败，请检查密码是否正确" << endl;
		return suc;
	}

	cout << "登录成功！" << endl;

	cur_usr->showMenu();

	return suc;
}


int Client::sendLogInRequest(string usrname, string password)
{

	send_info.cmd = LOG_IN;
	strcpy_s(send_info.UID, usrname.c_str());
	strcpy_s(send_info.PWD, password.c_str());

	//发送报文
	client_sock.Send(send_info);

	//接收服务器发回的响应报文
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, usrname.c_str()))
		client_sock.Recv(recv_info);	//确认是否是发给自己的

	if (recv_info.MODE == 1)	//Customer
	{
		cur_usr = new Customer(usrname, recv_info.MODE);
	}
	else
		cur_usr = new Admin();

	return recv_info.REPLY;
}

int Client::signUp()
{
	cout << "==========注册界面=========" << endl;
	//获取用户输入的用户名和密码
	//对密码没有限制，只要不为空即可；用户名不能与已有的重复
	string usrname, password;
	cout << "请输入用户名：";
	while (getline(cin, usrname), usrname == "")
		;
	cout << "请输入密码：";
	while (getline(cin, password), password == "")
		;

	string choice[] = { "普通用户", "管理员" };
	cout << "请选择您想注册的用户类型: ";
	printChoice(choice, 2);
	string id;
	while (getline(cin, id), !isLegalChoice(id, 2))
	{
		cout << "输入无效,请重新选择: ";
		printChoice(choice, 2);
	}

	//向服务器发送注册申请
	mutexSock.lock();
	int suc = sendSignUpRequest(usrname, password, atoi(id.c_str()));
	mutexSock.unlock();
	if (suc != 0)
	{
		if (suc == -1)
			cout << "注册失败，用户名长度必须在5~20之间" << endl;
		else if (suc == -2)
			cout << "注册失败，此用户名已被使用" << endl;
		return 0;
	}

	cout << "注册成功！" << endl;
	return 0;//注册后返回主界面 暂不登录
}

int Client::sendSignUpRequest(string usrname, string password, int type)
{
	send_info.cmd = SIGN_UP;
	strcpy_s(send_info.UID, usrname.c_str());
	strcpy_s(send_info.PWD, password.c_str());
	send_info.MODE = type;

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, usrname.c_str()))
		client_sock.Recv(recv_info);// 确认是否是发给自己的

	return recv_info.REPLY;
}
