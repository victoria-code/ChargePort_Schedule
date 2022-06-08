#include"User.h"

class Admin : public User
{
private:

public:
    //查询充电桩信息(充电桩状态+等候服务车辆信息)
    int getChargePortData();

    //开启充电桩
    bool openChargePort(int SID);

    //关闭充电桩
    bool closeChargePort(int SID);

    //向服务器发送请求
    int sendRequest(string usrname,int cmd,string info);
    
};
