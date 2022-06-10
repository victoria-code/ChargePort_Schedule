#include "Admin.h"

//登录界面
void Admin::Menu(){
    while (true)
	{
		cout << "**************************************************" << endl;
		cout << "******************  1.开启充电桩 *******************" << endl;
		cout << "******************  2.关闭充电桩 *******************" << endl;
		cout << "******************  3.查询充电桩状态 *******************" << endl;
		cout << "******************  4.查询排队车辆信息 *******************" << endl;
		cout << "******************  5.退出登录 *******************" << endl;
		cout << "******************  6.注销账号 *******************" << endl;
		cout << "**************************************************" << endl;
		cout << "\n请输入您的选择： " << endl;
		int choice;
		cin >> choice;
		switch (choice)
		{
		case 1:
		{
			this->openChargePort();
			break;
		}
		case 2:
		{
			this->closeChargePort();
			break;
		}
		case 3:
		{
			this->getChargePortInfo();
			break;
		}
		case 4:
		{
			this->getAllWaitingCarInfo();
			break;
		}
		case 5:
		{
			this->clearData();
			return;
		}
		case 6:
		{
			this->deleteAccount();
			return;
		}
		default:
			break;
		}
	}
}
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
bool Admin::getChargePortInfo(){
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
    suc = sendReportRequest(usrname, SPY_CHARGEPORT, info);
    if (suc){
        printRecv(SPY_CHARGEPORT);
        return true;
    }
    else cout << "信息查询失败，请稍后重试" << endl;
    return false;
    //获得信息和输出
}
//查询等候服务车辆信息
bool Admin::getAllWaitingCarInfo(){
    // int suc = 0;
    // cout << "==========查询等候服务车辆信息=========" << endl;
    // cout << "请选择要查询的充电桩" << endl;
    // string choice[] = {"充电桩1","充电桩2","充电桩3","充电桩4","所有充电桩"};
    // printChoice(choice,5);
    // string id_str;
    // while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    // {
    //     cout << "请输入有效选项: ";
    //     printChoice(choice, 5);
    // }
    // int id = id_str[0] - '0';
    // string info;
    // switch(id){
    //     case 1:
    //     case 2:
    //     case 3:
    //     case 4:
    //         info = id_str;
    //         break;
    //     case 5:
    //         info = "all";
    //         break;
    // }

    // //和服务器发送请求 todo
    // suc = sendReportRequest(usrname, SPY_CHARGEPORT, info);
    // if (suc){
    //     printRecv(SPY_CHARGEPORT);
    //     return true;
    // }
    // else cout << "信息查询失败，请稍后重试" << endl;
    // return false;
    //获得信息和输出
}

//发送服务器请求
//1表示请求成功 0表示请求失败 2表示充电桩已开启 3表示充电桩已关闭 返回内容存在于recv_info里
//socket todo
int Admin::sendRequest(string usrname,int cmd,string request){
    send_info.cmd = cmd;
    strcpy(send_info.username, usrname.c_str());
    strcpy(send_info.info, request.c_str());
    //client_sock.Send(send_info);
    //client_sock.Send(recv_info);
    //return recv_info.REPLY;
}

int Admin::printRecv(int cmd){
    string recv(recv_info.info);
    //todo 确认recv的内容
    switch(cmd){
        case SPY_CHARGEPORT:
            //todo 与服务器协商返回内容
        case DATA_STATISTICS:
            //todo 与服务器协商返回内容
    }

}