#include<iostream>
#include "../include/RedisServer.h"
#include "../include/RedisDatabase.h"
#include<thread>
#include<chrono>

int main(int argc , char* argv[])
{
    int port = 6379; // default port
      try {
        if (argc >= 2) {
            port = std::stoi(argv[1]); // Custom port
        }
    } catch (const std::exception& e) {
        std::cerr << "Invalid port. Using default port 6379.\n";
    }



    RedisServer server(port); // create server Instance


    // Background data persistence - > data dumping periodically
    std:: thread persistenceThread([]()
{
    while(true)
    {
        std:: this_thread :: sleep_for(std::chrono::seconds(300)) ; // 5 Minutes
        
        // Dump the database
        if(!RedisDatabase::getInstance().dump("dump.my_rdb"))
           { std::cerr << "Error Dumping Database\n";}
        else
        {
            std::cout<< "Database Dumped to dump.my_rdb\n";
        }
    }

}); 

    persistenceThread.detach();


    server.run();

    return 0;
}