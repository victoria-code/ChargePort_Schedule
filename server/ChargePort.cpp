#include "ChargePort.h"

const double HIGH_ELEC_PRICE = 1.0;   // 峰电价，单位元/度
const double LOW_ELEC_PRICE = 0.4;    // 谷电价，单位元/度
const double NORMAL_ELEC_PRICE = 0.7; // 平电价，单位元/度
const double SERVICE_PRICE = 0.8;     // 服务价，单位元/度
const double FAST_CHARGE_RATE = 30;   // 快充功率，单位度/小时
const double SLOW_CHARGE_RATE = 10;   // 慢充功率，单位度/小时

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

const int CHANGE_SIZE = 8;                          // 类型转化的时间集合的大小
int TimeChange[8] = {0, 7, 10, 15, 18, 21, 23, 24}; // 类型转化时间
int ChangeKind[8] = {0, 1, 2, 1, 2, 1, 0, 0};       // 每个TimeChage时间对应的的转化类型 0谷1平2峰
double KindPrice[3] = {0.4, 0.7, 1.0};              // 谷、平、峰电价

int ChargeProc(ChargePort *ChargePortPtr, Car *CarPtr, CarReply *RepPtr, double ElectReq)
{
    ++ChargePortPtr->ChargeCnt;
    ChargePortPtr->ElectNow = 0;     // 当前充电汽车充电量
    ChargePortPtr->ServicePrice = 0; // 当前充电汽车服务价
    ChargePortPtr->ElectPrice = 0;   // 当前充电汽车总电价
    time_t start_time = time(NULL);  // 起始时间

    time_t start_timeAC = (start_time - ChargePortPtr->FiveTime) * 60 + ChargePortPtr->ActualFiveTime; //起始时间转化为换算后时间
    RepPtr->StChargeTime = start_timeAC;
    time_t time_now = start_time;       // 当前时间
    time_t time_nowAC;                  //当前时间转化为换算之后的时间
    double unitrate = SLOW_CHARGE_RATE; // 充电速率，单位时间电量
    if (ChargePortPtr->IsFastCharge)
        unitrate = FAST_CHARGE_RATE;
    double start_ChargeCost = ChargePortPtr->ChargeCost;   // 保留充电桩原数据
    double start_ChargeTime = ChargePortPtr->ChargeTime;   // 保留充电桩原数据
    double start_TotalElect = ChargePortPtr->TotalElect;   // 保留充电桩原数据
    double start_ServiceCost = ChargePortPtr->ServiceCost; // 保留充电桩原数据
    double start_ElectCost = ChargePortPtr->ElectCost;     // 保留充电桩原数据
    double start_BatteryNow = CarPtr->BatteryNow;          // 保留汽车原数据

    std::cout << "User " << RepPtr->Ask.usrname << " is charging" << std::endl;

    while (ChargePortPtr->ElectNow < ElectReq &&
           CarPtr->BatteryNow != CarPtr->BatteryCap &&
           !ChargePortPtr->stopCharging)
    {
        Sleep(500); // 每500ms进行一次数据更新。
        time_now = time(NULL);
        int time = (time_now - start_time) * 60;                    // 这里的时间计算加速了，可以后期调整，每1s作为60s计算。
        time_nowAC = start_timeAC + time;                           //当前时间转化为换算之后的时间
        ChargePortPtr->ElectNow = unitrate * (double)time / 3600.0; // time/3600是从s换算成h

        /*
        计算price
        */
        double price = 0;                //计算电价
        int id = 0;                      //计算当前所在时间区间序号
        time_t tt = start_timeAC;        //每个时间块的起始时间
        time_t last_tt = tt;             //上一个时间块的起始时间
        struct tm *ttm = localtime(&tt); //时间块起始时间的tm格式
        for (;;)
        {
            //寻找当前时间所在的时间块
            id = 0;
            for (int i = 0; i < CHANGE_SIZE - 1; ++i)
            {
                if (ttm->tm_hour >= TimeChange[i] && ttm->tm_hour < TimeChange[i + 1])
                {
                    id = i;
                    break;
                }
            }
            last_tt = tt;
            if (TimeChange[id + 1] != 24)
            {
                ttm->tm_hour = TimeChange[id + 1];
                ttm->tm_min = 0;
                ttm->tm_sec = 0;
                tt = mktime(ttm);
            }
            else //若为24点则加一天
            {
                ttm->tm_hour = 23;
                ttm->tm_min = 59;
                ttm->tm_sec = 59;
                tt = mktime(ttm);
                tt = tt + 1;
                ttm = localtime(&tt);
            }
            if (tt >= time_nowAC)
            {
                break;
            }
            else
            {
                price += KindPrice[ChangeKind[id]] * unitrate * double(tt - last_tt) / 3600.0; // time/3600是从s换算成h
            }
        }
        price += KindPrice[ChangeKind[id]] * unitrate * double(time_nowAC - last_tt) / 3600.0; // time/3600是从s换算成h
        /*if (price < 0)
        {
            std::cout << price << ' ' << ' ' << KindPrice[ChangeKind[id]] << ' ' << unitrate << ' ' << timeAC - last_tt << std::endl;
        }*/
        // 把 price 更新到 ChargePortPtr->ElectPrice
        ChargePortPtr->ElectPrice = price;

        // 计算服务费
        ChargePortPtr->ServicePrice = SERVICE_PRICE * ChargePortPtr->ElectNow;

        ChargePortPtr->ChargeTime = start_ChargeTime + time;
        ChargePortPtr->TotalElect = start_TotalElect + ChargePortPtr->ElectNow;
        ChargePortPtr->CurElectReq = CarPtr->BatteryCap - CarPtr->BatteryNow;
        if (ElectReq - ChargePortPtr->ElectNow < CarPtr->BatteryCap - CarPtr->BatteryNow)
            ChargePortPtr->CurElectReq = ElectReq - ChargePortPtr->ElectNow;
        ChargePortPtr->ElectCost = start_ElectCost + ChargePortPtr->ElectPrice;
        ChargePortPtr->ServiceCost = start_ServiceCost + ChargePortPtr->ServicePrice;
        ChargePortPtr->ChargeCost = ChargePortPtr->ServiceCost + ChargePortPtr->ElectCost;
        if (start_BatteryNow + ChargePortPtr->ElectNow < CarPtr->BatteryCap)
            CarPtr->BatteryNow = start_BatteryNow + ChargePortPtr->ElectNow;
        else
        {
            CarPtr->BatteryNow = CarPtr->BatteryCap;
            break;
        }
    }

    /* 输出结束充电的时间和充电量和价格
    tm *ttmAC = localtime(&timeAC);
    std::cout << ttmAC->tm_year + 1900 << '/';
    std::cout << ttmAC->tm_mon + 1 << '/';
    std::cout << ttmAC->tm_mday << "  ";
    std::cout << ttmAC->tm_hour << ':' << ttmAC->tm_min << ':' << ttmAC->tm_sec << std::endl;

    std::cout << ChargePortPtr->ElectNow << ' ' << ChargePortPtr->ServicePrice << ' ' << ChargePortPtr->ElectPrice << std::endl;
    */

    // 更新一张充电表
    CostTable costtable{++ChargeTableID,
                        time(NULL),
                        ChargePortPtr->SID,
                        ChargePortPtr->IsFastCharge, // 充电模式
                        CarPtr->usrname,
                        start_timeAC,
                        time_nowAC,
                        ChargePortPtr->ElectNow,
                        time_nowAC - start_timeAC,
                        ChargePortPtr->ElectPrice + ChargePortPtr->ServicePrice,
                        ChargePortPtr->ServicePrice,
                        ChargePortPtr->ElectPrice};
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
        if (ChargePortPtr->stopCharging)
        {
            ChargePortPtr->stopCharging = false;
            return 0;
        }
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
    ChargePortPtr->CPlock.unlock();
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
    std::cout << "ChargeHead is initialed here\n";
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
ChargePort::ChargePort(int CPID, bool fast, bool on, time_t FiveT, int WNum, int CCnt, double CCost, double ECost, long long CTime, double TElect, double SCost)
{
    this->SID = CPID;
    this->IsFastCharge = fast;
    this->OnState = on;
    FiveTime = FiveT;
    ActualFiveTime = 1656104400; // 2022/6/25 05:00:00 对应的time_t
    WaitNum = WNum;
    WaitCount = 0;
    IsCharging = false;
    ChargeCnt = CurElectReq = 0;
    ChargingCarReply = NULLCarReply;
    WaitingCarReply.clear();
    ChargingCar = NULL;
    WaitingCar.clear();

    ElectNow = 0;
    ServicePrice = 0;
    ElectPrice = 0;

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
        CPlock.unlock();
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
    {
        CPlock.unlock();
        return ChargingCarReply;
    }
    else
    {
        NULLCarReply.Ask.StWaitTime = time(NULL);

        CPlock.unlock();
        return NULLCarReply;
    }
    CPlock.unlock();
}
bool ChargePort::AddCar(CarReply myreply, Car *mycar)
{
    std::cout << "addCar here" << std::endl;
    for (;;)
    {
        std::cout << mycar->usrname << " asking....\n";
        if (CPlock.try_lock())
        {
            break;
        }
    }
    if (IsCharging)
    {
        if (WaitNum <= WaitCount)
        {
            CPlock.unlock();
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
    std::cout << mycar->usrname << " add car ...done" << std::endl;
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

        CPlock.unlock();
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
        CPlock.unlock();
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
