#include "TSocket.h"	//socket
#include "User.h"
#include "main.h"

extern TSocket client_sock;
extern mutex mutexUsr, mutexSock;

/*
	专门用于接收报文的函数
*/
void Customer::keepRecv()
{
	while (1)
	{
		mutexSock.lock();
		client_sock.Recv(recv_info);
		switch (recv_info.cmd)
		{
		case DETAIL: //充电完成提醒+充电详单+扣费成功提醒
			printf("%s", recv_info.output);
			//是否要保存充电详单
			//更新本地余额信息
			this->balance = recv_info.BALANCE;
			break;
		case CALL: //叫号，客户进入充电区
			printf("%s", recv_info.output);
			break;
		case BREAKDOWN: //充电桩故障提醒+当前充电详单+扣费成功提醒
			printf("%s", recv_info.output);
			//更新本地余额信息
			this->balance = recv_info.BALANCE;
			break;
		default:
			break;
		}
		mutexSock.unlock();
	}
}

/*
清空数据 用于退出登录或注销
*/
void User::clearData()
{
	this->usrname = "";
	this->balance = 0.0;
	this->car = nullptr;
}

/*
发送注销请求
socket
*/
int User::sendDeleteRequest(string usrname)
{
	send_info.cmd = DELETE_USER;
	strcpy_s(send_info.UID, usrname.c_str());

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);
	return recv_info.REPLY;   //test
}

//--------------------Customer--------------------
/*
消费者注销
服务器：检查用户名和密码，检查充电和扣费状态，删除用户数据
返回：注销成功/失败(1/0)
*/
int Customer::deleteAccount()
{
	cout << "==========注销界面=========" << endl;

	//首先检查是否有充电任务正在进行
	//通过查询排队结果中的前车等待数量来实现
	int suc = 0;
	// int status = getChargeStatus();
	mutexSock.lock();
	mutexUsr.lock();
	sendQueueInfoRequest();
	if (this->car && this->car->Reply && this->car->Reply->num == 0)
	{ //如果正在充电
		suc = -1;
		string choice[] = { "等待结束", "强行停止" };
		cout << "您有充电任务正在进行，是否等待充电结束? ";
		printChoice(choice, 2);
		string id;
		while (getline(cin, id), !isLegalChoice(id, 2))
		{
			cout << "请输入有效选项: ";
			printChoice(choice, 2);
		}
		if (id == "1") {
			cout << "请等待充电结束后再次尝试注销" << endl;
			return 0;
		}
		else
		{
			suc = cancelCharge(); //申请结束充电
		}
	}

	if (suc == 0) //向服务器发送注销申请
		suc = sendDeleteRequest(this->usrname);
	if (suc == 0)	//Server::usrDataUpdate的返回值只会是0
	{
		cout << "注销成功！" << endl;
		this->clearData();
	}
	else
		cout << "注销失败，请稍后再试" << endl;

	mutexSock.unlock();
	mutexUsr.unlock();
	return suc;
}

