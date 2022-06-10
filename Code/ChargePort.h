
#ifndef CHARGEPORT_H
#define CHARGEPORT_H
#include "Car.h"
#include <vector>
#include <list>
#include <ctime>
#include "windows.h"
#include <chrono>
#include <thread>
#include <mutex>

typedef struct // 充电报表，for admin
{
    int SID;              // 充电桩编号
    bool IsFastCharge;    // 快充还是慢充，快充为1,慢充为0
    bool OnState;         // 开关状态，true开false关
    bool IsCharging;      // 为 true 则充电区有车
    bool IsWaiting;       // 为 true 则等待区有车
    int ChargeCnt;        // 累计充电次数
    double ChargeCost;    // 累计总费用。单位/元
    long long ChargeTime; // 累计充电时长。单位/min
    double TotalElect;    // 累计总充电量。单位/度
    double ElectCost;     // 累计充电费用。单位/元
    double ServiceCost;   // 累计服务费用。单位/元
    time_t TableTime;     // 状态信息生成时间
} CPStatusTable;

//详单编号、详单生成时间、充电桩编号、充电电量、充电时长、启动时间、停止时间、充电费用、服务费用、总费用
typedef struct // 充电详单,for user
{
    int ChargeID;           // 详单编号
    time_t CreateTableTime; // 详单生成时间
    int SID;                // 充电桩编号
    bool IsFastCharge;      // 充电模式,1为快充
    std::string usrname;    // 车辆用户名
    time_t StartTime;       // 启动时间
    time_t End_Time;        // 停止时间
    double TotalElect;      // 总充电电量,单位/元
    long long ChargeTime;   // 充电时长，单位/s
    double ChargeCost;      // 总费用，单位/元
    double ServiceCost;     // 服务费，单位/元
    double ElectCost;       // 电费，单位/元
} CostTable;

class ChargePort //充电桩
{
public:
    int SID;           // 充电桩编号
    bool IsFastCharge; // 快充还是慢充，快充为1,慢充为0
    bool OnState;      // 开关状态，1开0关

    bool IsCharging;           // 为1充电区有车
    CarReply ChargingCarReply; // 正在充电的汽车的充电响应
    int CurElectReq;           // 正在充电的车待充电量
    Car *ChargingCar;          // 正在充电的车
    bool stopCharging;         // 中止充电标志
    bool IsWaiting;            // 为1则等待区有车
    CarReply WaitingCarReply;  // 当前等待处理的充电响应
    Car *WaitingCar;           // 正在等待的车

    int ChargeCnt;        // 累计充电次数
    double ChargeCost;    // 累计总费用。单位/元
    double ElectCost;     // 累计电费。单位/元
    long long ChargeTime; // 累计充电时长。单位/min
    double TotalElect;    // 累计总充电量。单位/度
    double ServiceCost;   // 累计服务费用。单位/元
    std::mutex CPlock;    // 充电桩互斥锁

    ChargePort(int CPID,
               bool fast,
               bool on,
               int CCnt = 0,
               double CCost = 0,
               double ECost = 0,
               long long CTime = 0,
               double TElect = 0,
               double SCost = 0);              // 充电桩编号，是否为快充，是否为开,剩下的是一些统计数据,便于数据持久化和重载
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
    bool isAvailable;          // 是否有新的请求
    ChargePort *ChargePortPtr; // 充电桩
    Car *CarPtr;               // 汽车
    CarReply *RepPtr;          // 充电请求
    double ElectReq;           // 充电量
    ChargeThreadPool *next;
};

struct ChargeTablePool // 开始返回详单请求池
{
    bool isAvailable;      //是否有新的请求
    CostTable ChargeTable; //充电详单
    ChargeTablePool *next; //链表的下一个位置
};

extern ChargeTablePool *ChargeTableHead; // 回复充电详单请求队列头
extern ChargeTablePool *ChargeTableTail; // 回复充电详单请求队列尾
extern std::mutex ChargeTablelock;       //充电详单请求互斥锁

int ChargeProc(ChargePort *ChargePortPtr,
               Car *CarPtr,
               CarReply *RepPtr,
               double ElectReq); //一个充电线程：充电桩指针，车指针，充电报表指针，充电量
void ChargeThread();             //一个对线程池循环处理的线程
void BuildChargePortThread();    // 在服务器开始时调用，启动线程管理，即对充电的模拟
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
