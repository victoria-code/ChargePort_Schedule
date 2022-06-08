
#include "Server.h"


/*Server类实现*/

/*server with chargePort*/



/*server with admin*/



/*server with user*/




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
                DBupdate::logData.insert(make_pair(lE->usrname,lE));
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
        this->logData.insert(make_pair(data->usrname,data));
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
    map<string,logEntry*>::reverse_iterator it;
    for(it=this->logData.rbegin();it!=this->logData.rend();it++){
        string line=DBupdate::getEntry(it->second);
        log<<line<<"\n";
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
