
#include "Server.h"


/*Server类实现*/

//服务器初始化
Server::Server(){
    //获取充电桩当前信息

}

//服务器下线前同步数据库
Server::~Server(){
    this->database.update();
}

/*database Interface*/

//用户信息查询
int Server::usrFind(string usrname,usrEntry *res){
    auto it=this->database.usrData.find(usrname);
    //用户不存在
    if(it==this->database.usrData.end()){
        res=nullptr;
        return -1;
    }
    res=it->second;
    return 0;
}

//用户充电记录查询
int Server::logFind(string usrname,vector<logEntry*>&res){
    auto it=this->database.logData.find(usrname);
    if(it==this->database.logData.end()){
        cout<<"the usr "<<usrname<<" has no charge log currently."<<endl;
        return -1;
    }
    res.assign(it->second.begin(),it->second.end());
    return 0;
}

//用户信息更新<新增用户，用户信息改变，用户注销>
int Server::usrDataUpdate(bool to_delete,usrEntry *uE){
    auto it=this->database.usrData.find(uE->usrname);

    //注销
    if(to_delete){
        //待注销的用户名不存在
        if(it==this->database.usrData.end()){
            cout<<"待注销的账户<"<<uE->usrname<<">不存在！"<<endl;
            return 0;
        }
        this->database.usrData.erase(it);
        cout<<"账户<"<<uE->usrname<<">注销成功！"<<endl;
        return 0;
    }

    //注册 or 用户信息改变
    this->database.usrData[uE->usrname]=uE;
    return 0;
}


/*server with chargePort*/



/*server with admin*/



/*server with user*/

//响应用户客户端请求
int replyUser(Info *usrInfo){

}

//cmd:100 登录验证,登录成功则获取用户信息
int Server::logIn(string usrname,string passwd,usrEntry *uE){
    //检验用户是否存在
    if(Server::usrFind(usrname,uE)!=0){
        cout<<"用户名<"<<usrname<<">不存在！"<<endl;
        return -1;
    }
    //判断密码是否正确
    else if(passwd!=uE->passwd){
        cout<<"用户名和密码不匹配！"<<endl;
        return -2;
    }
    //登录成功
    else{
        cout<<"登录成功！"<<endl;
        return 0;
    }
}

//cmd:101 注册认证
int Server::signIn(string usrname,string passwd,string role){
    //用户名长度非法
    if(usrname.size()>20||usrname.size()<2){
        cout<<"用户名长度必须在5~20之间!"<<endl;
        return -1;
    }
    //用户名已存在
    else if(this->database.usrData.find(usrname)!=this->database.usrData.end()){
        cout<<"用户名已存在！"<<endl;
        return -2;
    }
    
    cout<<"注册成功!"<<endl;
    usrEntry* uE=new struct usrEntry();
    uE->usrname=usrname;
    uE->passwd=passwd;
    uE->role=role;
    uE->balance=0;
    usrDataUpdate(false,uE);
    return 0;
}

//cmd:102 充值、扣费（amount表示具体增、减金额）
int Server::balanceChange(string usrname,int amount){
    auto it=this->database.usrData.find(usrname);
    if(it==this->database.usrData.end())
        return -1;

    //充值
    if(amount>0){
        it->second->balance+=amount;
        cout<<"充值成功！"<<endl;
    }
    //扣费
    if(amount<0){
        it->second->balance+=amount;
        cout<<"扣费成功"<<endl;
    }
    return 0;
}

//cmd:103 用户注销
int Server::deleteUsr(string usrname){
    usrEntry* uE;
    uE->usrname=usrname;
    return Server::usrDataUpdate(true,uE);
}


string queueNumGenerate(string usrname,int mode);//车辆排队号码生成
int schedule(string usrname,int mode,int amount);//调度策略生成，返回充电桩编号
int recordBill(string usrname,int mode,int time);//费用计算，返回指定用户需要支付的充电费用,time为实际充电时间
bool usrDataMaintain(string usrname,vector<pair<string,string>> info);//用户信息维护：info格式：<“被修改属性名”，“修改后属性值”>】



/*DBupdate类实现*/

