
#ifndef CHARGEPORT_H
#define CHARGEPORT_H
#include "Car.h"
#include <vector>
#include <list>
#include <ctime>
#include "windows.h"
#include <chrono>
#include <thread>

typedef struct //充电报表，for admin
{
    int SID;            // 充电桩编号
    bool IsFastCharge;  //快充还是慢充，快充为1,慢充为0
    bool OnState;       // 开关状态，true开false关
    bool IsCharging;    // 为 true 则充电区有车
    bool IsWaiting;     // 为 true 则等待区有车
    int ChargeCnt;      // 累计充电次数
    double ChargeCost;  // 累计总费用。单位/元
    double ChargeTime;  // 累计充电时长。单位/min
    double TotalElect;  // 累计总充电量。单位/度
    double ElectCost;   // 累计充电费用。单位/元
    double ServiceCost; // 累计服务费用。单位/元
    time_t TableTime;   // 状态信息生成时间
} CPStatusTable;
typedef struct //充电详单,for user
{
    double ChargeCost;  // 总费用，单位/元
    double ServiceCost; // 服务费，单位/元
    double ElectCost;   // 电费，单位/元
    double TotalElect;  // 总电量,单位/元
    int ChargeTime;     // 充电时间，单位/s
} costTable;
class ChargePort //充电桩
{
public:
    int SID;                   // 充电桩编号
    bool IsFastCharge;         // 快充还是慢充，快充为1,慢充为0
    bool OnState;              // 开关状态，1开0关
    bool IsCharging;           // 为1充电区有车
    CarReply ChargingCarReply; // 正在充电的汽车的充电响应
    Car *ChargingCar;          // 正在充电的车
    bool stopCharging;         // 中止充电标志
    bool IsWaiting;            // 为1则等待区有车
    CarReply WaitingCarReply;  // 当前等待处理的充电响应
    Car *WaitingCar;           // 正在等待的车
    int ChargeCnt;             // 累计充电次数
    double ChargeCost;         // 累计总费用。单位/元
    double ElectCost;          // 累计电费。单位/元
    double ChargeTime;         // 累计充电时长。单位/min
    double TotalElect;         // 累计总充电量。单位/度
    double ServiceCost;        // 累计服务费用。单位/元

    ChargePort(int CPID, bool fast, bool on);  // 充电桩编号，是否为快充，是否为开
    bool off();                                // 关闭充电桩，1为成功,失败是因为有车正在充电区
    bool on();                                 // 打开充电桩,1为成功
    CPStatusTable GetStatus();                 // 返回充电桩状态
    CarReply GetChargingCar();                 // 返回等待服务的车辆信息
    CarReply GetWaitingCar();                  // 返回等待服务的车辆信息
    bool AddCar(CarReply myreply, Car *mycar); // 增加一辆车,1为成功
    bool DeleteCar(Car *mycar);                // 删除一辆车,1为成功,0为车不存在
};
struct ChargeThreadPool // 开始充电请求池
{
    bool isAvailable;          //是否有新的请求
    ChargePort *ChargePortPtr; // 充电桩
    Car *CarPtr;               //汽车
    CarReply *RepPtr;          // 充电请求
    double ElectReq;           // 充电量
    ChargeThreadPool *next;
};
/*
一个充电线程
    充电桩指针，车指针，充电报表指针，指针，充电量，回调函数
*/
int ChargeProc(ChargePort *ChargePortPtr, Car *CarPtr, CarReply *RepPtr, double ElectReq, void (*callback)());
void ChargeThread();          //一个对线程池循环处理的线程
void BuildChargePortThread(); // 在服务器开始时调用，启动线程管理，即对充电的模拟
#endif
/*
struct StopThreadPool // 停止充电请求池
{
    bool isAvailable;          //是否有新的请求
    ChargePort *ChargePortPtr; // 充电桩
    Car *CarPtr;               //汽车
    StopThreadPool *next;
};
void StopChargeThread();
*/