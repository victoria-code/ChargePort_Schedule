/*
 * @Author: yummy
 * @Date: 2022-06-08 17:35:24
 * @LastEditors: yummy
 * @LastEditTime: 2022-06-12 17:07:28
 * @Description: 请填写简介
 */
#include "User.h"
#include "client.h"
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
    int printRecv(int cmd);
};
