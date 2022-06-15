#include "TSocket.h"	//socket
#include "User.h"
#include "main.h"

extern TSocket client_sock;
extern mutex mutexUsr, mutexSock;

/*
	ר�����ڽ��ձ��ĵĺ���
*/
void Customer::keepRecv()
{
	while (1)
	{
		//mutesSock.lock();
		client_sock.Recv(recv_info);
		switch (recv_info.cmd)
		{
		case DETAIL: //����������+����굥+�۷ѳɹ�����
			printf("%s", recv_info.output);
			//�Ƿ�Ҫ�������굥
			//���±��������Ϣ
			this->balance = recv_info.BALANCE;
			this->car->Reply = nullptr;		//REPLY�ÿ� ���Ŷӽ��
			break;
		case CALL: //�кţ��ͻ���������
			printf("%s", recv_info.output);
			break;
		case BREAKDOWN: //���׮��������+��ǰ����굥+�۷ѳɹ�����
			printf("%s", recv_info.output);
			//���±��������Ϣ
			this->balance = recv_info.BALANCE;
			break;
		default:
			break;
		}
		//mutesSock.unlock();
	}
}

/*
������� �����˳���¼��ע��
*/
void User::clearData()
{
	this->usrname = "";
	this->balance = 0.0;
	this->car = nullptr;
}

/*
����ע������
socket
*/
int User::sendDeleteRequest(string usrname)
{
	send_info.cmd = DELETE_USER;
	strcpy_s(send_info.UID, usrname.c_str());

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);
	return recv_info.REPLY;   //test
}

//--------------------Customer--------------------
/*
������ע��
������������û��������룬�����Ϳ۷�״̬��ɾ���û�����
���أ�ע���ɹ�/ʧ��(1/0)
*/
int Customer::deleteAccount()
{
	cout << "==========ע������=========" << endl;

	//���ȼ���Ƿ��г���������ڽ���
	//ͨ����ѯ�Ŷӽ���е�ǰ���ȴ�������ʵ��
	int suc = 0;
	//mutesSock.lock();
	//mutesUsr.lock();
	//sendQueueInfoRequest();
	//if (this->car && this->car->Reply && this->car->Reply->num == 0)
	if (this->car && this->car->Reply)	//�û��г��ҳɹ����ͳ������
	{
		sendQueueInfoRequest();
		if (this->car->Reply->num == 0)	//������ڳ��
		{
			suc = -1;
			string choice[] = { "�ȴ�����", "ǿ��ֹͣ" };
			cout << "���г���������ڽ��У��Ƿ�ȴ�������? ";
			printChoice(choice, 2);
			string id;
			while (getline(cin, id), !isLegalChoice(id, 2))
			{
				cout << "��������Чѡ��: ";
				printChoice(choice, 2);
			}
			if (id == "1")
			{
				cout << "��ȴ����������ٴγ���ע��" << endl;
				return 0;
			}
			else
			{
				suc = cancelCharge(); //����������
			}
		}
	}

	if (suc == 0) //�����������ע������
		suc = sendDeleteRequest(this->usrname);
	if (suc == 0)	//Server::usrDataUpdate�ķ���ֵֻ����0
	{
		cout << "ע���ɹ���" << endl;
		this->clearData();
	}
	else
		cout << "ע��ʧ�ܣ����Ժ�����" << endl;

	//mutesSock.unlock();
	//mutesUsr.unlock();
	return suc;
}

void Customer::showMenu()
{
	while (true)
	{
		cout << "**************************************************" << endl;
		cout << "******************  1.��ʼ��� *******************" << endl;
		cout << "******************  2.ȡ����� *******************" << endl;
		cout << "******************  3.�Ŷӽ�� *******************" << endl;
		cout << "******************  4.�˻���ֵ *******************" << endl;
		cout << "******************  5.�˳���¼ *******************" << endl;
		cout << "******************  6.ע���˺� *******************" << endl;
		cout << "**************************************************" << endl;
		cout << "\n����������ѡ��: ";
		string choice;
		while (getline(cin, choice), !isLegalChoice(choice, 6))
			cout << "������Ч��������ѡ��: ";

		switch (choice[0] - '0')
		{
		case 1:
		{
			this->newChargeRequest();
			break;
		}
		case 2:
		{
			this->cancelCharge();
			break;
		}
		case 3:
		{
			this->getQueueRes();
			break;
		}
		case 4:
		{
			this->recharge();
			break;
		}
		case 5:
		{
			this->clearData();
			return;
		}
		case 6:
		{
			this->deleteAccount();
			return;
		}
		default:
			break;
		}
	}
}

