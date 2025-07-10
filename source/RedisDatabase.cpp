#include "../include/RedisDatabase.h"

#include<thread>
#include<iostream>
#include<fstream>
#include<sstream>

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
    // constructor body (even if empty)
}

RedisDatabase::~RedisDatabase() {
    // destructor body (even if empty)
}