#include "main.h"
class Server{
    public:
        /*with chargePort*/
        //当前等候区内正在等候的快充/慢充车辆数目
        int FNum,TNum;
        //当前等候区内即将进入充电区的车辆编号
        int fSeq,tSeq;
        
        /*with admin*/
        //所有管理员账户,用于验证管理员身份
        vector<string>admin;

        //记录充电桩状态

        //车辆排队号码生成
        string queueNumGenerate(string usrname,int mode);

        //调度策略生成，返回充电桩编号
        int schedule(string usrname,int mode,int amount);

        //费用计算，返回指定用户需要支付的充电费用,time为实际充电时间
        int recordBill(string usrname,int mode,int time);

        //用户信息维护：info格式：<“被修改属性名”，“修改后属性值”>】
        bool usrDataMaintain(string usrname,vector<pair<string,string>> info);

        //充电桩信息监控:获取所有充电桩的相关信息
        int spyChargePort();

        //响应用户客户端请求
        int replyUser(string usrname,int cmd);

        //响应管理员客户端请求
        int replyAdmin(string usrname,int cmd);

};

