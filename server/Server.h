/*��������ʵ��*/
#include "ChargePort.h"
#include "TSocket_server.h"
#include<algorithm>
#pragma warning(disable : 4996)

extern TSocket server_sock;

//��¼�û���Ϣ�ļ�������־�ļ��������ļ��ͷ�������main.cppλ��ͬһĿ¼��
#define USER_FILENAME "C:\\Users\\Zheng siyang\\Desktop\\user.txt"
#define LOG_FILENAME   "C:\\Users\\Zheng siyang\\Desktop\\log.txt"

//��¼���׮��ʷ��¼��Ϣ
#define HISTORY "C:\\Users\\Zheng siyang\\Desktop\\chargePort.txt"

#define SERVICE_PRICE 0.8
#define ELEC_PRICE 1

//���׮��Ŀ
#define CHARGEPORT_NUM 5

//�û����ݿ���Ŀ
typedef struct UsrEntry {
    string usrname;
    string passwd;//����sha1ֵ
    string role;//customer �� admin
    int balance;//���
    UsrEntry() {};
}usrEntry;

//��־��Ŀ
typedef struct LogEntry {
    string start_time;//��ʼ���ʱ��
    string usrname;
    int SID;//���׮ID
    string queueNum;//�ŶӺ���
    int mode;//���ģʽ
    int time;//ʵ�ʳ��ʱ��
    int cost;//���
    LogEntry() {};
}logEntry;

//���ݿ����
class DBupdate {
public:
    /*���ݿ��ʼ��*/
    DBupdate();

    /*���ݿ���Ŀ����*/
    int entryResolve(usrEntry* uE, string line);//�û���Ŀ
    int entryResolve(logEntry* lE, string line);//��־��Ŀ

    //���ݿ������û���Ϣ
    map<string, usrEntry*>usrData;

    //������־
    map<string, vector<logEntry*>>logData;

    //��ʷ��¼
    vector<vector<int>>history;

    /*�û���Ϣά��*/
    int addUser(usrEntry* data); //�����û�     

    /*�����־*/
    int addLogEntry(logEntry* data);//������־��Ŀ

    //���ݿ����
    int update();

    //���ݿ���Ŀ����
    string getEntry(usrEntry* data);//�û���Ŀ
    string getEntry(logEntry* data);//��־��Ŀ
};



class Server {
public:

    /*��������ʼ�������׮��Ϣ��ʼ��*/
    Server(int res);

    /*����������*/
    ~Server();

    /*�����������Ƶ����ݿ�*/
    DBupdate database;
    int usrFind(string usrname, usrEntry* res);//�û���Ϣ��ѯ
    int logFind(string usrname, vector<logEntry*>& res);//�û�����¼��ѯ***
    int usrDataUpdate(bool to_delete, usrEntry* uE);//�û���Ϣ����<�����û����û���Ϣ�ı䣬�û�ע��>

    /*with chargePort*/
    vector<ChargePort*>cData;//��¼���׮״̬��Ϣ
    int FNum, TNum;  //��ǰ�Ⱥ��������ڵȺ�Ŀ��/���䳵���ŶӺ���
    int PID;//���ֹ��ϵĳ��׮ID
    time_t failTime;//���ֹ��ϵ�ʱ��
    void forwardRequet(string usrname, int SID);//����׮ת��������ڳ������Ԥ�����**
    void sendDetail(ChargeTablePool* next);//���û����Ͷ�Ӧ�ĳ���굥�����п۷�**

   /*��¼��ǰ�ѵ�¼���û����û���ַ*/
    map<string, string>usrAdress;

    /*with client*/
    map<string, CarAsk*>WUser;    //��¼����λ�ڵȺ������û���������
    map<string, CarAsk*>CUser;    //��¼����λ�ڳ�������û���������
    map<string, pair<string, int>>queueData;//��¼�ŶӺ����ǰ���ȴ�����
    map<string, int>CUserID;//��¼��������û�������ĳ��׮���
    map<string, bool>reqRes;//��¼�û�������񱻴������
    int replyClient(Info usrInfo);//��Ӧ�û��ͻ�������
    int logIn(string usrname, string passwd, usrEntry* uE);//��¼��֤����¼�ɹ����ȡ�û���Ϣ
    int signUp(string usrname, string passwd, string role);//ע����֤
    int deleteUsr(string usrname);//�û�ע��

    //CUSTOMER
    int balanceChange(string usrname, int amount);//��ֵ���۷ѣ�customer)***
    int copeChargeRequest(CarAsk* ask);//����Ⱥ������û��ĳ������(customer)***
    int cancelCharge(string usrname);//�����û�ȡ���������(customer)***
    int getQueueData(string usrname, string& qNum, int& curWait);//��ȡ�Ŷ���Ϣ(customer)***
    string queueNumGenerate(string usrname, int mode);//�����ŶӺ�������(customer)
    int getCurWaitNum(string usrname);//��ȡ���µ�ǰ���ȴ�������Ϣ(customer)
    int Calling(string& fUser, string& tUser);//��ȡ�Ⱥ����ڼ������������Ŀ�������usrname(�кţ�(customer)***
    int getFreeCP(int& fSID, int& tSID, int& fTime, int& tTime);//��ȡ��������ŶӶ����п�λ�ҵȴ�ʱ����̵ĳ��׮(customer)***
    int resolveRequest(Info& usrInfo, CarAsk* ask);//��ȡ�������(customer)

    //ADMIN
    int openCP(string usrname, int SID);//�������׮
    int closeCP(string usrname, int SID);//�رճ��׮
    int getChargePortData(string usrname);//��ȡ���׮״̬��Ϣ
    int getReport(string usrname);//��ȡ����
    int getCarData(string usrname);//��ȡ�Ⱥ����ĳ�����Ϣ

    //��ȡʱ��
    string getCurTime(time_t t);

};




//���ַ�chΪ�ָ����ַ������
int split(vector<string>& target, string line, char ch);
