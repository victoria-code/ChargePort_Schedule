#include<vector>
#include<iostream>
#include<string>
#include<utility>


#define MAX_WAIT_NUM 6  //等候区最大车位容量
#define FAST_POWER 30   //快充功率（单位：度/小时）
#define SLOW_POWER 7    //慢充功率（单位：度/小时）

//用户客户端对服务器请求cmd编号（可自加）
#define QUEUE_NUM_GET  100  //车辆排队号码生成
#define SCHEDULE_GET   101  //调度策略生成
#define GET_BILL       102  //获取计费信息


//管理员客户端对服务器请求cmd编号（可自加）
#define SPY_CHARGEPORT  201 //充电桩信息监控
#define DATA_STATISTICS 202 //数据统计


using namespace std;