//数据库初始化
DBupdate::DBupdate(){

    //加载用户文件，若不存在则直接创建
    ifstream uf;
    uf.open(USER_FILENAME, ifstream::out | ifstream::app);
    if(uf.is_open()){
        while(!uf.eof()){
            //读取用户条目
            string line;
            getline(uf,line);
            while(line=="\n"&&!uf.eof()){
                getline(uf,line);
            }
            //用户条目解析
            usrEntry* uE=new usrEntry();
            if(entryResolve(uE,line)==0){
                DBupdate::usrData.insert(make_pair(uE->usrname,uE));
            }
        }
    }
    else{
        cout<<"[FATAL ERROR]: user file "<<USER_FILENAME<<" open failure"<<endl;
    }
    uf.close();

     //加载日志文件，若不存在则直接创建
    ifstream log;
    log.open(LOG_FILENAME, ifstream::out | ifstream::app);
    if(log.is_open()){
        while(!log.eof()){
            //读取日志条目
            string line;
            getline(log,line);
            while(line=="\n"&&!log.eof()){
                getline(log,line);
            }
            //日志条目解析
            logEntry* lE=new logEntry();
            if(entryResolve(lE,line)==0){
                DBupdate::logData[lE->usrname].push_back(lE);
            }
        }
    }
    else{
        cout<<"[FATAL ERROR]: log file "<<LOG_FILENAME<<" open failure"<<endl;
    }
    log.close();
}

//数据库条目解析(用户条目)
int entryResolve(usrEntry* uE,string line){
    vector<string>temp;
    split(temp,line,' ');
    if(temp.size()<4){
        cout<<"usrEntry "<<"\""<<line<<" is incomplete"<<endl;
        return -1;
    }
    uE->usrname=temp[0];
    uE->passwd=temp[1];
    uE->role=temp[2];
    uE->balance=stoi(temp[3]);
    return 0;
}

//数据库条目解析（日志条目）
int entryResolve(logEntry* lE,string line){
    vector<string>temp;
    split(temp,line,' ');
    if(temp.size()<7){
        cout<<"logEntry "<<"\""<<line<<" is incomplete"<<endl;
        return -1;
    }
    lE->start_time=temp[0];
    lE->usrname=temp[1];
    lE->SID=stoi(temp[2]);
    lE->queueNum=temp[3];
    lE->mode=stoi(temp[4]);
    lE->time=stoi(temp[5]);
    lE->cost=stoi(temp[6]);
}

//新增用户信息
int DBupdate::addUser(usrEntry *data){
    if(data!=nullptr){
        this->usrData.insert(make_pair(data->usrname,data));
        return 0;
    }
    return -1;
}

//新增日志条目
int DBupdate::addLogEntry(logEntry* data){
    if(data!=nullptr){
        this->logData[data->usrname].push_back(data);
        return 0;
    }
    return -1;
}

//更新数据库
int DBupdate::update(){

    //更新用户文件
     ofstream uf;
     uf.open(USER_FILENAME,ios::trunc|ios::out);
     map<string,usrEntry*>::reverse_iterator iter;
     for(iter=this->usrData.rbegin();iter!=this->usrData.rend();iter++){
        string line=DBupdate::getEntry(iter->second);
        uf<<line<<"\n";
     } 
     uf.close();

    //更新服务日志
    ofstream log;
    log.open(LOG_FILENAME,ios::trunc|ios::out);
    map<string,vector<logEntry*>>::reverse_iterator it;
    for(it=this->logData.rbegin();it!=this->logData.rend();it++){
        for(int j=0;j< it->second.size();j++){
            string line=DBupdate::getEntry(it->second[j]);
            log<<line<<"\n";
        }
    }
    uf.close();
}

//构造条目(用户条目)
string DBupdate::getEntry(usrEntry *data){
    string res=data->usrname;
    res+=" ";
    res+=data->passwd;
    res+=" ";
    res+=data->role;
    res+=" ";
    res+=to_string(data->balance);
    return res;
}

//构造条目（日志条目）
string DBupdate::getEntry(logEntry *data){
    string res=data->start_time;
    res+=" ";
    res+=data->usrname;
    res+=" ";
    res+=to_string(data->SID);
    res+=" ";
    res+=data->queueNum;
    if(data->mode==FAST)
        res+=" FAST ";
    else
        res+=" SLOW ";
    res+=to_string(data->time);
    res+="min ￥";
    res+=to_string(data->cost);
    return res;
}


//以字符ch为分隔符字符串拆分
int split(vector<string>&target,string line,char ch){
    string temp;
    for(int i=0;i<line.size();i++){
        if(line[i]==ch){
            target.push_back(temp);
            temp.clear();
        }
        else{
            temp+=line[i];
        }
    }
    return 0;
}
