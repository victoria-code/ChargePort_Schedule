#include "client.h"


extern TSocket client_sock;
extern mutex mutexSock;

int Client::logIN()
{

	int suc = 0;
	cout << "==========��¼����=========" << endl;

	//��ȡ�û�������û���������
	string usrname, password;
	cout << "�������û�����";
	while (getline(cin, usrname), usrname == "")
		;
	cout << "���������룺";
	while (getline(cin, password), password == "")
		;

	//����������͵�¼����
	suc = sendLogInRequest(usrname, password);

	//��ʾ��¼�ɹ�/ʧ����Ϣ
	// suc=0��ʾ�ɹ���-1��ʾ�û�������-2��ʾ�������
	if (suc != 0)
	{
		if (suc == -1)
			cout << "��¼ʧ�ܣ����û���������" << endl;
		else if (suc == -2)
			cout << "��¼ʧ�ܣ����������Ƿ���ȷ" << endl;
		return suc;
	}

	cout << "��¼�ɹ���" << endl;

	
	cur_usr->showMenu();

	return suc;
}


int Client::sendLogInRequest(string usrname, string password)
{

	send_info.cmd = LOG_IN;
	strcpy_s(send_info.UID, usrname.c_str());
	strcpy_s(send_info.PWD, password.c_str());

	//���ͱ���
	client_sock.Send(send_info);

	//���շ��������ص���Ӧ����
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, usrname.c_str()))
		client_sock.Recv(recv_info);	//ȷ���Ƿ��Ƿ����Լ���

	if (recv_info.MODE == 1)	//Customer
	{
		cur_usr = new Customer(usrname, recv_info.MODE, recv_info.BALANCE);
	}
	else
		cur_usr = new Admin();

	return recv_info.REPLY;
}

int Client::signUp()
{
	cout << "==========ע�����=========" << endl;
	//��ȡ�û�������û���������
	//������û�����ƣ�ֻҪ��Ϊ�ռ��ɣ��û������������е��ظ�
	string usrname, password;
	cout << "�������û�����";
	while (getline(cin, usrname), usrname == "")
		;
	cout << "���������룺";
	while (getline(cin, password), password == "")
		;

	string choice[] = { "��ͨ�û�", "����Ա" };
	cout << "��ѡ������ע����û�����: ";
	printChoice(choice, 2);
	string id;
	while (getline(cin, id), !isLegalChoice(id, 2))
	{
		cout << "������Ч,������ѡ��: ";
		printChoice(choice, 2);
	}

	//�����������ע������
	mutexSock.lock();
	int suc = sendSignUpRequest(usrname, password, atoi(id.c_str()));
	mutexSock.unlock();
	if (suc != 0)
	{
		if (suc == -1)
			cout << "ע��ʧ�ܣ��û������ȱ�����5~20֮��" << endl;
		else if (suc == -2)
			cout << "ע��ʧ�ܣ����û����ѱ�ʹ��" << endl;
		return 0;
	}

	cout << "ע��ɹ���" << endl;
	return 0;//ע��󷵻������� �ݲ���¼
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
		client_sock.Recv(recv_info);// ȷ���Ƿ��Ƿ����Լ���

	return recv_info.REPLY;
}

void Client::close()
{
	client_sock.Close();
	cout << "��ӭ���´�ʹ�ã�" << endl;
	system("pause");
	exit(0);
}
