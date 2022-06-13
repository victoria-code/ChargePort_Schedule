#include "Admin.h"

extern TSocket client_sock;
//��¼����
void Admin::showMenu(){
    while (true)
	{
		cout << "**************************************************" << endl;
		cout << "******************  1.�������׮ *******************" << endl;
		cout << "******************  2.�رճ��׮ *******************" << endl;
		cout << "******************  3.��ѯ��Ϣ *******************" << endl;
		cout << "******************  4.�˳���¼ *******************" << endl;
		cout << "******************  5.ע���˺� *******************" << endl;
		cout << "**************************************************" << endl;
		cout << "\n����������ѡ�� " << endl;
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
//����Աע��
//todo ����Ϊע���ǵǳ� ������user�Ǳ�д�Ĳ��ǵǳ���ɾ���˻�
int Admin::deleteAccount(){ 
    int suc = 0;
    cout << "==========ע������=========" << endl;
    cout << "��ȷ��Ҫע����" << endl;
    string choice[] = {"ȷ��","ȡ��"};
    printChoice(choice,2);
    string id;
    while (getline(cin, id), !isLegalChoice(id, 2))
    {
        cout << "��������Чѡ��: ";
        printChoice(choice, 2);
    }
    if(id == "1")
        suc = sendDeleteRequest(this->usrname);
    if(suc){
        cout << "ע���ɹ���" << endl;
        return 1;
    }
    else {
        cout << "ע��ʧ�ܣ����Ժ�����" << endl;
        return 0;
    }
}

//�򿪳��׮����ѡ��
//����ֵ��true�򿪳ɹ� false��ʧ��
bool Admin::openChargePort(){
    int suc = 0;
    cout << "==========�򿪳��׮=========" << endl;
    cout << "��ѡ��Ҫ�򿪵ĳ��׮��ţ�ȫ������ظ�0" << endl;
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "��������Чѡ��: ";
    }
    int id = id_str[0] - '0' - 1;
    suc = sendRequest(usrname, OPEN_CHARGEPORT, id);
    if (suc == 0){
        cout << recv_info.output << endl;
        return true;
    }
    else cout << recv_info.output << endl;
    return false;
    //todo ���׮�ѿ���
}

//�رճ��׮����ѡ��
//����ֵ��true�رճɹ� false�ر�ʧ��
bool Admin::closeChargePort(){
    int suc = 0;
    cout << "==========�رճ��׮=========" << endl;
    cout << "��ѡ��Ҫ�򿪵ĳ��׮��ţ�ȫ������ظ�0" << endl;
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "��������Чѡ��: ";
    }
    int id = id_str[0] - '0' - 1;
    suc = sendRequest(usrname, CLOSE_CHARGEPORT, id);
    if (suc == 0){
        cout << recv_info.output << endl;
        return true;
    }
    else cout << recv_info.output << endl;
    return false;
}

//��ѯ���׮��Ϣ(���׮״̬)
//����ֵ��true�ɹ� falseʧ��
bool Admin::getChargePortInfo(){
    int suc = 0;
    cout << "==========��ѯ�����Ϣ=========" << endl;
    string choice[] = {"��ѯ���׮״̬","��ѯ�Ŷӳ�����Ϣ","��ѯ����"};
    printChoice(choice,3);
    string id_str;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "��������Чѡ��: ";
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
            else cout << "��Ϣ��ѯʧ�ܣ����Ժ�����" << endl;
            return false;
            break;
        case 2:
            suc = sendReportRequest(usrname, WAIT_CAR_DATA);
            if (suc){
                printRecv(WAIT_CAR_DATA);
                return true;
            }
            else cout << "��Ϣ��ѯʧ�ܣ����Ժ�����" << endl;
            return false;
            break;
        default:
            suc = sendReportRequest(usrname, DATA_STATISTICS);
            if (suc){
                printRecv(DATA_STATISTICS);
                return true;
            }
            else cout << "��Ϣ��ѯʧ�ܣ����Ժ�����" << endl;
            return false;
            break;
    }
}

//���ͷ���������
//1��ʾ����ɹ� 0��ʾ����ʧ�� 2��ʾ���׮�ѿ��� 3��ʾ���׮�ѹر� �������ݴ�����recv_info��
int Admin::sendRequest(string usrname,int cmd,int number){
    send_info.cmd = cmd;
    strcpy_s(send_info.UID, usrname.c_str());
    send_info.REPLY = number;
    client_sock.Send(send_info);
    client_sock.Recv(recv_info);
    return recv_info.REPLY;
}

//���Ͳ�ѯ����
int Admin::sendReportRequest(string usrname,int cmd){
    send_info.cmd = cmd;
    strcpy_s(send_info.UID, usrname.c_str());
    client_sock.Send(send_info);
    client_sock.Recv(recv_info);
    //����reply 1��ʾ�ɹ� 0��ʾʧ��
    return recv_info.REPLY;
}

//��ӡ������Ϣ����
void Admin::printRecv(int cmd){
    string recv(recv_info.output);
    switch(cmd){
        case SPY_CHARGEPORT:
            cout << "���׮״̬����" << endl;
            cout << recv;
            break;
        case WAIT_CAR_DATA:
            cout << "��糵����Ϣ����" << endl;
            cout << recv;
            break;
        case DATA_STATISTICS:
            cout << "������Ϣ����" << endl;
            cout << recv;
            break;
    }
}