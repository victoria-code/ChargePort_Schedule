#include<vector>
#include<iostream>
#include<string>
#include<utility>


#define MAX_WAIT_NUM 6  //等候区最大车位容量
#define FAST_POWER 30   //快充功率（单位：度/小时）
#define SLOW_POWER 7    //慢充功率（单位：度/小时）

//充电模式mode
#define FAST 0
#define SLOW 1

//用户客户端对服务器请求cmd编号（可自加）
#define LOG_IN  100  //登录验证
#define SIGN_UP 101  //注册认证
#define RECHARGE 102 //用户充值
#define DELETE_USER 103//用户注销
#define CHARGE_REQUEST 104  //提交充电申请
#define CHANGE_REQUEST 105  //修改充电申请
#define QUEUE_NUM_GET 106  //车辆排队号码生成
#define SCHEDULE_GET 107  //调度策略生成
#define GET_QUEUE_RES 108  //获取排队结果
#define END_CHARGE 109  //提前结束充电
#define GET_BILL 110  //获取计费信息
#define DEDUCT 111   //用户扣费

//管理员客户端对服务器请求cmd编号（可自加）
#define SPY_CHARGEPORT  200 //充电桩信息监控
#define DATA_STATISTICS 201 //数据统计
#define OPEN_CHARGEPORT 202 //打开所有充电桩
#define CLOSE_CHARGEPORT 203//关闭所有充电桩

using namespace std;
