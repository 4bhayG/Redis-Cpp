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
     void purgeExpire();
     // rename
     bool rename(const std::string& oldkey , const std::string& newKey);

     // LIST Operations
    
     size_t llen(const std::string& key);
     void lpush(const std::string& key , std::vector<std::string>& values);
     void rpush(const std::string& key , std::vector<std::string>& values);
     bool lpop(const std::string& key , std::string& val);
     bool rpop(const std::string& key , std::string& val);
     int lrem(const std::string&key , const std::string& val , int count);
     bool lindex(const std::string& key , int index ,std::string& val);
     bool lset(const std::string& key , int index , std::string value);
     std::vector<std::string> lget(const std::string& key);

    // Hash Operations
    bool hset(const std::string& key , const std::string& field , const std::string& value);
    bool hget(const std::string& key , const std::string& field , std::string& value);
    bool hexist(const std::string& key , const std::string& field);
    bool hdel(const std::string& key , const std::string& field );
    std::unordered_map<std::string , std::string> hgetAll(const std::string& key);
    std::vector<std::string> hkeys(const std:: string& key);
    int hlen(std::string& key);
    std::vector<std::string> hvals(const std::string& keys);
    bool Hmset(std::string& key , std::vector<std::pair<std::string , std::string> >&  vals);



    private: 

    RedisDatabase();
    ~RedisDatabase();
    RedisDatabase(const RedisDatabase& ) = delete ;
    RedisDatabase& operator = (const RedisDatabase&) = delete; // Restricts the Copying Behaviour // Singleton Pattern

    std::mutex db_mutex;
    std::unordered_map<std::string , std::string > kv_store;
    std:: unordered_map<std::string , std::unordered_map<std::string , std::string > > hash_store;
    std::unordered_map< std::string , std::vector<std::string> > list_store;
    std::unordered_map<std::string , std::chrono::steady_clock::time_point > expiry_map;
    std::unordered_map<std::string , std::deque<std::string> > queue_store;
};

#endif