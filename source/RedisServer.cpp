#include <iostream>
#include "../include/RedisServer.h"
#include "../include/RedisCommandHanlder.h"
#include "../include/RedisDatabase.h"
#include<vector>
#include<thread>
#include<cstring>
#include <winsock2.h>   // Core Winsock functions
#include <ws2tcpip.h>   // IP address manipulation 
#include<signal.h>

#pragma comment(lib, "ws2_32.lib") // Ensure linking Winsock library
static RedisServer* globalServer = nullptr; // creating a global db server instance



void signalHanlder(int signum)
{
    if(globalServer)
    {
        std::cout << "Caught Signal " <<signum << " , shutting down...\n";
        globalServer -> shutdown();
    }

    exit(signum);

}

void RedisServer :: setupSignalHandler()
{
    signal(SIGINT , signalHanlder);
}




RedisServer :: RedisServer(int port) : port(port) , server_socket(INVALID_SOCKET) , running(true){
    globalServer = this;
    setupSignalHandler();
}

void RedisServer:: shutdown()
{
    running = false;
    if(server_socket != INVALID_SOCKET)
    {
        closesocket(server_socket);
        server_socket = INVALID_SOCKET;
    }

    WSACleanup(); // Cleans up Winsock sockets after usage


    std::cout<< "Server Shutdown Complete\n";
}

void RedisServer:: run()
{   

     WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return;
    } // Windows need manual Socket initialisation thus this is needed


    server_socket = socket(AF_INET , SOCK_STREAM , 0);
    if(server_socket == INVALID_SOCKET )
    {
        std::cout<<"Socket Initialisation Failed ";
    }

    int opt = 1;
    int result_socket_opt = setsockopt(server_socket , SOL_SOCKET , SO_REUSEADDR ,(const char*) &opt , sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET ; // Ipv4 Addressing
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket , (struct sockaddr*)&serverAddr ,  sizeof(serverAddr))  == SOCKET_ERROR )
    {
        // Error binding socket to Port
        std::cerr << "Error Binding Server Socket\n";
        closesocket(server_socket);
        WSACleanup();
        return ;
    }

    if ( listen(server_socket , 10 ) == SOCKET_ERROR )
    {
        std::cerr << "Error Listening on Server\n";
        closesocket(server_socket);
        WSACleanup();
        return;
    }


    std::cout << "Redis Server is pinging on " << port << "\n"; 


    std:: vector<std::thread > threads;
    RedisCommandHanlder cmdHandler;

    while(running)
    {
        SOCKET client_socket = accept(server_socket , nullptr , nullptr);
        if(client_socket == INVALID_SOCKET)
        {
            if(running)
            {
                 std::cerr<< "Error Accepting Client Req for Connection\n";
                 break;
            }
        }

        threads.emplace_back([client_socket , &cmdHandler]()
        {
            char buffer[1024];
            while(true)
            {
                memset(buffer , 0 , sizeof(buffer));
                int bytes = recv(client_socket , buffer , sizeof(buffer) -1 , 0);
                if(bytes <= 0 ) break;

                std::string request(buffer , bytes);
                std::string response = cmdHandler.proccessCommand(request);
                send(client_socket , response.c_str() , response.size() , 0);
            }
            closesocket(client_socket);
            
        });
    }

    for(auto& t : threads)
    {
        if (t.joinable()) t.join();
    }

    // persistence Checking

    if(RedisDatabase::getInstance().dump("dump.my_rdb"))
    {
        std::cout << "Database dumped to dump.my_rdb";
    }
    else
    {
        std::cerr <<  "Error Dumping Database"; 
    }

    // Shutdown

}