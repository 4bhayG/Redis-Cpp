#ifndef RedisServer_h
# define RedisServer_h

#include <winsock2.h> 
#include <ws2tcpip.h>
#include<string>
#include<atomic>
class RedisServer{
    public :
    RedisServer(int port);

    void run();

    void shutdown();

    private : 
    int port;
    SOCKET server_socket;
    std::atomic<bool>  running;

    void setupSignalHandler();

};





#endif