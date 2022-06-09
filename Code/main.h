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
#define FAST 0
#define SLOW 1

//用户客户端对服务器请求cmd编号（可自加）
#define LOG_IN  100  //登录验证.
#define SIGN_UP 101  //注册认证·
#define Balance_CHANGE 102 //用户充值 or 扣费·
#define DELETE_USER 103//用户注销·
#define CHARGE_REQUEST 104  //提交充电申请
#define CHANGE_REQUEST 105  //取消充电
#define GET_QUEUE_RES 106  //获取排队结果
#define GET_BILL 107  //获取计费信息
#define GET_CHARGE_INFO 108//充电结束后获取充电详情

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

//报文格式
struct Info
{
	int cmd;	//消息类型
	char username[20];	//用户名
	char info[INFO_SIZE];	//具体消息内容
};

extern struct Info send_info, recv_info;