/*
��ֵ
���������ṩ�û������Ϣ�������û����
���أ���ֵ�ɹ�/ʧ��(1/0)
*/
int Customer::recharge()
{
	cout << "==========��ֵ����=========" << endl;
	cout << "��ǰ�˻����: " << this->balance << endl;
	string choice[] = { "20", "50", "100", "200", "�Զ���" };
	cout << "��ѡ����Ҫ��ֵ�Ľ��(��λ��Ԫ): ";
	printChoice(choice, 5);

	//��ȡ�û�ѡ��
	string id_str;
	int id;
	while (getline(cin, id_str), !isLegalChoice(id_str, 5))
	{
		cout << "������Ч��������ѡ��";
		printChoice(choice, 5);
	}

	id = id_str[0] - '0';
	double amount = 0.0;
	switch (id)
	{
	case 1:
		amount = 20.0;
		break;
	case 2:
		amount = 50.0;
		break;
	case 3:
		amount = 100.0;
		break;
	case 4:
		amount = 200.0;
		break;
	case 5:
		cout << "�������Զ����ֵ��";
		string str;
		while (getline(cin, str), !isPosNum(str))
			cout << "��������Ч������������:";
		const char* s = str.c_str();
		amount = atof(s);
		break;
	}
	//����������ͳ�ֵ����
	//mutesSock.lock();
	int suc = sendUpdateBalanceRequest(this->balance + amount);
	this->balance += amount;
	cout << "��ֵ�ɹ�������ǰ�����Ϊ��" << this->balance << "Ԫ��" << endl;
	//mutesSock.unlock();
	//if (suc == 0) //��ֵ�ɹ������±�������
	//{
	//	//mutesUsr.lock();
	//	this->balance += amount;
	//	cout << "��ֵ�ɹ�������ǰ�����Ϊ��" << this->balance << "Ԫ��" << endl;
	//	//mutesUsr.unlock();
	//}
	//else
	//	cout << "��ֵʧ�ܣ����Ժ�����" << endl;
	return 1;
}

/*
���͸����������
socket
*/
int Customer::sendUpdateBalanceRequest(double add)
{
	send_info.cmd = Balance_CHANGE;
	send_info.BALANCE = add;
	strcpy_s(send_info.UID, this->usrname.c_str());

	client_sock.Send(send_info);
	//client_sock.Recv(recv_info);
	//while (strcmp(recv_info.UID, this->usrname.c_str()))
	//	client_sock.Recv(recv_info);

	return 1;
}

