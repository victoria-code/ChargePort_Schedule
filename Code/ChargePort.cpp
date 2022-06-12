#include "ChargePort.h"

const double HIGH_ELEC_PRICE = 1.0;   // 峰电价，单位元/度
const double LOW_ELEC_PRICE = 0.4;    // 谷电价，单位元/度
const double NORMAL_ELEC_PRICE = 0.7; // 平电价，单位元/度
const double SERVICE_PRICE = 0.8;     // 服务价，单位元/度
const double FAST_CHARGE_RATE = 30;   // 快充功率，单位度/小时
const double SLOW_CHARGE_RATE = 7;    // 慢充功率，单位度/小时

CPStatusTable NULLStatusTable; // 空状态表
CarReply NULLCarReply;         // 空充电请求
ChargeThreadPool *ChargeHead;  // 开始充电请求队列头
ChargeThreadPool *ChargeTail;  // 开始充电请求队列尾
int ChargeTableID = 0;         // 用来生成充电详单ID
// StopThreadPool *StopHead;      // 结束充电请求池队列头
// StopThreadPool *StopTail;      // 结束充电请求池队列尾

ChargeTablePool *ChargeTableHead; // 回复充电详单请求队列头
ChargeTablePool *ChargeTableTail; // 回复充电详单请求队列尾

std::mutex Chargelock;      // 添加充电过程到线程池的互斥锁
std::mutex ChargeTablelock; // 添加充电详单到请求池的互斥锁

