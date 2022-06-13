#ifndef __ADMIN_H__
#define __ADMIN_H__
#pragma once

#include<iostream>
#include<string>

#include "User.h"
#include "client.h"
#include "main.h"
#include "TSocket.h"

using namespace std;

class Admin : public User
{
private:

public:
    Admin(){
        this->type = ADMIN;
    }

    Admin(string name, int t)
    {
        this->usrname = name;
        this->type = t;
    }

    
    // 登录后的操作界面
    void showMenu();
    
    /*with chargePort*/
    //查询充电桩信息(充电桩状态、排队车辆信息、报表)
    bool getChargePortInfo();

    //开启充电桩
    bool openChargePort();

    //关闭充电桩
    bool closeChargePort();

    //向服务器发送请求
    int sendRequest(string usrname, int cmd, int number); 
    int sendReportRequest(string usrname, int cmd);
    
    //用户注销
    int deleteAccount();

    //处理返回信息
    void printRecv(int cmd);
};


#endif