/*
�ύ�������
�������������Ŷӽ��
*/
int Customer::newChargeRequest()
{
	cout << "==========����������=========" << endl;
	//���ȼ���������Ƿ�����˳���
	if (!this->car)
	{
		this->car = new Car;
		cout << "ϵͳ��⵽����δ��ӳ������������Ƴ�����Ϣ" << endl;
		cout << "�����복���������(��λ���ȣ������λС��): ";
		string kwh_str;
		while (getline(cin, kwh_str), !isPosNum(kwh_str))
			cout << "������Ч������������������: ";
		const char* s = kwh_str.c_str();
		double kwh = atof(s);
		this->car->BatteryCap = kwh;	//���õ������
	}

	if (this->car->Reply)	//����Ѿ������˳������
	{
		//���ȼ���Ƿ��Ѿ���ʼ���
		//mutesSock.lock();
		//mutesUsr.lock();
		sendQueueInfoRequest();
		//mutesSock.unlock();
		// status=1��ʾ���ڳ�磬=0��ʾδ���ڳ��
		if (this->car && this->car->Reply && this->car->Reply->num == 0)
		{
			cout << "�ܱ�Ǹ�����ѿ�ʼ��磬�޷��ύ�µĳ������" << endl;
			//mutesUsr.unlock();
			return 0;
		}

		//����δ��ʼ���
		cout << "ϵͳ��⵽���Ѿ��ύ��������룬�����Ƿ�Ҫ�޸ĸ����룿" << endl;
		string choice[] = { "�޸�", "���޸�" };
		printChoice(choice, 2);
		string co;
		while (getline(cin, co), !isLegalChoice(co, 2))
			cout << "������Ч��������ѡ��: ";

		if (co == "2")
			return 0;
	}


	//��ȡ�û�����ĳ��ģʽ�ͳ����
	int mode, charge; // charge: �����
	string choice1[] = { "���", "����" };
	cout << "��ѡ����ģʽ: ";
	printChoice(choice1, 2);
	string id;
	while (getline(cin, id), !isLegalChoice(id, 2))
	{
		cout << "������Ч��������ѡ��: ";
		printChoice(choice1, 2);
	}
	mode = (id == "1" ? FAST : SLOW);

	//�����������綯�����ĵ������Ϊ15~60kWh
	string choice2[] = { "10", "20", "30", "40", "50", "60" };
	int charge_n[] = { 0, 10, 20, 30, 40, 50, 60 };
	cout << "����������(��λ����): ";
	printChoice(choice2, 6);
	while (getline(cin, id), !isLegalChoice(id, 6))
	{
		cout << "������Ч��������ѡ��";
		printChoice(choice2, 6);
	}
	charge = charge_n[id[0] - '0'];
	//������������������������Զ�����Ϊ�������������ȡ����
	double cap = this->car->BatteryCap;
	if (charge > cap)
	{
		cout << "��ѡ��ĳ���������˳������������ϵͳ�ѽ����Զ�����Ϊ�����������: " << cap << "kWh\n";
		charge = floor(cap);
	}

	//���ó������
	/*
	std::string usrname;  // �û�ID
	int ChargeCap;        // ��������(���׮ʹ��)
	bool IsFastCharge;    // ��仹�����䣬���Ϊ1
	double BatteryCap;    // �������
	double BatteryNow;    // ��ǰ��ص���
	time_t StWaitTime;    // ��ʼ�ȴ�ʱ��
	*/
	CarAsk* ask = new CarAsk;
	ask->usrname = this->usrname;
	ask->ChargeCap = charge;
	ask->IsFastCharge = (mode == FAST ? true : false);
	ask->BatteryCap = this->car->BatteryCap;
	ask->BatteryNow = this->car->BatteryNow;
	ask->StWaitTime = time(NULL);
	//��ʼ�ȴ�ʱ���ɷ���������׮���룿

	//mutesUsr.lock();
	//mutesSock.lock();
	this->car->Ask = ask;
	//��������ύ�������
	int suc = 0;
	suc = sendChargeRequest(); //ֱ�Ӷ�ȡthis->car->Ask������
	if (suc == 0)
	{
		cout << "�ύ�������ɹ������ڻ�ȡ�Ŷ���Ϣ..." << endl;
		cout << "����ǰ���ŶӺ���Ϊ: " << this->car->Reply->queueNum << ", ǰ������" << this->car->Reply->waitingNum << "�����ڵȴ�" << endl;
		Customer* tmp = this;
		std::thread sub{ &Customer::keepRecv,&(*tmp) };
		sub.detach();
	}
	else 
	{
		this->car->Reply = nullptr;
		cout << "�ύ�������ʧ�ܣ�" << endl;
		printf("%s", recv_info.output);
	}
	/*if (suc == -1) {
		cout << "�ύ�������ʧ�ܣ�������ǰλ�ڳ�������޷��ύ�µĳ������" << endl;
	}
	else if (suc == -2) {
		cout << "�ύ�������ʧ�ܣ���ǰ�Ⱥ����������޷������µ��û��������Ժ�����" << endl;
	}
	else if (suc == -3) {
		cout << "�ύ�������ʧ�ܣ�ϵͳ��������㣬���ܽ��г�磬���ֵ�����ύ�������" << endl;
	}*/


	//mutesSock.lock();
	//mutesUsr.lock();
	return suc;
}


/*
���ͳ������
socket
*/
int Customer::sendChargeRequest()
{
	send_info.cmd = CHARGE_REQUEST;//�˴�δ��¼������� ��server��Ҫ�ټ�
	strcpy_s(send_info.UID, this->usrname.c_str());
	send_info.MODE = this->car->Ask->IsFastCharge == true ? FAST : SLOW;
	send_info.COST = this->car->Ask->BatteryNow;	//COST����Ϊ��ǰ����
	send_info.BatteryCap = this->car->Ask->BatteryCap;	//BatteryCap��¼���������
	send_info.BALANCE = this->car->Ask->ChargeCap;	//BANLANCE����Ϊ�����

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);

	//�����Ŷӽ��
	CarReply* reply = new CarReply;
	reply->SID = recv_info.REPLY; //��REPLY����Ϊ���׮���
	reply->num = recv_info.Q_NUM;
	reply->waitingNum = recv_info.W_NUM;
	reply->MODE = recv_info.MODE == 1 ? 'F' : 'T';

	char QNum[10]; // F1000
	QNum[0] = (recv_info.MODE == 1 ? 'F' : 'T');
	_itoa_s(reply->num, QNum + 1, 9, 10);
	reply->queueNum = QNum;
	this->car->Reply = reply;

	return recv_info.REPLY;
}

