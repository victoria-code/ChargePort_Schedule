#ifndef CAR_H
#define CAR_H
#include <ctime>
#include <string>
//#include "ChargePort.h" //����굥

typedef struct //�����������
{
    std::string usrname; // �û�ID
    int ChargeCap;       // ��������(���׮ʹ��)
    bool IsFastCharge;   // ��仹�����䣬���Ϊ1
    double BatteryCap;   // �������
    double BatteryNow;   // ��ǰ��ص���
    time_t StWaitTime;   // ��ʼ�ȴ�ʱ��
} CarAsk;

typedef struct // ���׮�����������Ӧ
{
    CarAsk Ask;           // ����һ��ASK�����Ϣ
    int SID;              // ���׮ID
    int num;              // �����ڸó��׮���ŶӺ�, 0����ֱ�ӳ��, Ϊ�������ֱ�ʾǰ���м�����
    char MODE;            // F or S
    time_t StChargeTime;  // ��ʼ���ʱ��(������δ��ʼ���ʱ��ֵ������)
    std::string queueNum; //�ŶӺ�(e.g. F1, S2)
    int waitingNum;       //ǰ���ȴ�����
} CarReply;

class Car // ����
{
public:
    std::string usrname;       // �û���
    double BatteryCap = 0.0;   // �������
    double BatteryNow = 0.0;   // ��ǰ��ص���
    CarAsk* Ask = nullptr;     // ΪNULLʱû��ask
    CarReply* Reply = nullptr; // ΪNULLʱû��reply
    // CostTable *info = nullptr; //����굥

    Car(std::string uname = "", std::string Cname = "", double BCap = 0.0, double BNow = 0.0);
    ~Car();
};
#endif
