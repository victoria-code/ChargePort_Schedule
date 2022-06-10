#ifndef CAR_H
#define CAR_H
#include <ctime>
#include <string>
#include "ChargePort.h" //充电详单

typedef struct //汽车充电请求
{
    std::string usrname;  // 用户ID
    int CarID;            // 车辆ID
    int ChargeCap;        // 请求充电量(充电桩使用)
    bool IsFastCharge;    // 快充还是慢充，快充为1
    double BatteryCap;    // 电池容量
    double BatteryNow;    // 当前电池电量
    time_t StWaitTime;    // 开始等待时间
} CarAsk;

typedef struct // 充电桩对汽车充电响应
{
    CarAsk Ask;           // 包含一个ASK里的信息
    int SID;              // 充电桩ID
    int num;              // 汽车在该充电桩的排队号, 0可以直接充电, 为其他数字表示前面有几辆车
    char MODE;	// F or S
    time_t StChargeTime;  // 开始充电时间(当汽车未开始充电时此值无意义)
    std::string queueNum; //排队号(e.g. F1, S2)
    int waitingNum;       //前车等待数量
} CarReply;

class Car // 车辆
{
public:
    std::string usrname;        // 用户名
    int CarID;                  // 车辆ID
    std::string CarName;        // 车辆名称
    double BatteryCap = 0.0;    // 电池容量
    double BatteryNow = 0.0;    // 当前电池电量
    CarAsk *Ask = nullptr;      // 为NULL时没有ask
    CarReply *Reply = nullptr;  // 为NULL时没有reply
    CostTable *info = nullptr;  //充电详单

    Car(std::string uname = "", int CID = 0, std::string Cname = "", double BCap = 0.0, double BNow = 0.0);
    ~Car();
};
#endif
