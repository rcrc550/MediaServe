#include "redistool.h"
#include "string.h"

std::string RedisTool::m_redisIP    = _DEF_REDIS_IP;
int         RedisTool::m_redisPort  = _DEF_REDIS_PORT;

RedisTool::RedisTool():m_redis(nullptr)
{
    init();
}

RedisTool::~RedisTool()
{
    if( m_redis ){
        redisFree( m_redis );
        cout << "redis free connect." << endl;
        m_redis = nullptr;
    }
}

//REDIS_REPLY响应的类型type
//cout << "#define REDIS_REPLY_STRING 1"<< endl;
//cout << "#define REDIS_REPLY_ARRAY 2"<< endl;
//cout << "#define REDIS_REPLY_INTEGER 3"<< endl;
//cout << "#define REDIS_REPLY_NIL 4"<< endl;
//cout << "#define REDIS_REPLY_STATUS 5"<< endl;
//cout << "#define REDIS_REPLY_ERROR 6"<< endl;

bool RedisTool::isExist(string key)
{
    redisReply * reply = nullptr; //请求返回值

    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "keys %s" , key.c_str() );

    reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_ARRAY)
    {
        if( reply->elements != 0 )
            flag = true;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;
}

void RedisTool::removeKey(string key)
{
    redisReply * reply = nullptr; //请求返回值

    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "del %s" , key.c_str() );

    reply = this->command( buf );

    if( !reply ) return;

    //回收返回值
    freeReplyObject(reply);
    return;

}

bool RedisTool::setString(string key, string val)
{

    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "set %s %s" , key.c_str() , val.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( strcmp( reply->str , "OK") == 0 ) // == ok
    {
        flag = true;
    }else{
        flag = false;
        cout << "set string fail : " << reply->str <<endl;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}

bool RedisTool::getString( string key , string& val )
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "get %s" , key.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->len > 0 ) //返回结果有多长
    {
        val = reply->str;
        flag = true;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}

bool RedisTool::setList(string key, vector<string> &val)
{
    //先删除
    removeKey( key );
    //然后设置

    uint ncount = 0 ;
    for( uint i = 0 ; i < val.size() ; i++ )
    {
        if( appendOneToList( key , val[i] ) )
            ncount++;
    }
    if( ncount == val.size() )
        return true;
    else
        return false;

}

bool RedisTool::appendOneToList(string key, string val)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "rpush %s %s" , key.c_str(), val.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_INTEGER ) //返回结果有多长
    {
        if( reply->integer > 0  )
            flag = true;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}

bool RedisTool::getList(string key, vector<string> &val)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "lrange %s 0 -1" , key.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if (reply->type == REDIS_REPLY_ARRAY) {
            for (unsigned int j = 0; j < reply->elements; j++) {
                val.push_back( reply->element[j]->str) ;
            }
            flag = true;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}

bool RedisTool::getHashVal(string key, string field, string& val)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "hget %s %s" , key.c_str() , field.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;//如果reply=0 那就是m_redis出问题了


    bool flag = false;

    if( reply->len > 0 ) //返回结果有多长
    {
        val = reply->str;
        flag = true;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}

bool RedisTool::getHashVals(string key, vector<string> &fields, vector<string> &vals)
{
    for( uint i = 0 ; i < fields.size() ; ++i ){
        string val = "";
        getHashVal( key , fields[i] ,val ) ;
        vals.push_back( val );
    }
    return true;
}

bool RedisTool::setHashVal(string key, string field, string val)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "hset %s %s %s"
             , key.c_str() , field.c_str() , val.c_str() );

    //先删除再设置
    delHashField( key , field );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_INTEGER )
    {
        if( reply->integer > 0 )
            flag = true;
        else
        {
            flag = false;
            printf("set hash failed\n");
        }
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}
bool RedisTool::setzseVal(string key, int score, string val)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "zadd %s %d %s"
             , key.c_str() , score , val.c_str() );

    zdel( key.c_str(),val.c_str());

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_INTEGER )
    {
        if( reply->integer > 0 )
            flag = true;
        else
            flag = false;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}
bool RedisTool::setHashVals(string key, vector<string> &field, vector<string> &val)
{
    if( field.size() != val.size() )
    {
        cout << " field size != val size \n";
        return false;
    }

    for( uint i = 0 ; i < field.size() ; ++i )
    {
        if( !setHashVal( key , field[i] , val[i] ) ){
            removeKey( key );
            return false;
        }
    }
    return true;
}

