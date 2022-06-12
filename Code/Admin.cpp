#include "Admin.h"

//登录界面
void Admin::Menu(){
    while (true)
	{
		cout << "**************************************************" << endl;
		cout << "******************  1.开启充电桩 *******************" << endl;
		cout << "******************  2.关闭充电桩 *******************" << endl;
		cout << "******************  3.查询充电状态 *******************" << endl;
		cout << "******************  4.退出登录 *******************" << endl;
		cout << "******************  5.注销账号 *******************" << endl;
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
			this->clearData();
			break;
		}
		case 5:
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
bool Admin::openChargePort(){
    int suc = 0;
    cout << "==========打开充电桩=========" << endl;
    cout << "请选择要打开的充电桩编号，全部打开请回复0" << endl;
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "请输入有效选项: ";
    }
    int id = id_str[0] - '0' - 1;
    suc = sendRequest(usrname, OPEN_CHARGEPORT, id);
    if (suc){
        cout << recv_info.output << endl;
        return true;
    }
    else cout << recv_info.output << endl;
    return false;
    //todo 充电桩已开启
}

//关闭充电桩（可选择）
//返回值：true关闭成功 false关闭失败
bool Admin::closeChargePort(){
    int suc = 0;
    cout << "==========关闭充电桩=========" << endl;
    cout << "请选择要打开的充电桩编号，全部打开请回复0" << endl;
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "请输入有效选项: ";
    }
    int id = id_str[0] - '0' - 1;
    suc = sendRequest(usrname, CLOSE_CHARGEPORT, id);
    if (suc){
        cout << recv_info.output << endl;
        return true;
    }
    else cout << recv_info.output << endl;
    return false;
}

//查询充电桩信息(充电桩状态)
bool Admin::getChargePortInfo(){
    int suc = 0;
    cout << "==========查询充电信息=========" << endl;
    string choice[] = {"查询充电桩状态","查询排队车辆信息","查询报表"};
    printChoice(choice,3);
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "请输入有效选项: ";
        printChoice(choice, 3);
    }
    int id = id_str[0] - '0';
    string info;
    switch(id){
        case 1:
            suc = sendReportRequest(usrname, SPY_CHARGEPORT);
            if (suc){
                printRecv(SPY_CHARGEPORT);
                return true;
            }
            else cout << "信息查询失败，请稍后重试" << endl;
            return false;
            break;
        case 2:
            suc = sendReportRequest(usrname, WAIT_CAR_DATA);
            if (suc){
                printRecv(WAIT_CAR_DATA);
                return true;
            }
            else cout << "信息查询失败，请稍后重试" << endl;
            return false;
            break;
        case 3:
            suc = sendReportRequest(usrname, DATA_STATISTICS);
            if (suc){
                printRecv(DATA_STATISTICS);
                return true;
            }
            else cout << "信息查询失败，请稍后重试" << endl;
            return false;
            break;
    }
}

//发送服务器请求
//1表示请求成功 0表示请求失败 2表示充电桩已开启 3表示充电桩已关闭 返回内容存在于recv_info里
int Admin::sendRequest(string usrname,int cmd,int number){
    send_info.cmd = cmd;
    strcpy(send_info.UID, usrname.c_str());
    send_info.REPLY = number;
    client_sock.Send(send_info);
    client_sock.Recv(recv_info);
    return recv_info.REPLY;
}

//发送查询请求
int Admin::sendReportRequest(string usrname,int cmd){
    send_info.cmd = cmd;
    strcpy(send_info.UID, usrname.c_str());
    client_sock.Send(send_info);
    client_sock.Recv(recv_info);
}

//打印返回信息内容
int Admin::printRecv(int cmd){
    string recv(recv_info.output);
    switch(cmd){
        case SPY_CHARGEPORT:
            cout << "充电桩状态如下" << endl;
            cout << recv;
            break;
        case WAIT_CAR_DATA:
            cout << "充电车辆信息如下" << endl;
            cout << recv;
            break;
        case DATA_STATISTICS:
            cout << "报表信息如下" << endl;
            cout << recv;
            break;
    }

}