#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

#include<string>
#include<queue>
#include<mutex>
#include <unordered_map>
#include<vector>
#include<chrono>

class RedisDatabase
{
    public : 

    // Get SingleTon Instance
     static RedisDatabase& getInstance();

     // Database Logic
     bool dump(const std::string& filename);
     bool load(const std::string& filename);

     // Commands
     bool flushAll();

     // Key Value
     void set(const std::string& key , const std::string& value);
     bool get( const std::string& key , std::string &  value);
     std::vector<std::string> keys();
     std::string type(const std::string& key);
     bool del(const std::string& key);
     // expire
     bool expire(const std::string& key , const std::string&  seconds);
     // rename
     bool rename(const std::string& oldkey , const std::string& newKey);

     // LIST Operations
    
     bool llen(const std::string& key);
     void lpush(const std::string& key , std::string value);
     void rpush(const std::string& key , std::string& value);
     bool lpop(const std::string& key , std::string& val);
     bool rpop(const std::string& key , std::string& val);
     int lrem(const std::string&key , std::String& val , int count);
     bool lindex(const std::string& key , int index , const std::string& val);
     bool lset(const std::string& key , int index , std::string& value);





    


    private: 

    RedisDatabase();
    ~RedisDatabase();
    RedisDatabase(const RedisDatabase& ) = delete ;
    RedisDatabase& operator = (const RedisDatabase&) = delete; // Restricts the Copying Behaviour // Singleton Pattern

    std::mutex db_mutex;
    std::unordered_map<std::string , std::string > kv_store;
    std:: unordered_map<std::string , std::unordered_map<std::string , std::string > > hash_store;
    std::unordered_map<std::string , std::chrono::steady_clock::time_point > expiry_map;
    std::unordered_map<std::string , std::deque<std::string> > list_store;
};

#endif