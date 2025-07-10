#include<thread>
#include<iostream>
#include<fstream>
#include<sstream>

#include "../include/RedisDatabase.h"

RedisDatabase& RedisDatabase :: getInstance()
{
     static RedisDatabase instance ;
     return instance;
}

// Creating Memory
// Dump File -> Again Load Memory from File

/*
K = Key Value
L = List
H = Hash
*/













bool RedisDatabase:: dump(const std:: string& filename)
{
    std:: lock_guard<std::mutex> lock(db_mutex);
    std::ofstream ofs(filename ,std :: ios :: binary);
    if(!ofs) return false;
    
    for(const auto& kv :kv_store)
    {
        ofs << "K" << kv.first << " " << kv.second << "\n";
    }
    for(const auto& kv : list_store)
    {
        ofs << "L" << kv.first;
        for(const auto& item : kv.second)
        {
            ofs << " " << item;
        }
        ofs << "\n";
    }

    for(const auto& kv : hash_store)
    {
        ofs << "H" << kv.first;
        for(const auto& fieldv : kv.second)
        {
            ofs << " " << fieldv.first << ":" << fieldv.second;
        }
        ofs << "\n";
    }

    return true;

}

bool RedisDatabase::load(const std::string & filename)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
    std:: ifstream ifs(filename  ,std::ios :: binary);
    if(!ifs)
    {
        return false;
    }

    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std :: string line;

    while(std::getline(ifs , line))
    {
        std::istringstream iss(line); // Input Memory Stream
        char type;
        iss >> type;
        if(type == 'K')
        {
            std::string key;
            std::string val;
            iss >> key >> val;
            kv_store[key] = val;
        }
        else if(type == 'L')
        {   

            std::string key;
            iss >> key;
            std::string item;
            std::vector<std::string > list_item;
            while(iss >> item )
            {
                list_item.push_back(item);
            }
            list_store[key] = list_item;

        }
        else if (type == 'H') {
            std::string key;
            iss >> key;
            std::unordered_map<std::string, std::string> hash;
            std::string pair;
            while (iss >> pair) {
                auto pos = pair.find(':');
                if (pos != std::string::npos) {
                    std::string field = pair.substr(0, pos);
                    std::string value = pair.substr(pos+1);
                    hash[field] = value;
                }
            }
            hash_store[key] = hash;
        }
    }

    
    return true;
}

RedisDatabase::RedisDatabase() {
  
}

RedisDatabase::~RedisDatabase() {
   
}

bool RedisDatabase :: flushAll()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    
    return true;
}

// Key-Value Operations

void RedisDatabase::set(const std::string& key , const std::string& val)
{
    std::lock_guard<std::mutex>lock(db_mutex);
    kv_store[key] = val;
}

bool RedisDatabase :: get(const std::string& key ,  std::string& val)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    if(kv_store.find(key) == kv_store.end())return false;
    val = kv_store[key];
    return true;
}

std::vector<std::string> RedisDatabase :: keys()
{
    std::lock_guard<std::mutex> locK(db_mutex);
    std::vector<std::string> res;
    for(auto i : kv_store)
    {
        res.push_back(i.first);
    }
     for(auto i : list_store)
    {
        res.push_back(i.first);
    }
     for(auto i : hash_store)
    {
        res.push_back(i.first);
    }

    return res;
    
}

std::string RedisDatabase :: type(const std::string& key)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    if(kv_store.find(key) != kv_store.end())
    {
        return  "String";
    }
    if(list_store.find(key) != list_store.end())
    {
        return  "List";
    }
    if(hash_store.find(key) != hash_store.end())
    {
        return  "Hash";
    }
    return "None";
    
}

bool RedisDatabase :: del(const std::string& key)
{
     std::lock_guard<std::mutex> lock(db_mutex);
     bool erased = false;
     erased |= (kv_store.erase(key) > 0) ;
     erased |= (list_store.erase(key) > 0) ;
     erased |= (hash_store.erase(key) > 0) ;

     return erased;

}

bool RedisDatabase ::  expire(const std::string& key , const std::string& seconds)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    bool exist = ( kv_store.find(key) != kv_store.end()) ||
    ( list_store.find(key) != list_store.end()) || 
    ( hash_store.find(key) != hash_store.end());

    if(!exist) return false;
    expiry_map[key] = std::chrono::steady_clock :: now() + std::chrono :: seconds(std::stoi(seconds));
    return true;
}

bool RedisDatabase::rename(const std::string &oldkey, const std::string &newKey)
{
    std::lock_guard<std::mutex>lock(db_mutex);
    bool found = false;
    if(kv_store.find(oldkey) != kv_store.end())
    {
        found = true;
        kv_store[newKey] = kv_store[oldkey];
        kv_store.erase(oldkey);
    }
     if(list_store.find(oldkey) != list_store.end())
    {
        found = true;
        list_store[newKey] = list_store[oldkey];
        list_store.erase(oldkey);
    }
     if(hash_store.find(oldkey) != hash_store.end())
    {
        found = true;
        hash_store[newKey] = hash_store[oldkey];
        hash_store.erase(oldkey);
    }
     if(expiry_map.find(oldkey) != expiry_map.end())
    {
        found = true;
        expiry_map[newKey] = expiry_map[oldkey];
        expiry_map.erase(oldkey);

    }

    return found;

}
