#ifndef __CAR_H__
#define __CAR_H__

#include <ctime>
#include <string>
using namespace std;

struct CarAsk //汽车充电请求
{
    int UID;           // 用户ID
    int CarID;         // 车辆ID
    int ChargeCap;     // 请求充电量
    bool IsFastCharge;  //快充还是慢充，快充为1,慢充为0
    double BatteryCap; // 电池容量
    time_t StWaitTime; // 开始等待时间
};
struct CarReply //汽车充电响应
{
    CarAsk Ask;
    int SID; // 充电桩ID
    int num; // 在该充电桩的排队号
    
    string queueNum;   //排队号(e.g. F1, S2)
    int waitingNum;    //前车等待数量
};
struct ChargeInfo   //用户端查看的充电详情
{
    int SID;    //充电桩ID
    int ChargeMode; //充电模式
    int time;   //充电时长(秒)
    double cap; //充电电量
    double pay; //充电费用
};

class Car // 车辆
{
private:
    int UID;           // 用户ID
    int CarID;         // 车辆ID
    int CarName;       // 车辆名称
    double BatteryCap = 0.0; // 电池容量
    CarAsk *Ask = nullptr;       // 为NULL时没有ask
    CarReply *Reply = nullptr;   // 为NULL时没有reply
    ChargeInfo *info = nullptr;  //充电详情
public:
    ~Car(){
        if(this->Ask != nullptr)
            delete this->Ask;
        if(this->Reply != nullptr)
            delete this->Reply;
    }
    //setters
    void setBatteryCap(double cap){
        this->BatteryCap = cap;
    }
    void setAsk(CarAsk *ask){
        this->Ask = ask;
    }
    void setReply(CarReply *reply){
        this->Reply = reply;
    }
    void setChargeInfo(ChargeInfo *info){
        this->info = info;
    }

    //getters
    double getBatteryCap(){
        return this->BatteryCap;
    }
    CarAsk *getCarAsk(){
        return this->Ask;
    }
    CarReply *getCarReply(){
        return this->Reply;
    }
    ChargeInfo *getChargeInfo(){
        return this->info;
    }
};

#endif
