/*
 * @Author: yummy
 * @Date: 2022-06-08 17:35:24
 * @LastEditors: yummy
 * @LastEditTime: 2022-06-10 22:52:24
 * @Description: 请填写简介
 */
#include "User.h"
#include "main.h"
#include "TSocket.h"
#include "ChargePort.h"

class Admin : public User
{
private:

public:
    Admin(){
        this->type = ADMIN;
    }

    
    // 登录后的操作界面
    void Menu();
    
    /*with chargePort*/
    //查询充电桩信息(充电桩状态)
    bool getChargePortInfo();
    //查询等候服务车辆信息
    bool getAllWaitingCarInfo();

    //开启充电桩
    bool openChargePort(int SID);

    //关闭充电桩
    bool closeChargePort(int SID);

    //向服务器发送请求
    int sendRequest(string usrname, int cmd, string request); 
    int sendReportRequest(string usrname, int cmd, string request);
    
    //用户注销
    int deleteAccount();

    //处理返回信息
    int printRecv(int cmd);
};
