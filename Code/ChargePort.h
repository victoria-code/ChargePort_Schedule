#include<vector>
#include<list>
#include "Car.h"
class ChargePort{//充电桩
private:
    int SID; // 充电桩编号
    string type; //快充还是慢充，快充为F,慢充为T
    std::list<CarReply*>WaitingCar; //当前等待处理的充电响应
    bool OnState; // 开关状态，1开0关
    int ChargeCnt; // 累计充电次数
    double ChargeCost; // 累计充电费用。单位/元
    double ChargeTime; // 累计充电时长。单位/min
    double TotalCharge; // 累计总充电量。单位/度
    double ServiceCost; //累计服务费用。单位/元
public:
    StatusTable GetStatus();//返回充电桩状态
    
};
