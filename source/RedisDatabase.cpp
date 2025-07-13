#include "../include/RedisDatabase.h"

#include<thread>
#include<iostream>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<iterator>




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
    purgeExpire();
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
     purgeExpire();
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

void RedisDatabase::purgeExpire()
{
    auto now_Clock = std::chrono::steady_clock::now();
    for(auto it : expiry_map )
    {
        if( now_Clock > it.second)
        {
            // Remove from all Stores
            kv_store.erase(it.first);
            list_store.erase(it.first);
            hash_store.erase(it.first);
        }
    }
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



// LIST Opreations




size_t RedisDatabase::llen(const std::string &key)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    if(list_store.find(key) == list_store.end()) return 0;
    return list_store[key].size();
}

void RedisDatabase::lpush(const std::string &key, std::vector<std::string>& values)
{
     std::lock_guard<std::mutex> lock(db_mutex);
     for(auto value : values)list_store[key].insert(list_store[key].begin() , value);

     return ;

}

void RedisDatabase::rpush(const std::string &key, std::vector<std::string>& values)
{
     std::lock_guard<std::mutex> lock(db_mutex);
     for(auto j : values)list_store[key].push_back(j);

     return ;
}

bool RedisDatabase::lpop(const std::string &key, std::string &val)
{
     std::lock_guard<std::mutex> lock(db_mutex);
     purgeExpire();
    if(list_store.find(key) == list_store.end())
    return false;
    if(list_store[key].size() == 0) return false;
    val = list_store[key].front();
    list_store[key].erase(list_store[key].begin());

    return true;
}

bool RedisDatabase::rpop(const std::string &key, std::string &val)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    if(list_store.find(key) == list_store.end()) return false;
    if(list_store[key].size() == 0 ) return false;
    val = list_store[key][list_store[key].size() - 1];
    list_store[key].pop_back();
    return true;
}

int RedisDatabase::lrem(const std::string &key,const std::string &value , int count)
{
     std::lock_guard<std::mutex> lock(db_mutex);
     int removed = 0 ;
     auto it = list_store.find(key);
     if(it == list_store.end()) return 0;

     auto &lst = it->second;

    if (count == 0) {
        // Remove all occurances
        auto new_end = std::remove(lst.begin(), lst.end(), value);
        removed = std::distance(new_end, lst.end());
        lst.erase(new_end, lst.end());
    } else if (count > 0) {
        // Remove from head to tail
        for (auto iter = lst.begin(); iter != lst.end() && removed < count; ) {
            if (*iter == value) {
                iter = lst.erase(iter);
                ++removed;
            } else {
                ++iter;
            }
        }
    } else {
        // Remove from tail to head (count is negative)
        for (auto riter = lst.rbegin(); riter != lst.rend() && removed < (-count); ) {
            if (*riter == value) {
                auto fwdIter = riter.base();
                --fwdIter;
                fwdIter = lst.erase(fwdIter);
                ++removed;
                riter = std::reverse_iterator<std::vector<std::string>::iterator>(fwdIter);
            } else {
                ++riter;
            }
        }
    }

     return removed;
}


bool RedisDatabase::lindex(const std::string &key, int index, std::string &value)
{
     std::lock_guard<std::mutex> lock(db_mutex);
     auto it = list_store.find(key);
    if (it == list_store.end()) 
        return false;

    const auto& lst = it->second;
    if (index < 0)
        index = lst.size() + index;
    if (index < 0 || index >= static_cast<int>(lst.size()))
        return false;
    
    value = lst[index];
    return true;
     
}

bool RedisDatabase::lset(const std::string &key, int index, std::string value)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it == list_store.end()) 
        return false;

    auto& lst = it->second; 
    if (index < 0)
        index = lst.size() + index;
    if (index < 0 || index >= static_cast<ssize_t>(lst.size()))
        return false;
    
    lst[index] = value;
    return true;
}

std::vector<std::string> RedisDatabase::lget(const std::string &key)
{
    if(list_store.find(key) == list_store.end()) return {};
    return list_store[key];
}

// Hash Operations

bool RedisDatabase::hset(const std::string &key, const std::string &field, const std::string &value)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
   bool isNewField = hash_store[key].find(field) == hash_store[key].end();
    hash_store[key][field] = value;
    return isNewField;
}

bool RedisDatabase::hget(const std::string &key, const std::string &field, std::string &value)
{   
     std::lock_guard<std::mutex> lock(db_mutex);
    if(hash_store.find(key) == hash_store.end()) return false;
    if(hash_store[key].find(field) == hash_store[key].end()) return false;

    value = hash_store[key][field];
    return true;
}

bool RedisDatabase::hexist(const std::string &key, const std::string &field)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
    if(hash_store.find(key) == hash_store.end()) return  false;
    if(hash_store[key].find(field) == hash_store[key].end()) return false;
    return true;
}

bool RedisDatabase:: hdel(const std::string &key, const std::string &field)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
    if((hash_store.find(key) == hash_store.end()) || hash_store[key].find(field) == hash_store[key].end()) return false;

    hash_store[key].erase(field);
    return true;
}

std::unordered_map<std::string, std::string> RedisDatabase::hgetAll(const std::string &key)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
    if(hash_store.find(key) != hash_store.end())
    {
        return hash_store[key];
    }
    return {};
}

std::vector<std::string> RedisDatabase::hkeys(const std::string &key)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> res;
    for(auto i : hash_store[key])res.push_back(i.first);
    return res;
}

int RedisDatabase::hlen(std::string &key)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    if(hash_store.find(key) == hash_store.end()) return 0 ;
    return (int) hash_store[key].size();
}

std::vector<std::string> RedisDatabase::hvals(const std::string &key)
{
   std::lock_guard<std::mutex> lock(db_mutex); 
   std::vector<std::string> Values;
   for(auto i : hash_store[key])
   {
        Values.push_back(i.second);
   }
   return Values;
}

bool RedisDatabase::Hmset(std::string &key, std::vector<std::pair<std::string, std::string>> &vals)
{   
    std::lock_guard<std::mutex> lock(db_mutex);
    for(auto p : vals)
    {
        hash_store[key][p.first] = p.second;
    }

    return true;
}




