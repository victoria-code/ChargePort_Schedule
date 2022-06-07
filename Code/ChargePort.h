#include <vector>
#include <list>
#include <ctime>
#include "Car.h"
struct CPStatusTable
{
    int SID;            // 充电桩编号
    bool IsFastCharge;  //快充还是慢充，快充为1,慢充为0
    bool OnState;       // 开关状态，true开false关
    bool IsCharging;    // 为 true 则充电区有车
    bool IsWaiting;     //为 true 则等待区有车
    int ChargeCnt;      // 累计充电次数
    double ChargeCost;  // 累计充电费用。单位/元
    double ChargeTime;  // 累计充电时长。单位/min
    double TotalCharge; // 累计总充电量。单位/度
    double ServiceCost; //累计服务费用。单位/元
    time_t TableTime;   //状态信息生成时间
};
class ChargePort
{ //充电桩
public:
    int SID;              // 充电桩编号
    bool IsFastCharge;    //快充还是慢充，快充为1,慢充为0
    bool OnState;         // 开关状态，1开0关
    bool IsCharging;      //为1充电区有车
    CarReply ChargingCar; //正在充电的汽车信息
    bool IsWaiting;       //为1则等待区有车
    CarReply WaitingCar;  //当前等待处理的充电响应
    int ChargeCnt;        // 累计充电次数
    double ChargeCost;    // 累计充电费用。单位/元
    double ChargeTime;    // 累计充电时长。单位/min
    double TotalCharge;   // 累计总充电量。单位/度
    double ServiceCost;   //累计服务费用。单位/元

    ChargePort(int CPID, bool fast, bool on); // 充电桩编号，是否为快充，是否为开
    void off();                               // 关闭充电桩
    void on();                                // 打开充电桩
    CPStatusTable GetStatus();                //返回充电桩状态
    CarReply GetChargingCar();                //返回等待服务的车辆信息
    CarReply GetWaitingCar();                 //返回等待服务的车辆信息
    void StopCharge();                        //正在充电的车停止充电
    void StopWait();                          //正在等待的车停止等待
};
