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


#define MAX_WAIT_NUM 6  //等候区最大车位容量
#define FAST_POWER 30   //快充功率（单位：度/小时）
#define SLOW_POWER 7    //慢充功率（单位：度/小时）

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
#define DETAIL 300	//to 客户 充电完成提示 + 充电详单 + 扣费成功提醒（处理充电请求前检查余额）
#define LOCATION 301 //to 客户 叫号（进入充电区）
#define BREAKDOWN 302 //to 客户 充电过程中充电桩故障提示+充电详单1+扣费成功提醒（重新调度结果发104)

#define REPORT 310 //to admin 报表



//管理员客户端对服务器请求cmd编号（可自加）
#define SPY_CHARGEPORT  200 //充电桩信息监控
#define DATA_STATISTICS 201 //数据统计
#define OPEN_CHARGEPORT 202 //打开所有充电桩
#define CLOSE_CHARGEPORT 203//关闭所有充电桩

//socket连接用的请求编号
#define CLOSE 300 //断开连接

using namespace std;

//最大消息长度
#define INFO_SIZE 500

struct Info
{
	int cmd;	//消息类型
	char UID[20];	//用户名
	char PWD[10];	//密码	复用：服务器返回的报文此字段为“++++++++++”
	int MODE;		//充电模式：0慢1快  复用：用户类型 1Customer  2Admin
	int Q_NUM;		//排队号码
	int W_NUM;		//前车等待数量
	double COST;	//充电费用
	double BatteryCap;	//电池总容量
	double BALANCE;	//余额			复用：充电量
	int REPLY;		//响应报文使用 表示请求结果		复用：充电桩编号
	char output[200]; //服务器返回的内容
};

extern struct Info send_info, recv_info;
