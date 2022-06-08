#include "ChargePort.h"

const double HIGH_ELEC_PRICE = 1.0;   //峰电价，单位元/度
const double LOW_ELEC_PRICE = 0.4;    //谷电价，单位元/度
const double NORMAL_ELEC_PRICE = 0.7; //平电价，单位元/度
const double SERVICE_PRICE = 0.8;     //服务价，单位元/度
const double FAST_CHARGE_RATE = 30;   //快充功率，单位度/小时
const double SLOW_CHARGE_RATE = 7;    //慢充功率，单位度/小时

CPStatusTable NULLStatusTable; //空状态表
CarReply NULLCarReply;         //空充电请求
ChargeThreadPool *ChargeHead;  // 开始充电请求队列头
ChargeThreadPool *ChargeTail;  // 开始充电请求队列尾
// StopThreadPool *StopHead;      // 结束充电请求池队列头
// StopThreadPool *StopTail;      // 结束充电请求池队列尾

int ChargeProc(ChargePort *ChargePortPtr, Car *CarPtr, CarReply *RepPtr, double ElectReq, void (*callback)())
{
    ++ChargePortPtr->ChargeCnt;
    double ElectNow = 0;            // 当前充电量
    double ServicePrice = 0;        // 当前服务价
    double ElectPrice = 0;          // 当前总电价
    time_t start_time = time(NULL); // 起始时间
    RepPtr->StChargeTime = start_time;
    time_t time_now = start_time;       // 当前时间
    double unitrate = SLOW_CHARGE_RATE; // 充电速率

    double start_ChargeCost = ChargePortPtr->ChargeCost;   // 保留充电桩原数据
    double start_ChargeTime = ChargePortPtr->ChargeTime;   // 保留充电桩原数据
    double start_TotalElect = ChargePortPtr->TotalElect;   // 保留充电桩原数据
    double start_ServiceCost = ChargePortPtr->ServiceCost; // 保留充电桩原数据
    double start_ElectCost = ChargePortPtr->ElectCost;     // 保留充电桩原数据
    double start_BatteryNow = CarPtr->BatteryNow;          // 保留汽车原数据

    if (ChargePortPtr->IsFastCharge)
        unitrate = FAST_CHARGE_RATE;

    while (ElectNow < ElectReq &&
           CarPtr->BatteryNow != CarPtr->BatteryCap &&
           !ChargePortPtr->stopCharging)
    {
        Sleep(3); //每3秒进行一次数据更新。
        time_now = time(NULL);
        int time = time_now - start_time;
        //这里的时间计算加速了，可以后期调整，每1s作为1min计算。
        ElectNow = unitrate * time / 60;
        ElectPrice = HIGH_ELEC_PRICE * ElectNow;
        ServicePrice = SERVICE_PRICE * ElectNow;
        ChargePortPtr->ChargeTime = start_ChargeTime + time;
        ChargePortPtr->TotalElect = start_TotalElect + ElectNow;
        ChargePortPtr->ElectCost = start_ElectCost + ElectPrice;
        ChargePortPtr->ServiceCost = start_ServiceCost + ServicePrice;
        ChargePortPtr->ChargeCost = ChargePortPtr->ServiceCost + ChargePortPtr->ElectCost;
        if (start_BatteryNow + ElectNow < CarPtr->BatteryCap)
            CarPtr->BatteryNow = start_BatteryNow + ElectNow;
        else
        {
            CarPtr->BatteryNow = CarPtr->BatteryCap;
            break;
        }
    }
    costTable costtable;
    costtable.ChargeCost = ElectPrice + ServicePrice;
    costtable.ElectCost = ElectPrice;
    costtable.ChargeTime = time(NULL) - start_time;
    costtable.ServiceCost = ServicePrice;
    costtable.TotalElect = ElectNow;
    ChargePortPtr->stopCharging = false;
    return 0;
}
int aaa(ChargePort *ChargePortPtr, Car *CarPtr, CarReply *RepPtr, double ElectReq)
{
    return 1;
}
void ChargeThread()
{
    for (;;)
    {
        if (ChargeHead->isAvailable)
        {
            ChargeThreadPool *next = ChargeHead->next;
            delete ChargeHead;

            // 从线程池里取出并创造一个充电线程
            std::thread CP(ChargeProc, next->ChargePortPtr, next->CarPtr, next->RepPtr, next->ElectReq);
            CP.detach(); //将该线程分离出去，server关闭时该线程同时结束

            ChargeHead = next;
        }
    }
}
void BuildChargePortThread()
{
    //不仅进行模拟充电的线程处理，一些初始化也在这里完成
    //初始化一个空的充电桩状态表
    NULLStatusTable.SID = NULLStatusTable.ChargeCnt = 0;
    NULLStatusTable.OnState = NULLStatusTable.IsCharging = false;
    NULLStatusTable.IsWaiting = NULLStatusTable.IsFastCharge = false;
    NULLStatusTable.ChargeCost = NULLStatusTable.ChargeTime = 0;
    NULLStatusTable.TotalElect = NULLStatusTable.ServiceCost = NULLStatusTable.ElectCost = 0;
    NULLStatusTable.TableTime = time(NULL);
    //初始化一个空的汽车充电请求回应
    NULLCarReply.Ask.usrname = "";
    NULLCarReply.Ask.CarID = 0;
    NULLCarReply.Ask.ChargeCap = 0;
    NULLCarReply.Ask.IsFastCharge = false;
    NULLCarReply.Ask.BatteryCap = 0;
    NULLCarReply.Ask.StWaitTime = time(NULL);

    ChargeHead = new ChargeThreadPool;
    ChargeTail = ChargeHead;
    ChargeHead->isAvailable = false;
    std::thread Cth(ChargeThread);
    Cth.detach(); //将该线程分离出去，server关闭时该线程同时结束
    /*
        StopHead = new StopThreadPool;
        StopTail = StopHead;
        StopHead->isAvailable = false;
        std::thread Sth(StopChargeThread);
        Sth.detach();
    */
}
ChargePort::ChargePort(int CPID, bool fast, bool on)
{
    this->SID = CPID;
    this->IsFastCharge = fast;
    this->OnState = on;
    IsCharging = IsWaiting = false;
    ChargeCnt = 0;
    ChargeCost = ChargeTime = TotalElect = ServiceCost = ElectCost = 0;
    ChargingCarReply = NULLCarReply;
    WaitingCarReply = NULLCarReply;
}
bool ChargePort::on()
{
    OnState = true;
    return 1;
}
bool ChargePort::off()
{
    if (IsCharging || IsWaiting) // 有车在充电区时失败
    {
        return 0;
    }
    OnState = false;
    return 1;
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
    a.ChargeCost = ChargeCost;     // 累计总费用。单位/元
    a.ChargeTime = ChargeTime;     // 累计充电时长。单位/min
    a.TotalElect = TotalElect;     // 累计总充电量。单位/度
    a.ServiceCost = ServiceCost;   // 累计服务费用。单位/元
    a.ElectCost = ElectCost;       // 累计充电费用。单位/元
    a.TableTime = time(NULL);      // 状态信息生成时间
    return a;
}
CarReply ChargePort::GetChargingCar()
{
    if (IsCharging)
        return ChargingCarReply;
    else
    {
        NULLCarReply.Ask.StWaitTime = time(NULL);
        return NULLCarReply;
    }
}
CarReply ChargePort::GetWaitingCar()
{
    if (IsWaiting)
        return WaitingCarReply;
    else
    {
        NULLCarReply.Ask.StWaitTime = time(NULL);
        return NULLCarReply;
    }
}
bool ChargePort::AddCar(CarReply myreply, Car *mycar)
{
    if (IsCharging)
    {
        if (IsWaiting)
        {
            return false;
        }
        else
        {
            IsWaiting = true;
            WaitingCar = mycar;
            WaitingCarReply = myreply;
        }
    }
    else
    {
        IsCharging = true;
        ChargingCar = mycar;
        ChargingCarReply = myreply;

        //将一个新的充电过程放到充电线程池。
        ChargeThreadPool *nextth = new ChargeThreadPool;
        nextth->isAvailable = false;
        nextth->ChargePortPtr = this;
        nextth->CarPtr = mycar;
        nextth->RepPtr = &(this->ChargingCarReply);
        nextth->ElectReq = myreply.Ask.ChargeCap;

        ChargeTail->next = nextth;
        ChargeTail->isAvailable = true;
    }
    return true;
}
bool ChargePort::DeleteCar(Car *mycar)
{
    if (mycar == WaitingCar)
    {
        WaitingCar = NULL;
        WaitingCarReply = NULLCarReply;
        IsWaiting = false;
        return true;
    }
    else if (mycar == ChargingCar)
    {
        ChargingCar = NULL;
        ChargingCarReply = NULLCarReply;
        stopCharging = true;
        for (;;) //等待stopCharging标志被修改，存在的charging结束
        {
            if (!stopCharging)
                break;
        }
        if (IsWaiting)
        {
            ChargingCar = WaitingCar;
            ChargingCarReply = WaitingCarReply;

            WaitingCar = NULL;
            WaitingCarReply = NULLCarReply;
            IsCharging = true;
            IsWaiting = false;
            //将一个新的充电过程放到充电线程池。
            ChargeThreadPool *nextth = new ChargeThreadPool;
            nextth->isAvailable = false;
            nextth->ChargePortPtr = this;
            nextth->CarPtr = mycar;
            nextth->RepPtr = &(this->ChargingCarReply);
            nextth->ElectReq = this->ChargingCarReply.Ask.ChargeCap;
            ChargeTail->next = nextth;
            ChargeTail->isAvailable = true;
        }
        else
        {
            IsCharging = false;
        }
        return true;
    }
    return false;
}