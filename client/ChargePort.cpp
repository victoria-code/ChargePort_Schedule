#include "ChargePort.h"

const double HIGH_ELEC_PRICE = 1.0;   // ���ۣ���λԪ/��
const double LOW_ELEC_PRICE = 0.4;    // �ȵ�ۣ���λԪ/��
const double NORMAL_ELEC_PRICE = 0.7; // ƽ��ۣ���λԪ/��
const double SERVICE_PRICE = 0.8;     // ����ۣ���λԪ/��
const double FAST_CHARGE_RATE = 30;   // ��书�ʣ���λ��/Сʱ
const double SLOW_CHARGE_RATE = 7;    // ���书�ʣ���λ��/Сʱ

CPStatusTable NULLStatusTable; // ��״̬��
CarReply NULLCarReply;         // �ճ������
ChargeThreadPool* ChargeHead;  // ��ʼ����������ͷ
ChargeThreadPool* ChargeTail;  // ��ʼ����������β
int ChargeTableID = 0;         // �������ɳ���굥ID
// StopThreadPool *StopHead;      // �����������ض���ͷ
// StopThreadPool *StopTail;      // �����������ض���β

ChargeTablePool* ChargeTableHead; // �ظ�����굥�������ͷ
ChargeTablePool* ChargeTableTail; // �ظ�����굥�������β

std::mutex Chargelock;      // ��ӳ����̵��̳߳صĻ�����
std::mutex ChargeTablelock; // ��ӳ���굥������صĻ�����

int ChargeProc(ChargePort* ChargePortPtr, Car* CarPtr, CarReply* RepPtr, double ElectReq)
{
    ++ChargePortPtr->ChargeCnt;
    double ElectNow = 0;            // ��ǰ�����
    double ServicePrice = 0;        // ��ǰ�����
    double ElectPrice = 0;          // ��ǰ�ܵ��
    time_t start_time = time(NULL); // ��ʼʱ��
    RepPtr->StChargeTime = start_time;
    time_t time_now = start_time;       // ��ǰʱ��
    double unitrate = SLOW_CHARGE_RATE; // �������

    double start_ChargeCost = ChargePortPtr->ChargeCost;   // �������׮ԭ����
    double start_ChargeTime = ChargePortPtr->ChargeTime;   // �������׮ԭ����
    double start_TotalElect = ChargePortPtr->TotalElect;   // �������׮ԭ����
    double start_ServiceCost = ChargePortPtr->ServiceCost; // �������׮ԭ����
    double start_ElectCost = ChargePortPtr->ElectCost;     // �������׮ԭ����
    double start_BatteryNow = CarPtr->BatteryNow;          // ��������ԭ����

    if (ChargePortPtr->IsFastCharge)
        unitrate = FAST_CHARGE_RATE;

    while (ElectNow < ElectReq &&
        CarPtr->BatteryNow != CarPtr->BatteryCap &&
        !ChargePortPtr->stopCharging)
    {
        // Sleep(3); // ÿ3�����һ�����ݸ��¡�
        Sleep(3000); // ÿ3�����һ�����ݸ��¡�
        time_now = time(NULL);
        int time = time_now - start_time;
        // �����ʱ���������ˣ����Ժ��ڵ�����ÿ1s��Ϊ1min���㡣
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

    // ����һ�ų���
    CostTable costtable{ ++ChargeTableID,
                        time(NULL),
                        ChargePortPtr->SID,
                        ChargePortPtr->IsFastCharge, // ���ģʽ
                        CarPtr->usrname,
                        start_time,
                        time_now,
                        ElectNow,
                        time_now - start_time,
                        ElectPrice + ServicePrice,
                        ServicePrice,
                        ElectPrice };
    for (;;)
    {
        if (ChargeTablelock.try_lock())
        {
            break;
        }
    }
    ChargeTablePool* nextct = new ChargeTablePool;
    nextct->isAvailable = false;
    nextct->ChargeTable = costtable;
    ChargeTableTail->next = nextct;
    ChargeTableTail->isAvailable = true;
    ChargeTableTail = nextct;
    ChargeTablelock.unlock();
    // ������StopCharging��ֱ�ӽ�����deleteCar�������waiting��charging
    if (ChargePortPtr->stopCharging)
    {
        ChargePortPtr->stopCharging = false;
        return 0;
    }
    //����һ��waiting����charging
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
            ChargeThreadPool* next = ChargeHead->next;
            delete ChargeHead;

            // ���̳߳���ȡ��������һ������߳�
            std::thread CP(ChargeProc, next->ChargePortPtr, next->CarPtr, next->RepPtr, next->ElectReq);
            CP.detach(); // �����̷߳����ȥ�����������߳������������server�ر�ʱ���߳�Ҳ�����

            ChargeHead = next;
            Chargelock.unlock();
        }
    }
}
void BuildChargePortThread()
{
    // ��������ģ������̴߳���һЩ��ʼ��Ҳ���������
    // ��ʼ��һ���յĳ��׮״̬��
    NULLStatusTable.SID = NULLStatusTable.ChargeCnt = 0;
    NULLStatusTable.OnState = NULLStatusTable.IsCharging = false;
    NULLStatusTable.WaitCount = 0;
    NULLStatusTable.WaitNum = 1;
    NULLStatusTable.IsFastCharge = false;
    NULLStatusTable.ChargeCost = NULLStatusTable.ChargeTime = 0;
    NULLStatusTable.TotalElect = NULLStatusTable.ServiceCost = NULLStatusTable.ElectCost = 0;
    NULLStatusTable.TableTime = time(NULL);
    // ��ʼ��һ���յ�������������Ӧ
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

    std::thread Cth(ChargeThread); // ��������߳�
    Cth.detach();                  // �����̷߳����ȥ��server�ر�ʱ���߳�ͬʱ����
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
    if (IsCharging || WaitCount != 0) // �г��ڳ����ʱʧ��
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
    a.SID = SID;                   // ���׮���
    a.IsFastCharge = IsFastCharge; // ��仹�����䣬���Ϊ1,����Ϊ0
    a.OnState = OnState;           // ����״̬��true��false��
    a.IsCharging = IsCharging;     // Ϊ true �������г�
    a.WaitNum = WaitNum;           // �ȴ�����������
    a.WaitCount = WaitCount;       // �ȴ�����ǰ������
    a.ChargeCnt = ChargeCnt;       // �ۼƳ�����
    a.ChargeCost = ChargeCost;     // �ۼ��ܷ��á���λ/Ԫ
    a.ChargeTime = ChargeTime;     // �ۼƳ��ʱ������λ/min
    a.TotalElect = TotalElect;     // �ۼ��ܳ��������λ/��
    a.ServiceCost = ServiceCost;   // �ۼƷ�����á���λ/Ԫ
    a.ElectCost = ElectCost;       // �ۼƳ����á���λ/Ԫ
    a.TableTime = time(NULL);      // ״̬��Ϣ����ʱ��
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
bool ChargePort::AddCar(CarReply myreply, Car* mycar)
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

        //��һ���µĳ����̷ŵ�����̳߳ء�
        for (;;)
        {
            if (Chargelock.try_lock())
            {
                break;
            }
        }
        ChargeThreadPool* nextth = new ChargeThreadPool;
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
bool ChargePort::DeleteCar(Car* mycar)
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
        for (;;) //�ȴ�stopCharging��־���޸ģ����ڵ�charging����
        {
            if (!stopCharging)
                break;
        }
        //����һ��waiting����charging
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

    ChargeThreadPool* nextth = new ChargeThreadPool;
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
