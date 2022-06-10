#ifndef __USER_H__
#define __USER_H__
#pragma once

#include "Car.h"    //充电相关

#include<iostream>
#include<string>
#include<math.h>    //floor()

using namespace std;

enum USER_TYPE { ADMIN = 0, CUSTOMER };

class User {     //用户基类
protected:
	//public: //test
	int type;   //用户类型（1：消费者；0：管理员）
	string usrname = "";    //用户名
	double balance = 0.0;   //余额（仅对消费者有意义）
	Car *car = nullptr;    //消费者的车辆

public:
	~User() {
		if (this->car != nullptr)
			delete this->car;
	}

	//用户登录
	//int logIN();
	//int sendLogInRequest(string usrname, string password);  //发送登录请求报文

	////用户注册
	//int signUp();
	//int sendSignUpRequest(string usrname, string password, int type); //发送注册请求报文

	//用户注销
	virtual int deleteAccount() { return 0; }    //由子类实现
	int sendDeleteRequest(string usrname);    //发送注销请求报文

	//用户成功登录后显示的菜单
	virtual void showMenu() { return ; };

	//清空数据 用于退出登录和注销
	void clearData();


	/*------Utils------*/
	//判断充值金额合法性
	bool isPosNum(string str);
	//判断输入是否是合法选项(max: 最大选项序号)
	bool isLegalChoice(string str, int max);
	//输出选项
	void printChoice(const string choice[], int size);
	//获取时间（秒转换为时分秒）
	//string getTime(int sec);
	string getTime(long long sec);
};

class Customer : public User
{
public:
	Customer()
	{
		this->type = CUSTOMER;
	}

	Customer(string name, int t)
	{
		this->usrname = name;
		this->type = t;
	}

	//消费者注销
	int deleteAccount();

	//消费者登录成功后显示的菜单
	void showMenu();

	//充值
	int recharge();
	//int sendRechargeRequest(double amount);     //发送充值请求报文
	//扣费
	int deduct();
	//int sendDeductRequest();    //发送扣费请求(告知服务器可以进行扣费操作)
	int sendUpdateBalanceRequest(double amount);	//发送余额更新报文


	//获取用户余额(仅对消费者有意义)
	double getUsrBalance();
	//获取用户车辆信息（仅对消费者有意义）
	Car *getCarInfo();

	//提交充电请求
	int newChargeRequest();
	int sendChargeRequest();    //发送充电请求报文
	int getChargeStatus();  //获取充电状态（是否正在充电）
	//取消充电
	int cancelCharge();
	int sendCancelRequest();    //发送取消充电的报文

	//查看排队结果
	int getQueueRes();
	int sendQueueInfoRequest();

	//(充电结束后)查看充电详情
	int getChargeInfo();
	int sendChargeInfoRequest();

};

//test
class Admin : public User
{
public:
	Admin() {
		this->type = ADMIN;
	}

	//管理员登录成功后显示的菜单
	void showMenu();
};

#endif
