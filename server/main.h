#pragma once

#include<vector>
#include<iostream>
#include<string>
#include<utility>
#include<sstream>
#include<iomanip>
#include<fstream>
#include<map>
#include<iterator>
#include<mutex>

extern std::mutex serverlock;       //服务器操作互斥锁

//可变参数
#define MAX_WAIT_NUM 10  //等候区最大车位容量
#define QUEUE_LENGTH 3  //充电桩排队队列长度
#define FAST_NUM 2		//快充充电桩数目
#define SLOW_NUM 3		//慢充充电桩数目


#define FAST_POWER 30   //快充功率（单位：度/小时）
#define SLOW_POWER 10    //慢充功率（单位：度/小时）

//充电模式mode
#define FAST 1
#define SLOW 0

//用户客户端对服务器请求cmd编号（可自加）
#define LOG_IN  100  //登录验证.
#define SIGN_UP 101  //注册认证·
#define Balance_CHANGE 102 //用户充值 or 扣费·
#define DELETE_USER 103//用户注销·
#define CHARGE_REQUEST 104  //提交充电申请
#define CANCEL_REQUEST 105  //取消充电
#define GET_QUEUE_DATA 106  //获取排队信息

//服务端主动向客户端发送的消息
#define DETAIL 300	//充电完成提醒+充电详单+扣费成功提醒
#define CALL 301	//叫号，客户进入充电区
#define BREAKDOWN 302 //充电桩故障提醒+当前充电详单+扣费成功提醒


//管理员客户端对服务器请求cmd编号（可自加）
#define SPY_CHARGEPORT  200 //充电桩信息监控
#define WAIT_CAR_DATA 201	//查看等候的车辆信息
#define DATA_STATISTICS 202 //查看报表
#define OPEN_CHARGEPORT 203 //打开充电桩
#define CLOSE_CHARGEPORT 204//关闭充电桩

//socket连接用的请求编号
#define CLOSE 88 //断开连接

using namespace std;

//最大消息长度
#define INFO_SIZE 1500

struct Info
{
	int cmd;	//消息类型
	char UID[20];	//用户名
	char PWD[10];	//密码	
	int MODE;		//充电模式：0慢1快  复用：用户类型 1Customer  2Admin
	int Q_NUM;		//排队号码
	int W_NUM;		//前车等待数量
	double COST;	//充电费用	复用：当前电池电量
	double BatteryCap;	//电池总容量
	double BALANCE;	//余额			复用：请求充电量
	int REPLY;		//响应报文使用 表示请求结果		复用：充电桩编号
	char output[1000];//响应结果，直接输出
};

extern struct Info send_info, recv_info;
