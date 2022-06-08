#include "User.h"
#include "main.h"
#include "ChargePort.h"

class Admin : public User
{
private:

public:
    Admin(){
        this->type = ADMIN;
    }

    /*with chargePort*/
    //查询充电桩信息(充电桩状态)
    string getChargePortInfo();
    //查询等候服务车辆信息
    string getAllWaitingCarInfo();

    //开启充电桩
    bool openChargePort(int SID);

    //关闭充电桩
    bool closeChargePort(int SID);

    //向服务器发送请求
    int sendRequest(string usrname, int cmd, string request); 
    int sendReportRequest(string usrname, int cmd, string request);
    //用户注销
    int deleteAccount();
};
