#include <ctime>
#include <string>
#ifndef CAR_H
#define CAR_H
typedef struct //汽车充电请求
{
    std::string usrname;  // 用户ID
    int CarID;            // 车辆ID
    int ChargeCap;        // 请求充电量
    bool IsFastCharge;    // 快充还是慢充，快充为1
    double BatteryCap;    // 电池容量
    double BatteryNow;    // 当前电池电量
    double RequestCharge; // 请求充电量
    time_t StWaitTime;    // 开始等待时间
} CarAsk;

typedef struct // 充电桩对汽车充电响应
{
    CarAsk Ask;          // 包含一个ASK里的信息
    int SID;             // 充电桩ID
    int num;             // 汽车在该充电桩的排队号, 0可以直接充电, 为其他数字表示前面有几辆车
    time_t StChargeTime; // 开始充电时间(当汽车未开始充电时此值无意义)
} CarReply;

class Car // 车辆
{
public:
    int UID;           // 用户ID
    int CarID;         // 车辆ID
    int CarName;       // 车辆名称
    double BatteryCap; // 电池容量
    double BatteryNow; // 当前电池电量
    CarAsk *Ask;       // 为NULL时没有ask
    CarReply *Reply;   // 为NULL时没有reply
};
#endif