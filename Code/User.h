#ifndef __USER_H__
#define __USER_H__

#include<iostream>
#include<string>

using namespace std;

class User{     //用户基类
protected:
//test
//public:
    string usrname;     //用户名

public:
    //用户登录
    int logIN();
    int sendLogInRequest(string usrname, string password);  //发送登录请求

    //用户注册
    int signUp();
    int sendSignUpRequest(string usrname, string password); //发送注册请求

    //用户注销
    int deleteAccount();
    int sendDeleteRequest(string usrname);    //发送注销请求

    /*------Utils------*/
    //判断充值金额合法性
    bool isPosNum(string str);
    //判断输入是否是合法选项(max: 最大选项序号)
    bool isLegalChoice(string str, int max);
};

class Customer : public User
{
private:
//test
//public:
    double balance = 0.0;    //用户余额

public:
    //充值
    int recharge();
    int setUsrBalance(double num);  //向服务器发送余额更新数据(num:新的余额)
    //扣费
    int deduct();

    //提交充电请求
    int chargeRequest();
    int sendChargeRequest(int mode, int charge);    //发送充电请求

    //修改充电请求
    //※复用chargeRequest()?
    //int changeChargeRequest();

    //查看排队结果
    int getQueueRes();
    int sendGetQueueRequest();
    int sendGetWaitingRequest();

    //(充电结束后)查看充电详情
    string getChargeInfo();

};

// 管理员类：请在Admin.h中定义
// class Admin : public User
// {
// private:

// public:
//     //管理员方法
// };

#endif