bool RedisTool::delHashField(string key, string field)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "hdel %s %s"
             , key.c_str() , field.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_INTEGER )
    {
        if( reply->integer > 0 )
            flag = true;
        else
            flag = false;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;
}
bool RedisTool::zdel(string key, string field)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "zrem %s %s"
             , key.c_str() , field.c_str() );

    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_INTEGER )
    {
        if( reply->integer > 0 )
            flag = true;
        else
            flag = false;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;
}
void RedisTool::init()
{
    //连接redis
    struct timeval tmout ={ 1, 500000 }; //1.5 秒超时时间
    cout << "init ip:" << m_redisIP.c_str() <<" port:" << m_redisPort << endl;
    m_redis = redisConnectWithTimeout( m_redisIP.c_str() , m_redisPort, tmout);

    //查看是否有问题
    if( m_redis->err )
    {
        cout << "connect error :" << m_redis->errstr << endl;
    }else{
        cout << "redis connect success!" <<endl ;
    }
}

redisReply *RedisTool::command(const char *str)
{
    redisReply * reply = nullptr; //请求返回值

    if( !m_redis || m_redis->err ){
        cout << "redis init error" <<endl;
        init();
        return reply;
    }
    //请求
    reply = (redisReply*)redisCommand( m_redis , str );
    //返回值 返回出去外面使用
    if( reply ){
        return reply;
    }else{ //command 异常
        redisFree( m_redis );
        m_redis = nullptr;

        cout << str << " error reply = null " <<endl;
        return reply;
    }

}
bool RedisTool::hkeys(string key,  vector<string>& val)
{
    RedisTool l;
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "hkeys %s" , key.c_str());

    redisReply * reply = l.command( buf );

    if( !reply ) return 0;

    bool flag = false;

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (unsigned int j = 0; j < reply->elements; j++) {
            val.push_back( reply->element[j]->str);
        }
        flag = true;
    }
    return flag;
}
bool RedisTool::heys(string key,  vector<string>& name,vector<string>&stm)
{

    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "hkeys %s" , key.c_str());

    redisReply * reply = this->command( buf );

    if( !reply ) return 0;

    bool flag = false;
  //  srand(0);

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (unsigned int j = 0; j <3; j++) {
            int i=rand()%reply->elements;
            printf("ran %d\n",i);


            string p(reply->element[i]->str);
            name[j]= p;
            printf("ele %s\n",reply->element[i]->str);
            getHashVal(key,p,stm[j]);
        }
        flag = true;
    }
    return flag;
}

bool RedisTool::zrange(string key, vector<string> &name, vector<string> &stm)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "zrange %s 0 2" , key.c_str());

    redisReply * reply = this->command( buf );

    if( !reply ) return 0;

    bool flag = false;


    if (reply->type == REDIS_REPLY_ARRAY) {
        for (unsigned int j = 0; j <3; j++) {
            name[j]=reply->element[j]->str;
            string p(reply->element[j]->str);
            getHashVal("rtmp",p,stm[j]);
        }
        flag = true;
    }
    return flag;
}
bool RedisTool::zset(string key)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "zscore rtmp2 %s" , key.c_str());

    redisReply * reply = this->command( buf );

    if( !reply ) return 0;

    bool flag = false;
    if( reply->len > 0 ) //返回结果有多长
    {
        char *val = reply->str;
        int c=atoi(reply->str)+1;
        printf("set %d\n",c);
        zdel("rtmp2",key.c_str());
        sprintf( buf , "zadd rtmp2 %d %s" , c,key.c_str());
        this->command( buf );

        flag = true;
    }



    return flag;
}
bool RedisTool::smember(string key,  vector<string>& val)
{
    RedisTool l;
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "smembers %s" , key.c_str());

    redisReply * reply = l.command( buf );

    if( !reply ) return 0;

    bool flag = false;

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (unsigned int j = 0; j < reply->elements; j++) {
            val.push_back( reply->element[j]->str);
        }
        flag = true;
    }
    return flag;
}
int RedisTool::sinter(string key, string val)
{
    RedisTool l;
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "sinter %s %s" , key.c_str(),val.c_str());

    redisReply * reply = l.command( buf );

    if( !reply ) return 0;


    if (reply->type == REDIS_REPLY_ARRAY) {
        return reply->elements;
    }
   return 0;
}
bool RedisTool::settheSet(string key, string field)
{
    char buf[_DEF_BUF_SIZE] = "";
    sprintf( buf , "sadd %s %s"
             , key.c_str() , field.c_str()  );



    redisReply * reply = this->command( buf );

    if( !reply ) return false;

    bool flag = false;

    if( reply->type == REDIS_REPLY_INTEGER )
    {
        if( reply->integer > 0 )
            flag = true;
        else
            flag = false;
    }

    //回收返回值
    freeReplyObject(reply);
    return flag;

}
