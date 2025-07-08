#ifndef REDIS_COMMAND_HANDLER_h
# define REDIS_COMMAND_HANDLER_H
#include<string>

class RedisCommandHanlder
{
    public :
    RedisCommandHanlder();

    std::string proccessCommand(const std::string& commandLine);
};


#endif 