void Customer::showMenu()
{
	while (true)
	{
		cout << "**************************************************" << endl;
		cout << "******************  1.开始充电 *******************" << endl;
		cout << "******************  2.取消充电 *******************" << endl;
		cout << "******************  3.排队结果 *******************" << endl;
		cout << "******************  4.账户充值 *******************" << endl;
		cout << "******************  5.退出登录 *******************" << endl;
		cout << "******************  6.注销账号 *******************" << endl;
		cout << "**************************************************" << endl;
		cout << "\n请输入您的选择: ";
		string choice;
		while (getline(cin, choice), !isLegalChoice(choice, 6))
			cout << "输入无效，请重新选择: ";

		switch (choice[0] - '0')
		{
		case 1:
		{
			this->newChargeRequest();
			break;
		}
		case 2:
		{
			this->cancelCharge();
			break;
		}
		case 3:
		{
			this->getQueueRes();
			break;
		}
		case 4:
		{
			this->recharge();
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

/*
充值
服务器：提供用户余额信息，更新用户余额
返回：充值成功/失败(1/0)
*/
int Customer::recharge()
{
	cout << "==========充值界面=========" << endl;
	cout << "当前账户余额: " << this->balance << endl;
	string choice[] = { "20", "50", "100", "200", "自定义" };
	cout << "请选择您要充值的金额(单位：元): ";
	printChoice(choice, 5);

	//获取用户选项
	string id_str;
	int id;
	while (getline(cin, id_str), !isLegalChoice(id_str, 5))
	{
		cout << "输入无效，请重新选择：";
		printChoice(choice, 5);
	}

	id = id_str[0] - '0';
	double amount = 0.0;
	switch (id)
	{
	case 1:
		amount = 20.0;
		break;
	case 2:
		amount = 50.0;
		break;
	case 3:
		amount = 100.0;
		break;
	case 4:
		amount = 200.0;
		break;
	case 5:
		cout << "请输入自定义充值金额：";
		string str;
		while (getline(cin, str), !isPosNum(str))
			cout << "输入金额无效，请重新输入:";
		const char *s = str.c_str();
		amount = atof(s);
		break;
	}
	//向服务器发送充值请求
	mutexSock.lock();
	int suc = sendUpdateBalanceRequest(this->balance + amount);
	this->balance += amount;
	cout << "充值成功！您当前的余额为：" << this->balance << "元。" << endl;
	mutexSock.unlock();
	//if (suc == 0) //充值成功，更新本地数据
	//{
	//	mutexUsr.lock();
	//	this->balance += amount;
	//	cout << "充值成功！您当前的余额为：" << this->balance << "元。" << endl;
	//	mutexUsr.unlock();
	//}
	//else
	//	cout << "充值失败，请稍后再试" << endl;
	return 1;
}

/*
发送更新余额请求
socket
*/
int Customer::sendUpdateBalanceRequest(double add)
{
	send_info.cmd = Balance_CHANGE;
	send_info.BALANCE = add;
	strcpy_s(send_info.UID, this->usrname.c_str());

	client_sock.Send(send_info);
	//client_sock.Recv(recv_info);
	//while (strcmp(recv_info.UID, this->usrname.c_str()))
	//	client_sock.Recv(recv_info);

	return 1;
}

/*
提交充电请求
服务器：返回排队结果
*/
int Customer::newChargeRequest()
{
	cout << "==========充电请求界面=========" << endl;
	//首先检查消费者是否添加了车辆
	if (!this->car)
	{
		this->car = new Car;
		cout << "系统检测到您尚未添加车辆，请先完善车辆信息" << endl;
		cout << "请输入车辆电池容量(单位：度，最多两位小数): ";
		string kwh_str;
		while (getline(cin, kwh_str), !isPosNum(kwh_str))
			cout << "输入无效，请重新输入电池容量: ";
		const char *s = kwh_str.c_str();
		double kwh = atof(s);
		this->car->BatteryCap = kwh;	//设置电池容量
	}

	if (this->car->Reply)	//如果已经发出了充电请求
	{
		//首先检查是否已经开始充电
		mutexSock.lock();
		mutexUsr.lock();
		sendQueueInfoRequest();
		mutexSock.unlock();
		// status=1表示正在充电，=0表示未正在充电
		if (this->car && this->car->Reply && this->car->Reply->num == 0)
		{
			cout << "很抱歉，您已开始充电，无法提交新的充电请求" << endl;
			mutexUsr.unlock();
			return 0;
		}

		//若还未开始充电
		cout << "系统检测到您已经提交过充电申请，请问是否要修改该申请？" << endl;
		string choice[] = { "修改", "不修改" };
		printChoice(choice, 2);
		string co;
		while (getline(cin, co), !isLegalChoice(co, 2))
			cout << "输入无效，请重新选择: ";

		if (co == "2")
			return 0;
	}


	//获取用户输入的充电模式和充电量
	int mode, charge; // charge: 充电量
	string choice1[] = { "快充", "慢充" };
	cout << "请选择充电模式: ";
	printChoice(choice1, 2);
	string id;
	while (getline(cin, id), !isLegalChoice(id, 2))
	{
		cout << "输入无效，请重新选择: ";
		printChoice(choice1, 2);
	}
	mode = (id == "1" ? FAST : SLOW);

	//市面上主流电动汽车的电池容量为15~60kWh
	string choice2[] = { "10", "20", "30", "40", "50", "60" };
	int charge_n[] = { 0, 10, 20, 30, 40, 50, 60 };
	cout << "请输入充电量(单位：度): ";
	printChoice(choice2, 6);
	while (getline(cin, id), !isLegalChoice(id, 6))
	{
		cout << "输入无效，请重新选择";
		printChoice(choice2, 6);
	}
	charge = charge_n[id[0] - '0'];
	//请求充电量超出电池容量，则自动设置为电池容量（向下取整）
	double cap = this->car->BatteryCap;
	if (charge > cap)
	{
		cout << "您选择的充电量超出了车辆电池容量，系统已将其自动调整为车辆电池容量: " << cap << "kWh\n";
		charge = floor(cap);
	}

	//设置充电请求
	/*
	std::string usrname;  // 用户ID
	int ChargeCap;        // 请求充电量(充电桩使用)
	bool IsFastCharge;    // 快充还是慢充，快充为1
	double BatteryCap;    // 电池容量
	double BatteryNow;    // 当前电池电量
	time_t StWaitTime;    // 开始等待时间
	*/
	CarAsk *ask = new CarAsk;
	ask->usrname = this->usrname;
	ask->ChargeCap = charge;
	ask->IsFastCharge = (mode == FAST ? true : false);
	ask->BatteryCap = this->car->BatteryCap;
	ask->BatteryNow = this->car->BatteryNow;
	ask->StWaitTime = time(NULL);
	//开始等待时间由服务器或充电桩填入？

	mutexUsr.lock();
	mutexSock.lock();
	this->car->Ask = ask;
	//向服务器提交充电请求
	int suc = 0;
	suc = sendChargeRequest(); //直接读取this->car->Ask的内容
	if (suc == 0)
	{
		cout << "提交充电请求成功，正在获取排队信息..." << endl;
		cout << "您当前的排队号码为: " << this->car->Reply->queueNum << ", 前方还有" << this->car->Reply->waitingNum << "辆车在等待" << endl;
		Customer *tmp = this;
		std::thread sub{ &Customer::keepRecv,&(*tmp) };
		sub.detach();
	}
	else if (suc == -1) {
		cout << "提交充电请求失败，车辆当前位于充电区，无法提交新的充电请求" << endl;
	}
	else if (suc == -2) {
		cout << "提交充电请求失败，当前等候区已满，无法处理新的用户请求，请稍后再试" << endl;
	}

	mutexSock.lock();
	mutexUsr.lock();
	return suc;
}


/*
发送充电请求
socket
*/
int Customer::sendChargeRequest()
{
	send_info.cmd = CHARGE_REQUEST;//此处未记录电池容量 若server需要再加
	strcpy_s(send_info.UID, this->usrname.c_str());
	send_info.MODE = this->car->Ask->IsFastCharge == true ? FAST : SLOW;
	send_info.COST = this->car->Ask->BatteryNow;	//COST复用为当前容量
	send_info.BatteryCap = this->car->Ask->BatteryCap;	//BatteryCap记录电池总容量
	send_info.BALANCE = this->car->Ask->ChargeCap;	//BANLANCE复用为充电量

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);

	//设置排队结果
	CarReply *reply = new CarReply;
	reply->SID = recv_info.REPLY; //将REPLY复用为充电桩编号
	reply->num = recv_info.Q_NUM;
	reply->waitingNum = recv_info.W_NUM;
	reply->MODE = recv_info.MODE == 1 ? 'F' : 'S';

	char QNum[10]; // F1000
	QNum[0] = (recv_info.MODE == 1 ? 'F' : 'S');
	_itoa_s(reply->num, QNum + 1, 9, 10);
	reply->queueNum = QNum;
	this->car->Reply = reply;

	return recv_info.REPLY;
}

/*
取消充电
*/
// int Customer::cancelChargeRequest()
int Customer::cancelCharge()
{
	if (!this->car || this->car->Reply == nullptr)
	{
		cout << "您当前还未发出充电申请！" << endl;
		return -1;
	}
	cout << "==========取消充电界面=========" << endl;
	cout << "是否取消充电? 1. 是    2. 否" << endl;
	string id;
	while (getline(cin, id), !isLegalChoice(id, 2))
		cout << "输入无效，请重新选择: 1. 是    2. 否" << endl;

	int suc = 0;
	if (id == "1")
	{
		cout << "正在尝试取消充电..." << endl;
		//向服务器发送取消充电的请求
		mutexSock.lock();
		suc = sendCancelRequest();
		mutexSock.unlock();
		if (suc == 0)
		{
			cout << "取消成功！" << endl;
			mutexUsr.lock();
			this->car->Ask = nullptr;
			this->car->Reply = nullptr;
			mutexUsr.unlock();
		}
		else if (suc == -1)
			cout << "取消充电失败，您当前未提交充电请求" << endl;
	}
	return suc;
}
/*发送取消充电的请求
socket
*/
int Customer::sendCancelRequest()
{
	send_info.cmd = CANCEL_REQUEST;
	strcpy_s(send_info.UID, this->usrname.c_str());
	//目前只添加账户名信息 若需要用到Ask再后续添加

	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);

	// clearASK

	return recv_info.REPLY;
}

/*
查看排队结果
服务器：返回排队结果
*/
int Customer::getQueueRes()
{
	if (!this->car || this->car->Reply == nullptr)
	{
		cout << "您当前还未发出充电申请，请申请充电后再查看排队结果！" << endl;
		return -1;
	}

	cout << "==========排队信息界面=========" << endl;
	mutexSock.lock();
	mutexUsr.lock();
	int q = sendQueueInfoRequest();
	mutexUsr.unlock();
	mutexSock.unlock();
	CarReply *reply = this->car->Reply;
	cout << "您当前的排队号码为: " << reply->queueNum << ", 前方还有" << reply->num << "辆车在等待" << endl;
	return 1;
}

//发送查看排队结果的请求
// socket
int Customer::sendQueueInfoRequest()
{
	send_info.cmd = GET_QUEUE_DATA;
	strcpy_s(send_info.UID, this->usrname.c_str());
	client_sock.Send(send_info);
	client_sock.Recv(recv_info);
	while (strcmp(recv_info.UID, this->usrname.c_str()))
		client_sock.Recv(recv_info);

	//设置排队结果 
	//此处只设置一项排队号码 因为其余已在申请充电时设置
	this->car->Reply->num = recv_info.Q_NUM;
	return 0;
}

/*------Utils------*/

//判断充值金额合法性
//判断输入是否为正数（整型或浮点型，且最大小数位数为2）
bool isPosNum(string str)
{
	if (str == "")
		return false;

	int flag = 0;
	for (int i = 0; i < str.size(); i++)
	{
		if (flag > 2)
			return false; //最大小数位数为2

		if (str[i] == '.')
		{
			if (flag)
				return false; //小数点只能出现一次
			if (i == str.size() - 1 || i == 0)
				return false; //小数点不能是第一位或最后一位
			flag = 1;
			continue;
		}
		if (str[i] < '0' || str[i] > '9')
			return false;

		if (flag) //记录小数点后的位数
			flag++;
	}
	const char *s = str.c_str();
	if (atof(s) <= 0.0) //充值金额必须为正数
		return false;

	return true;
}

//判断输入是否为合法选项(max: 最大选项序号)
bool isLegalChoice(string str, int max)
{
	if (str.size() > 1)
		return false;
	int id = str[0] - '0';
	if (id < 1 || id > max)
		return false;
	return true;
}

//输出选项
void printChoice(const string choice[], int size)
{
	string separator = "    ";
	string str = "";
	for (int i = 1; i <= size; i++)
	{
		cout << i << ". " << choice[i - 1] << separator;
	}
	cout << endl;
}

//将秒转换为时分秒格式
//string User::getTime(int sec)
string getTime(long long sec)
{
	int h, m, s;
	h = sec / 3600;
	sec = sec % 3600;
	m = sec / 60;
	s = sec % 60;
	return to_string(h) + "时" + to_string(m) + "分" + to_string(s) + "秒";
}


void Admin::showMenu()
{
	//输出管理员相关功能选项
}
