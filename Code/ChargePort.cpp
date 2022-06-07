#include "ChargePort.h"
const double HIGH_ELEC_PRICE = 1.0;   //峰电价，单位元/度
const double LOW_ELEC_PRICE = 0.4;    //谷电价，单位元/度
const double NORMAL_ELEC_PRICE = 0.7; //平电价，单位元/度
const double SERVICE_PRICE = 0.8;     //服务价，单位元/度
const double FAST_CHARGE_RATE = 30;   //快充功率，单位度/小时
const double SLOW_CHARGE_RATE = 7;    //慢充功率，单位度/小时
CPStatusTable NULLStatusTable;
CarReply NULLCarReply;
ChargePort::ChargePort(int CPID, bool fast, bool on)
{
    this->SID = CPID;
    this->IsFastCharge = fast;
    this->OnState = on;
    IsCharging = IsWaiting = false;
    ChargeCnt = 0;
    ChargeCost = ChargeTime = TotalCharge = ServiceCost = 0;
    //初始化一个空的充电桩状态表
    NULLStatusTable.SID = NULLStatusTable.ChargeCnt = 0;
    NULLStatusTable.OnState = NULLStatusTable.IsCharging = false;
    NULLStatusTable.IsWaiting = NULLStatusTable.IsFastCharge = false;
    NULLStatusTable.ChargeCost = NULLStatusTable.ChargeTime = 0;
    NULLStatusTable.TotalCharge = NULLStatusTable.ServiceCost = 0;
    NULLStatusTable.TableTime = time(NULL);
    //初始化一个空的汽车充电请求回应
    NULLCarReply.Ask.UID = NULLCarReply.Ask.CarID = 0;
    NULLCarReply.Ask.ChargeCap = 0;
    NULLCarReply.Ask.IsFastCharge = false;
    NULLCarReply.Ask.BatteryCap = 0;
    NULLCarReply.Ask.StWaitTime = time(NULL);
    ChargingCar = NULLCarReply;
    WaitingCar = NULLCarReply;
}
void ChargePort::on()
{
    OnState = true;
}
void ChargePort::off()
{
    OnState = false;
}
CPStatusTable ChargePort::GetStatus()
{
    CPStatusTable a = NULLStatusTable;
    a.SID = SID;                   // 充电桩编号
    a.IsFastCharge = IsFastCharge; //快充还是慢充，快充为1,慢充为0
    a.OnState = OnState;           // 开关状态，true开false关
    a.IsCharging = IsCharging;     // 为 true 则充电区有车
    a.IsWaiting = IsWaiting;       //为 true 则等待区有车
    a.ChargeCnt = ChargeCnt;       // 累计充电次数
    a.ChargeCost = ChargeCost;     // 累计充电费用。单位/元
    a.ChargeTime = ChargeTime;     // 累计充电时长。单位/min
    a.TotalCharge = TotalCharge;   // 累计总充电量。单位/度
    a.ServiceCost = ServiceCost;   //累计服务费用。单位/元
    a.TableTime = time(NULL);      //状态信息生成时间
    return a;
}
CarReply ChargePort::GetChargingCar()
{
    if (IsCharging)
        return ChargingCar;
    else
    {
        NULLCarReply.Ask.StWaitTime = time(NULL);
        return NULLCarReply;
    }
}
CarReply ChargePort::GetWaitingCar()
{
    if (IsWaiting)
        return ChargingCar;
    else
    {
        NULLCarReply.Ask.StWaitTime = time(NULL);
        return NULLCarReply;
    }
}
void ChargePort::StopCharge()
{
    IsCharging = IsWaiting;
    ChargingCar = WaitingCar;
    IsWaiting = false;
    WaitingCar = NULLCarReply;
}
void ChargePort::StopWait()
{
    IsWaiting = false;
    WaitingCar = NULLCarReply;
}