int ChargeProc(ChargePort *ChargePortPtr, Car *CarPtr, CarReply *RepPtr, double ElectReq)
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
        // Sleep(3); // 每3秒进行一次数据更新。
        Sleep(3000); // 每3秒进行一次数据更新。
        time_now = time(NULL);
        int time = time_now - start_time;
        // 这里的时间计算加速了，可以后期调整，每1s作为1min计算。
        ElectNow = unitrate * time / 60;
        ElectPrice = HIGH_ELEC_PRICE * ElectNow;
        ServicePrice = SERVICE_PRICE * ElectNow;
        ChargePortPtr->ChargeTime = start_ChargeTime + time;
        ChargePortPtr->TotalElect = start_TotalElect + ElectNow;
        ChargePortPtr->CurElectReq = min(ElectReq - ElectNow, CarPtr->BatteryCap - CarPtr->BatteryNow);
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

    // 更新一张充电表
    CostTable costtable{++ChargeTableID,
                        time(NULL),
                        ChargePortPtr->SID,
                        ChargePortPtr->IsFastCharge, // 充电模式
                        CarPtr->usrname,
                        start_time,
                        time_now,
                        ElectNow,
                        time_now - start_time,
                        ElectPrice + ServicePrice,
                        ServicePrice,
                        ElectPrice};
    for (;;)
    {
        if (ChargeTablelock.try_lock())
        {
            break;
        }
    }
    ChargeTablePool *nextct = new ChargeTablePool;
    nextct->isAvailable = false;
    nextct->ChargeTable = costtable;
    ChargeTableTail->next = nextct;
    ChargeTableTail->isAvailable = true;
    ChargeTableTail = nextct;
    ChargeTablelock.unlock();
    // 若存在StopCharging则直接结束，deleteCar负责调度waiting和charging
    if (ChargePortPtr->stopCharging)
    {
        ChargePortPtr->stopCharging = false;
        return 0;
    }
    //把下一个waiting放入charging
    for (;;)
    {
        if (ChargePortPtr->CPlock.try_lock())
        {
            break;
        }
    }
    if (ChargePortPtr->WaitCount != 0)
        ChargePortPtr->PutWaitingToCharging();
    else
    {
        ChargePortPtr->IsCharging = false;
    }
    ChargePortPtr->CPlock.try_lock();
    return 0;
}
void ChargeThread()
{
    for (;;)
    {
        if (ChargeHead->isAvailable)
        {
            for (;;)
            {
                if (Chargelock.try_lock())
                {
                    break;
                }
            }
            ChargeThreadPool *next = ChargeHead->next;
            delete ChargeHead;

            // 从线程池里取出并创造一个充电线程
            std::thread CP(ChargeProc, next->ChargePortPtr, next->CarPtr, next->RepPtr, next->ElectReq);
            CP.detach(); // 将该线程分离出去，不阻塞，线程运行完结束，server关闭时该线程也会结束

            ChargeHead = next;
            Chargelock.unlock();
        }
    }
}
void BuildChargePortThread()
{
    // 不仅进行模拟充电的线程处理，一些初始化也在这里完成
    // 初始化一个空的充电桩状态表
    NULLStatusTable.SID = NULLStatusTable.ChargeCnt = 0;
    NULLStatusTable.OnState = NULLStatusTable.IsCharging = false;
    NULLStatusTable.WaitCount = 0;
    NULLStatusTable.WaitNum = 1;
    NULLStatusTable.IsFastCharge = false;
    NULLStatusTable.ChargeCost = NULLStatusTable.ChargeTime = 0;
    NULLStatusTable.TotalElect = NULLStatusTable.ServiceCost = NULLStatusTable.ElectCost = 0;
    NULLStatusTable.TableTime = time(NULL);
    // 初始化一个空的汽车充电请求回应
    NULLCarReply.Ask.usrname = "";
    NULLCarReply.Ask.ChargeCap = 0;
    NULLCarReply.Ask.IsFastCharge = false;
    NULLCarReply.Ask.BatteryCap = 0;
    NULLCarReply.Ask.StWaitTime = time(NULL);

    ChargeTableHead = new ChargeTablePool;
    ChargeTableTail = ChargeTableHead;
    ChargeTableHead->isAvailable = false;

    ChargeHead = new ChargeThreadPool;
    ChargeTail = ChargeHead;
    ChargeHead->isAvailable = false;

    std::thread Cth(ChargeThread); // 处理充电的线程
    Cth.detach();                  // 将该线程分离出去，server关闭时该线程同时结束
    /*
        StopHead = new StopThreadPool;
        StopTail = StopHead;
        StopHead->isAvailable = false;
        std::thread Sth(StopChargeThread);
        Sth.detach();
    */
}
ChargePort::ChargePort(int CPID, bool fast, bool on, int WNum, int CCnt, double CCost, double ECost, long long CTime, double TElect, double SCost)
{
    this->SID = CPID;
    this->IsFastCharge = fast;
    this->OnState = on;
    WaitNum = WNum;
    WaitCount = 0;
    IsCharging = false;
    ChargeCnt = CurElectReq = 0;
    ChargingCarReply = NULLCarReply;
    WaitingCarReply.clear();
    ChargingCar = NULL;
    WaitingCar.clear();

    PastChargeCnt = CCnt;
    PastChargeCost = CCost;
    PastElectCost = ECost;
    PastChargeTime = CTime;
    PastTotalElect = TElect;
    PastServiceCost = SCost;

    ChargeCnt = 0;
    ChargeCost = 0;
    ElectCost = 0;
    ChargeTime = 0;
    TotalElect = 0;
    ServiceCost = 0;
}
bool ChargePort::on()
{
    for (;;)
    {
        if (CPlock.try_lock())
        {
            break;
        }
    }
    OnState = true;
    CPlock.unlock();
    return 1;
}
bool ChargePort::off()
{
    for (;;)
    {
        if (CPlock.try_lock())
        {
            break;
        }
    }
    if (IsCharging || WaitCount != 0) // 有车在充电区时失败
    {
        return 0;
    }
    OnState = false;
    CPlock.unlock();
    return 1;
}
CPStatusTable ChargePort::GetStatus()
{
    CPStatusTable a = NULLStatusTable;
    a.SID = SID;                   // 充电桩编号
    a.IsFastCharge = IsFastCharge; // 快充还是慢充，快充为1,慢充为0
    a.OnState = OnState;           // 开关状态，true开false关
    a.IsCharging = IsCharging;     // 为 true 则充电区有车
    a.WaitNum = WaitNum;           // 等待区车辆上限
    a.WaitCount = WaitCount;       // 等待区当前车辆数
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
    for (;;)
    {
        if (CPlock.try_lock())
        {
            break;
        }
    }
    if (IsCharging)
        return ChargingCarReply;
    else
    {
        NULLCarReply.Ask.StWaitTime = time(NULL);
        return NULLCarReply;
    }
    CPlock.unlock();
}
bool ChargePort::AddCar(CarReply myreply, Car *mycar)
{
    for (;;)
    {
        if (CPlock.try_lock())
        {
            break;
        }
    }
    if (IsCharging)
    {
        if (WaitNum <= WaitCount)
        {
            return false;
        }
        else
        {
            ++WaitCount;
            WaitingCar.push_back(mycar);
            WaitingCarReply.push_back(myreply);
        }
    }
    else
    {
        IsCharging = true;
        ChargingCar = mycar;
        ChargingCarReply = myreply;

        //将一个新的充电过程放到充电线程池。
        for (;;)
        {
            if (Chargelock.try_lock())
            {
                break;
            }
        }
        ChargeThreadPool *nextth = new ChargeThreadPool;
        nextth->isAvailable = false;
        nextth->ChargePortPtr = this;
        nextth->CarPtr = mycar;
        nextth->RepPtr = &(this->ChargingCarReply);
        nextth->ElectReq = myreply.Ask.ChargeCap;
        ChargeTail->next = nextth;
        ChargeTail->isAvailable = true;
        ChargeTail = nextth;
        Chargelock.unlock();
    }
    CPlock.unlock();
    return true;
}
bool ChargePort::DeleteCar(Car *mycar)
{
    for (;;)
    {
        if (CPlock.try_lock())
        {
            break;
        }
    }
    int id = -1;
    for (int i = 0; i < WaitCount; ++i)
    {
        if (WaitingCar[i] == mycar)
        {
            id = i;
            break;
        }
    }
    if (id != -1)
    {
        for (int i = id; i < WaitCount - 1; ++i)
        {
            WaitingCar[i] = WaitingCar[i + 1];
        }
        WaitingCar.pop_back();
        for (int i = id; i < WaitCount - 1; ++i)
        {
            WaitingCarReply[i] = WaitingCarReply[i + 1];
        }
        WaitingCarReply.pop_back();
        --WaitCount;
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
        //把下一个waiting放入charging
        if (WaitCount != 0)
        {
            PutWaitingToCharging();
        }
        else
        {
            IsCharging = false;
        }
        return true;
    }
    CPlock.unlock();
    return false;
}
void ChargePort::PutWaitingToCharging()
{
    for (;;)
    {
        if (Chargelock.try_lock())
        {
            break;
        }
    }

    ChargingCar = WaitingCar[0];
    ChargingCarReply = WaitingCarReply[0];
    IsCharging = true;

    ChargeThreadPool *nextth = new ChargeThreadPool;
    nextth->isAvailable = false;
    nextth->ChargePortPtr = this;
    nextth->CarPtr = ChargingCar;
    nextth->RepPtr = &(this->ChargingCarReply);
    nextth->ElectReq = this->ChargingCarReply.Ask.ChargeCap;
    ChargeTail->next = nextth;
    ChargeTail->isAvailable = true;
    ChargeTail = nextth;

    for (int i = 0; i < WaitCount - 1; ++i)
    {
        WaitingCar[i] = WaitingCar[i + 1];
    }
    WaitingCar.pop_back();
    for (int i = 0; i < WaitCount - 1; ++i)
    {
        WaitingCarReply[i] = WaitingCarReply[i + 1];
    }
    WaitingCarReply.pop_back();
    --WaitCount;

    Chargelock.unlock();
}
