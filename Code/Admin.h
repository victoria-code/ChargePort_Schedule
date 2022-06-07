#include"user.h"

class Admin : public User
{
private:

public:
    //查询充电桩信息(充电桩状态+等候服务车辆信息)
    int getChargePortData();

    //开启充电桩(一次性全部开启)
    bool openChargePort();

    //关闭充电桩
    bool closeChargePort();

    //向服务器发送请求
    int sendRequest(string usrname,int cmd,string info);
    
};
