#ifndef CHARGEPORT_H
#define CHARGEPORT_H
#include "Car.h"
#include <vector>
#include <list>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>
#include "windows.h"
#include<iostream>

typedef struct // ��籨��for admin
{
    int SID;           // ���׮���
    bool IsFastCharge; // ��仹�����䣬���Ϊ1,����Ϊ0
    bool OnState;      // ����״̬��true��false��
    bool IsCharging;   // Ϊ true �������г�

    int WaitNum;   //�ȴ�����������
    int WaitCount; //�ȴ���������Ŀ

    int PastChargeCnt;        // ����ǰ�ۼƳ�����
    double PastChargeCost;    // ����ǰ�ۼ��ܷ��á���λ/Ԫ
    long long PastChargeTime; // ����ǰ�ۼƳ��ʱ������λ/s
    double PastTotalElect;    // ����ǰ�ۼ��ܳ��������λ/��
    double PastElectCost;     // ����ǰ�ۼƳ����á���λ/Ԫ
    double PastServiceCost;   // ����ǰ�ۼƷ�����á���λ/Ԫ

    int ChargeCnt;        // �������ۼƳ�����
    double ChargeCost;    // �������ܷ��á���λ/Ԫ
    long long ChargeTime; // ��������ʱ������λ/min
    double TotalElect;    // �������ܳ��������λ/��
    double ElectCost;     // ����������á���λ/Ԫ
    double ServiceCost;   // �����������á���λ/Ԫ

    time_t TableTime; // ״̬��Ϣ����ʱ��
} CPStatusTable;

//�굥��š��굥����ʱ�䡢���׮��š������������ʱ��������ʱ�䡢ֹͣʱ�䡢�����á�������á��ܷ���
typedef struct // ����굥,for user
{
    int ChargeID;           // �굥���
    time_t CreateTableTime; // �굥����ʱ��
    int SID;                // ���׮���
    bool IsFastCharge;      // ���ģʽ,1Ϊ���
    std::string usrname;    // �����û���
    time_t StartTime;       // ����ʱ��
    time_t End_Time;        // ֹͣʱ��
    double TotalElect;      // �ܳ�����,��λ/Ԫ
    long long ChargeTime;   // ���ʱ������λ/s
    double ChargeCost;      // �ܷ��ã���λ/Ԫ
    double ServiceCost;     // ����ѣ���λ/Ԫ
    double ElectCost;       // ��ѣ���λ/Ԫ
} CostTable;

class ChargePort //���׮
{
public:
    int SID;           // ���׮���
    bool IsFastCharge; // ��仹�����䣬���Ϊ1,����Ϊ0
    bool OnState;      // ����״̬��1��0��

    bool IsCharging;           // Ϊ1������г�
    CarReply ChargingCarReply; // ���ڳ��������ĳ����Ӧ
    int CurElectReq;           // ���ڳ��ĳ��������
    Car* ChargingCar;          // ���ڳ��ĳ�
    bool stopCharging;         // ��ֹ����־

    int WaitNum;                           //�ȴ�����������
    int WaitCount;                         //�ȴ���������Ŀ
    std::vector<CarReply> WaitingCarReply; // ��ǰ�ȴ�����ĳ����Ӧ
    std::vector<Car*> WaitingCar;         // ���ڵȴ��ĳ�

    int ChargeCnt;        // �������ۼƳ�����
    double ChargeCost;    // �������ۼ��ܷ��á���λ/Ԫ
    double ElectCost;     // �������ۼƵ�ѡ���λ/Ԫ
    long long ChargeTime; // �������ۼƳ��ʱ������λ/min
    double TotalElect;    // �������ۼ��ܳ��������λ/��
    double ServiceCost;   // �������ۼƷ�����á���λ/Ԫ

    int PastChargeCnt;        // ����ǰ�ۼƳ�����
    double PastChargeCost;    // ����ǰ�ۼ��ܷ��á���λ/Ԫ
    double PastElectCost;     // ����ǰ�ۼƵ�ѡ���λ/Ԫ
    long long PastChargeTime; // ����ǰ�ۼƳ��ʱ������λ/min
    double PastTotalElect;    // ����ǰ�ۼ��ܳ��������λ/��
    double PastServiceCost;   // ����ǰ�ۼƷ�����á���λ/Ԫ

    std::mutex CPlock; // ���׮������

    ChargePort(int CPID,
        bool fast,
        bool on,
        int WNum = 1,
        int CCnt = 0,
        double CCost = 0,
        double ECost = 0,
        long long CTime = 0,
        double TElect = 0,
        double SCost = 0);              // ���׮��ţ��Ƿ�Ϊ��䣬�Ƿ�Ϊ��,�ȴ�����������,ʣ�µ��ǿ���ǰ��ͳ������,�������ݳ־û�������
    bool off();                                // �رճ��׮��1Ϊ�ɹ�,ʧ������Ϊ�г����ڳ����
    bool on();                                 // �򿪳��׮,1Ϊ�ɹ�
    CPStatusTable GetStatus();                 // ���س��׮״̬
    CarReply GetChargingCar();                 // �������ڳ��ĳ�����Ϣ
    bool AddCar(CarReply myreply, Car* mycar); // ����һ����,1Ϊ�ɹ�
    bool DeleteCar(Car* mycar);                // ɾ��һ����,1Ϊ�ɹ�,0Ϊ��������
    void PutWaitingToCharging();               //��Waiting��Car����Charging���ɳ��׮�Զ����ã������ڱ���ļ�����
};

struct ChargeThreadPool // ��ʼ��������
{
    bool isAvailable;          // �Ƿ����µ�����
    ChargePort* ChargePortPtr; // ���׮
    Car* CarPtr;               // ����
    CarReply* RepPtr;          // �������
    double ElectReq;           // �����
    ChargeThreadPool* next;
};

struct ChargeTablePool // ��ʼ�����굥�����
{
    bool isAvailable;      //�Ƿ����µ�����
    CostTable ChargeTable; //����굥
    ChargeTablePool* next; //�������һ��λ��
};

extern ChargeTablePool* ChargeTableHead; // �ظ�����굥�������ͷ
extern ChargeTablePool* ChargeTableTail; // �ظ�����굥�������β
extern std::mutex ChargeTablelock;       //����굥���󻥳���

int ChargeProc(ChargePort* ChargePortPtr,
    Car* CarPtr,
    CarReply* RepPtr,
    double ElectReq); //һ������̣߳����׮ָ�룬��ָ�룬��籨��ָ�룬�����
void ChargeThread();             //һ�����̳߳�ѭ��������߳�
void BuildChargePortThread();    // �ڷ�������ʼʱ���ã������̹߳������Գ���ģ��
#endif

/*
struct StopThreadPool // ֹͣ��������
{
    bool isAvailable;          //�Ƿ����µ�����
    ChargePort *ChargePortPtr; // ���׮
    Car *CarPtr;               //����
    StopThreadPool *next;
};
void StopChargeThread();
*/
