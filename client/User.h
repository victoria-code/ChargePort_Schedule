#ifndef __USER_H__
#define __USER_H__
#pragma once

#include "Car.h"    //������
//#include "ChargePort.h"	//����굥

#include<iostream>
#include<string>
#include<math.h>    //floor()

using namespace std;

enum USER_TYPE { ADMIN = 0, CUSTOMER };

class User {     //�û�����
protected:
	//public: //test
	int type;   //�û����ͣ�1�������ߣ�0������Ա��
	string usrname = "";    //�û���
	double balance = 0.0;   //�����������������壩
	Car* car = nullptr;    //�����ߵĳ���

public:
	~User() {
		if (this->car != nullptr)
			delete this->car;
	}

	//�û�ע��
	virtual int deleteAccount() { return 0; }    //������ʵ��
	int sendDeleteRequest(string usrname);    //����ע��������

	//�û��ɹ���¼����ʾ�Ĳ˵�
	virtual void showMenu() { return; };

	//������� �����˳���¼��ע��
	void clearData();
};

class Customer : public User
{
public:
	//CostTable* info = nullptr; //����굥

	Customer()
	{
		this->type = CUSTOMER;
	}

	Customer(string name, int t, double ban)
	{
		this->usrname = name;
		this->type = t;
		this->balance = ban;
	}

	//������ע��
	int deleteAccount();

	//�����ߵ�¼�ɹ�����ʾ�Ĳ˵�
	void showMenu();

	//��ֵ
	int recharge();
	//int sendRechargeRequest(double amount);     //���ͳ�ֵ������
	int sendUpdateBalanceRequest(double amount);	//���������±���


	//�ύ�������
	int newChargeRequest();
	int sendChargeRequest();    //���ͳ��������
	//ȡ�����
	int cancelCharge();
	int sendCancelRequest();    //����ȡ�����ı���

	//�鿴�Ŷӽ��
	int getQueueRes();
	int sendQueueInfoRequest();

	int keepRecv();	//����������Ϣ���̺߳���

};

////test
//class Admin : public User
//{
//public:
//	Admin() {
//		this->type = ADMIN;
//	}
//
//	//����Ա��¼�ɹ�����ʾ�Ĳ˵�
//	void showMenu();
//};

/*------Utils------*/
//�жϳ�ֵ���Ϸ���
bool isPosNum(string str);
//�ж������Ƿ��ǺϷ�ѡ��(max: ���ѡ�����)
bool isLegalChoice(string str, int max);
//���ѡ��
void printChoice(const string choice[], int size);
//��ȡʱ�䣨��ת��Ϊʱ���룩
// string getTime(int sec);
string getTime(long long sec);

#endif
