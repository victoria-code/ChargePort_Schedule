#include <ctime>
struct CarAsk //汽车充电请求
{
    int UID; // 用户ID
    int CarID; // 车辆ID
    int ChargeCap; // 请求充电量
    bool IsFastCharge; //快充还是慢充，快充为1
    double BatteryCap; // 电池容量
    time_t StWaitTime; // 开始等待时间
};
struct CarReply //汽车充电响应
{
    CarAsk Ask;
    int SID; // 充电桩ID
    int num; // 在该充电桩的排队号
};
class Car // 车辆
{
private:
    int UID; // 用户ID
    int CarID; // 车辆ID
    int CarName; // 车辆名称
    double BatteryCap; // 电池容量
    bool IsAsk; // 是否请求充电，1为是
    CarAsk* Ask;
    bool IsReply; // 是否获得reply，1为是
    CarReply* Reply;
public:

}
