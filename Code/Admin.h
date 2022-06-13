#ifndef __ADMIN_H__
#define __ADMIN_H__
#pragma once

#include<iostream>
#include<string>

#include "User.h"
#include "client.h"
#include "main.h"
#include "TSocket.h"

using namespace std;

class Admin : public User
{
private:

public:
    Admin(){
        this->type = ADMIN;
    }

    Admin(string name, int t)
    {
        this->usrname = name;
        this->type = t;
    }

    
    // ��¼��Ĳ�������
    void showMenu();
    
    /*with chargePort*/
    //��ѯ���׮��Ϣ(���׮״̬���Ŷӳ�����Ϣ������)
    bool getChargePortInfo();

    //�������׮
    bool openChargePort();

    //�رճ��׮
    bool closeChargePort();

    //���������������
    int sendRequest(string usrname, int cmd, int number); 
    int sendReportRequest(string usrname, int cmd);
    
    //�û�ע��
    int deleteAccount();

    //��������Ϣ
    void printRecv(int cmd);
};


#endif