#pragma once

#include<vector>
#include<iostream>
#include<string>
#include<utility>
#include<sstream>
#include<iomanip>
#include<fstream>
#include<map>
#include<iterator>
#include<mutex>

//�ɱ����
#define MAX_WAIT_NUM 6  //�Ⱥ������λ����
#define QUEUE_LENGTH 2  //���׮�ŶӶ��г���
#define FAST_NUM 2		//�����׮��Ŀ
#define SLOW_NUM 3		//������׮��Ŀ


#define FAST_POWER 30   //��书�ʣ���λ����/Сʱ��
#define SLOW_POWER 7    //���书�ʣ���λ����/Сʱ��

//���ģʽmode
#define FAST 1
#define SLOW 0

//�û��ͻ��˶Է���������cmd��ţ����Լӣ�
#define LOG_IN  100  //��¼��֤.
#define SIGN_UP 101  //ע����֤��
#define Balance_CHANGE 102 //�û���ֵ or �۷ѡ�
#define DELETE_USER 103//�û�ע����
#define CHARGE_REQUEST 104  //�ύ�������
#define CANCEL_REQUEST 105  //ȡ�����
#define GET_QUEUE_DATA 106  //��ȡ�Ŷ���Ϣ

//�����������ͻ��˷��͵���Ϣ
#define DETAIL 300	//����������+����굥+�۷ѳɹ�����
#define CALL 301	//�кţ��ͻ���������
#define BREAKDOWN 302 //���׮��������+��ǰ����굥+�۷ѳɹ�����


//����Ա�ͻ��˶Է���������cmd��ţ����Լӣ�
#define SPY_CHARGEPORT  200 //���׮��Ϣ���
#define WAIT_CAR_DATA 201	//�鿴�Ⱥ�ĳ�����Ϣ
#define DATA_STATISTICS 202 //�鿴����
#define OPEN_CHARGEPORT 203 //�򿪳��׮
#define CLOSE_CHARGEPORT 204//�رճ��׮

//socket�����õ�������
#define CLOSE 300 //�Ͽ�����

using namespace std;

//�����Ϣ����
#define INFO_SIZE 500

struct Info
{
	int cmd;	//��Ϣ����
	char UID[20];	//�û���
	char PWD[10];	//����	
	int MODE;		//���ģʽ��0��1��  ���ã��û����� 1Customer  2Admin
	int Q_NUM;		//�ŶӺ���
	int W_NUM;		//ǰ���ȴ�����
	double COST;	//������	���ã���ǰ��ص���
	double BatteryCap;	//���������
	double BALANCE;	//���			���ã���������
	int REPLY;		//��Ӧ����ʹ�� ��ʾ������		���ã����׮���
	char output[1000];//��Ӧ�����ֱ�����
};

extern struct Info send_info, recv_info;