/*
ȡ����������
*/
// int Customer::cancelChargeRequest()
int Customer::cancelCharge()
{
	if (!this->car || this->car->Reply == nullptr)
	{
		cout << "����ǰ��δ����������룡" << endl;
		return -1;
	}
	cout << "==========ȡ��������=========" << endl;
	cout << "�Ƿ�ȡ�����? 1. ��    2. ��" << endl;
	string id;
	while (getline(cin, id), !isLegalChoice(id, 2))
		cout << "������Ч��������ѡ��: 1. ��    2. ��" << endl;

	int suc = 0;
	if (id == "1")
	{
		cout << "���ڳ���ȡ�����..." << endl;
		//�����������ȡ����������
		//mutesSock.lock();
		suc = sendCancelRequest();
		//mutesSock.unlock();
		if (suc == 0)
		{
			cout << "ȡ���ɹ���" << endl;
			printf("%s", recv_info.output);
			//mutesUsr.lock();
			this->car->Ask = nullptr;
			this->car->Reply = nullptr;
			//mutesUsr.unlock();
		}
		else if (suc == -1)
			cout << "ȡ�����ʧ�ܣ�����ǰδ�ύ�������" << endl;
	}
	return suc;
}
/*����ȡ����������
socket
*/
int Customer::sendCancelRequest()
{
	send_info.cmd = CANCEL_REQUEST;
	strcpy_s(send_info.UID, this->usrname.c_str());
	//Ŀǰֻ����˻�����Ϣ ����Ҫ�õ�Ask�ٺ������

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);

	// clearASK

	return recv_info.REPLY;
}

/*
�鿴�Ŷӽ��
�������������Ŷӽ��
*/
int Customer::getQueueRes()
{
	if (!this->car || this->car->Reply == nullptr)
	{
		cout << "����ǰ��δ����������룬����������ٲ鿴�Ŷӽ����" << endl;
		return -1;
	}

	cout << "==========�Ŷ���Ϣ����=========" << endl;
	//mutesSock.lock();
	//mutesUsr.lock();
	int q = sendQueueInfoRequest();
	//mutesUsr.unlock();
	//mutesSock.unlock();
	CarReply* reply = this->car->Reply;
	cout << "����ǰ���ŶӺ���Ϊ: " << reply->queueNum << ", ǰ������" << reply->num << "�����ڵȴ�" << endl;
	return 1;
}

//���Ͳ鿴�Ŷӽ��������
// socket
int Customer::sendQueueInfoRequest()
{
	send_info.cmd = GET_QUEUE_DATA;
	strcpy_s(send_info.UID, this->usrname.c_str());

	client_sock.Send(send_info);
	cout << "Send Succeed" << endl;
	client_sock.Recv(recv_info);
	cout << "recvive " << recv_info.UID << endl;
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);

	//�����Ŷӽ�� 
	//�˴�ֻ����һ��ǰ���ȴ����� ��Ϊ��������������ʱ����
	this->car->Reply->num = recv_info.W_NUM;
	cout << "ǰ���ȴ�������" << this->car->Reply->num << endl;
	printf("%s", recv_info.output);
	return 0;
}

/*------Utils------*/

//�жϳ�ֵ���Ϸ���
//�ж������Ƿ�Ϊ���������ͻ򸡵��ͣ������С��λ��Ϊ2��
bool isPosNum(string str)
{
	if (str == "")
		return false;

	int flag = 0;
	for (int i = 0; i < str.size(); i++)
	{
		if (flag > 2)
			return false; //���С��λ��Ϊ2

		if (str[i] == '.')
		{
			if (flag)
				return false; //С����ֻ�ܳ���һ��
			if (i == str.size() - 1 || i == 0)
				return false; //С���㲻���ǵ�һλ�����һλ
			flag = 1;
			continue;
		}
		if (str[i] < '0' || str[i] > '9')
			return false;

		if (flag) //��¼С������λ��
			flag++;
	}
	const char* s = str.c_str();
	if (atof(s) <= 0.0) //��ֵ������Ϊ����
		return false;

	return true;
}

//�ж������Ƿ�Ϊ�Ϸ�ѡ��(max: ���ѡ�����)
bool isLegalChoice(string str, int max)
{
	if (str.size() > 1)
		return false;
	int id = str[0] - '0';
	if (id < 1 || id > max)
		return false;
	return true;
}

//���ѡ��
void printChoice(const string choice[], int size)
{
	string separator = "    ";
	string str = "";
	for (int i = 1; i <= size; i++)
	{
		cout << i << ". " << choice[i - 1] << separator;
	}
	cout << endl;
}

//����ת��Ϊʱ�����ʽ
//string User::getTime(int sec)
string getTime(long long sec)
{
	int h, m, s;
	h = sec / 3600;
	sec = sec % 3600;
	m = sec / 60;
	s = sec % 60;
	return to_string(h) + "ʱ" + to_string(m) + "��" + to_string(s) + "��";
}


//void Admin::showMenu()
//{
//	//�������Ա��ع���ѡ��
//}
