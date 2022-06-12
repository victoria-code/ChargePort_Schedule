
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
    int SID;           // 充电桩编号
    bool IsFastCharge; // 快充还是慢充，快充为1,慢充为0
    bool OnState;      // 开关状态，true开false关
    bool IsCharging;   // 为 true 则充电区有车

    int WaitNum;   //等待区车辆上限
    int WaitCount; //等待区车辆数目

    int PastChargeCnt;        // 开机前累计充电次数
    double PastChargeCost;    // 开机前累计总费用。单位/元
    long long PastChargeTime; // 开机前累计充电时长。单位/s
    double PastTotalElect;    // 开机前累计总充电量。单位/度
    double PastElectCost;     // 开机前累计充电费用。单位/元
    double PastServiceCost;   // 开机前累计服务费用。单位/元

    int ChargeCnt;        // 开机后累计充电次数
    double ChargeCost;    // 开机后总费用。单位/元
    long long ChargeTime; // 开机后充电时长。单位/min
    double TotalElect;    // 开机后总充电量。单位/度
    double ElectCost;     // 开机后充电费用。单位/元
    double ServiceCost;   // 开机后服务费用。单位/元

    time_t TableTime; // 状态信息生成时间
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

    int WaitNum;                           //等待区车辆上限
    int WaitCount;                         //等待区车辆数目
    std::vector<CarReply> WaitingCarReply; // 当前等待处理的充电响应
    std::vector<Car *> WaitingCar;         // 正在等待的车

    int ChargeCnt;        // 开机后累计充电次数
    double ChargeCost;    // 开机后累计总费用。单位/元
    double ElectCost;     // 开机后累计电费。单位/元
    long long ChargeTime; // 开机后累计充电时长。单位/min
    double TotalElect;    // 开机后累计总充电量。单位/度
    double ServiceCost;   // 开机后累计服务费用。单位/元

    int PastChargeCnt;        // 开机前累计充电次数
    double PastChargeCost;    // 开机前累计总费用。单位/元
    double PastElectCost;     // 开机前累计电费。单位/元
    long long PastChargeTime; // 开机前累计充电时长。单位/min
    double PastTotalElect;    // 开机前累计总充电量。单位/度
    double PastServiceCost;   // 开机前累计服务费用。单位/元

    std::mutex CPlock; // 充电桩互斥锁

    ChargePort(int CPID,
               bool fast,
               bool on,
               int WNum = 1,
               int CCnt = 0,
               double CCost = 0,
               double ECost = 0,
               long long CTime = 0,
               double TElect = 0,
               double SCost = 0);              // 充电桩编号，是否为快充，是否为开,等待区车辆上限,剩下的是开机前的统计数据,便于数据持久化和重载
    bool off();                                // 关闭充电桩，1为成功,失败是因为有车正在充电区
    bool on();                                 // 打开充电桩,1为成功
    CPStatusTable GetStatus();                 // 返回充电桩状态
    CarReply GetChargingCar();                 // 返回正在充电的车辆信息
    bool AddCar(CarReply myreply, Car *mycar); // 增加一辆车,1为成功
    bool DeleteCar(Car *mycar);                // 删除一辆车,1为成功,0为车不存在
    void PutWaitingToCharging();               //将Waiting的Car放入Charging，由充电桩自动调用，请勿在别的文件调用
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
