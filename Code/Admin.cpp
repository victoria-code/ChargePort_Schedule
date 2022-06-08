#include "Admin.h"
//管理员注销
//todo 我以为注销是登出 看起来user那边写的不是登出是删除账户
int Admin::deleteAccount(){ 
    int suc = 0;
    cout << "==========注销界面=========" << endl;
    cout << "您确定要注销吗？" << endl;
    string choice[] = {"确定","取消"};
    printChoice(choice,2);
    string id;
    while (getline(cin, id), !isLegalChoice(id, 2))
    {
        cout << "请输入有效选项: ";
        printChoice(choice, 2);
    }
    if(id == "1")
        suc = sendDeleteRequest(this->usrname);
    if(suc)
        cout << "注销成功！" << endl;
    else cout << "注销失败，请稍后再试" << endl;
}

//打开充电桩（可选择）
//返回值：true打开成功 false打开失败
bool Admin::openChargePort(int SID){
    int suc = 0;
    cout << "==========打开充电桩=========" << endl;
    cout << "请选择要打开的充电桩" << endl;
    string choice[] = {"充电桩1","充电桩2","充电桩3","充电桩4","所有充电桩"};
    printChoice(choice,5);
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "请输入有效选项: ";
        printChoice(choice, 5);
    }
    int id = id_str[0] - '0';
    string info;
    switch(id){
        case 1:
        case 2:
        case 3:
        case 4:
            info = id_str;
            break;
        case 5:
            info = "all";
            break;
    }
    suc = sendRequest(usrname, OPEN_CHARGEPORT, info);
    if (suc){
        cout <<"充电桩开启成功" << endl;
        return true;
    }
    else cout << "充电桩开启失败" << endl;
    return false;
    //todo 充电桩已开启
}

//关闭充电桩（可选择）
//返回值：true关闭成功 false关闭失败
bool Admin::closeChargePort(int SID){
    int suc = 0;
    cout << "==========关闭充电桩=========" << endl;
    cout << "请选择要关闭的充电桩" << endl;
    string choice[] = {"充电桩1","充电桩2","充电桩3","充电桩4","所有充电桩"};
    printChoice(choice,5);
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "请输入有效选项: ";
        printChoice(choice, 5);
    }
    int id = id_str[0] - '0';
    string info;
    switch(id){
        case 1:
        case 2:
        case 3:
        case 4:
            info = id_str;
            break;
        case 5:
            info = "all";
            break;
    }
    suc = sendRequest(usrname, CLOSE_CHARGEPORT, info);
    if (suc){
        cout <<"充电桩关闭成功" << endl;
        return true;
    }
    else cout << "充电桩关闭失败" << endl;
    return false;
    //todo 充电桩已关闭
}

//查询充电桩信息(充电桩状态)
string Admin::getChargePortInfo(){
    int suc = 0;
    cout << "==========查询充电桩信息=========" << endl;
    cout << "请选择要查询的充电桩" << endl;
    string choice[] = {"充电桩1","充电桩2","充电桩3","充电桩4","所有充电桩"};
    printChoice(choice,5);
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "请输入有效选项: ";
        printChoice(choice, 5);
    }
    int id = id_str[0] - '0';
    string info;
    switch(id){
        case 1:
        case 2:
        case 3:
        case 4:
            info = id_str;
            break;
        case 5:
            info = "all";
            break;
    }

    //和服务器发送请求 todo
    suc = sendReportRequest(usrname, CLOSE_CHARGEPORT, info);
    //获得信息和输出
}
//查询等候服务车辆信息
string Admin::getAllWaitingCarInfo(){

}

//发送服务器请求
//1表示请求成功 0表示请求失败 2表示充电桩已开启 3表示充电桩已关闭
//socket todo
int Admin::sendRequest(string usrname,int cmd,string request){
}

//发送服务器请求
//返回查询信息
//socket todo
int Admin::sendReportRequest(string usrname,int cmd,string request){
}