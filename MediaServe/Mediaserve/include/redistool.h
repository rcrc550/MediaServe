#ifndef REDISTOOL_H
#define REDISTOOL_H

#include<iostream>
#include<list>
#include<hiredis/hiredis.h>
#include<vector>

using namespace std;

#define _DEF_REDIS_IP       "127.0.0.1"
#define _DEF_REDIS_PORT     6379
#define _DEF_BUF_SIZE       100
class RedisTool
{
public:
    RedisTool();
    ~RedisTool();
public:
    bool isExist( string key );
    void removeKey( string key);

    /// string
    ///
    bool setString(string key , string val);
    bool getString( string key , string& val ); //能否取出用bool返回

    /// list
    ///
    bool setList(string key , vector<string> &val );// 设置多个value
    bool appendOneToList( string key , string val);
    bool getList(string key , vector<string> &val );
    bool setzseVal(string key, int score, string val);
    bool zdel(string key, string field);
    /// hash
    ///
    bool getHashVal(string key , string field,  string &val);
    bool getHashVals(string key , vector<string>& field,  vector<string> &val);
    bool setHashVal( string key , string field,  string val );
    bool setHashVals( string key , vector<string>& field,  vector<string> &val );
    bool delHashField( string key , string field );
    bool hkeys(string key,  vector<string>& val);
    bool settheSet(string key, string field);
    bool smember(string key,  vector<string>& val);
    int  sinter(string key, string val);
    bool heys(string key,  vector<string>& val,vector<string>&stm);
     bool zrange(string key,  vector<string>& val,vector<string>&stm);
    bool zset(string key);

private:
    void init();
    redisReply* command( const char * str );

    redisContext * m_redis;

    static std::string m_redisIP;
    static int m_redisPort;
};




#endif // REDISTOOL_H
