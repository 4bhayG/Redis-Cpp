#include "../include/RedisCommandHanlder.h"

#include "../include/RedisDatabase.h"

#include<vector>
#include<string>
#include<iostream>
#include<sstream>
#include<algorithm>

// RESP Parser : 
// *2\r\n$4\r\n\PING\r\n$4\r\nTEST\r\n -- > PING TEST

std:: vector<std::string> parseRespCommand(const std::string& input)
{    std::vector<std::string> tokens;

    if(input.empty())return tokens; 


    // std::cout<< "\n Input Revieved : \n";
    // std::cout<<input<<"\n";

    // // Printing - Debugging
    // for(auto& t : tokens )
    // {
    //     std::cout<<t << "\n";
    // }

    // if not starting via * -> fallback splitting via whitespaces
    if(input[0] != '*')
    {
         std::istringstream iss(input);

         std::string token;
         while( iss >> token)
         {
            tokens.push_back(token);
         }
    }

    size_t pos = 0;
    // Expect * to follow bby number of elements
    if( input[pos] != '*')
    {
        return tokens;
    }
    pos++;
    size_t currLinefields = input.find("\r\n" , pos);

    if(currLinefields == std::string::npos ) return tokens;
    int numElements = std::stoi(input.substr(pos , currLinefields - pos));
    pos = currLinefields + 2;

    for (int i =0 ; i < numElements ; i++)
    {
        if(pos >= input.size() || input[pos] != '$') break;
        pos++; // skip $

        currLinefields = input.find("\r\n" , pos);
        if(currLinefields == std::string::npos) break;

        int len = std::stoi(input.substr(pos , currLinefields - pos));
        pos = currLinefields + 2 ;

        if(pos + len > input.size()) break;


        std::string token = input.substr(pos , len);
        tokens.push_back(token);

        pos += len + 2 ; // skip token and crlf

    }
    

    // // Debugging
    // std::cout<<"Parsed Message: \n";
    // for(auto& t : tokens)
    // {
    //     std::cout<<t<<" ";
    // }
    // std::cout<<"\n";
    return tokens;
   
}

RedisCommandHanlder :: RedisCommandHanlder(){};

std::string RedisCommandHanlder::proccessCommand(const std::string& commandLine)
{
    // Use Resp Parser
    auto tokens = parseRespCommand(commandLine);

    if(tokens.empty())
    {
        return "Error Empty Command Line\r\n";
    }   

    std::string cmd = tokens[0];
    std::transform(cmd.begin() , cmd.end() , cmd.begin() , ::toupper);

    std:: ostringstream response;

    // Connect to Db

    RedisDatabase& Db_Instance = RedisDatabase :: getInstance();
    
    // TODO
    
    // Check Commands
    if(cmd == "PING")
    {
        response << "+PONG\r\n";
    }
    else if(cmd == "ECHO")
    {
        if(tokens.size() < 2)
        {
            response << "-Error ECHO Reqiuires a Message\r\n";
        }
        else
             response << "+" << tokens[1] << "\r\n";
    }
    else if( cmd == "FLUSHALL" )
    {
        Db_Instance.flushAll();
        response << "+OK\r\n";
    }
    else if( cmd == "SET" )
    {
        if(tokens.size() < 3 )
        {
            response << "-Error : SET Reqiuires Key and Value\r\n ";
        }
        else
        {
            Db_Instance.set(tokens[1] , tokens[2]);
            response << "+OK\r\n";
        }
    }
    else if(cmd == "GET")
    {
        if(tokens.size() < 2 )
        {
            response << "-Error : GET Reqiuires Key\r\n ";
        }
        else
        {
            std::string value;
            if(Db_Instance.get(tokens[1] , value))
            {
                response  << "$" << value.size() << "\r\n" << value << "\r\n";
            }
            else
            {
                response << "$-1\r\n";
            }
            
        }
    }
    else if(cmd == "KEYS")
    {
        std::vector<std::string> allkeys;
        allkeys = Db_Instance.keys();
        response << "*" << allkeys.size() << "\r\n";
        for(auto& i : allkeys)
        {
            response << "$" << i.length() << "\r\n" << i << "\r\n";
        }
    }
    else if( cmd == "TYPE" )
    {
         if(tokens.size() < 2 )
        {
            response << "-Error : TYPE Reqiuires Key\r\n ";
        }
        else
        {
            response << "+" << Db_Instance.type(tokens[1]) << "\r\n";
        }
    }
    else if( cmd  == "DEL" || cmd == "UNLINK")
    {
        if(tokens.size() < 2)
        {
            response << "-Error : DEL Requires Key\r\n"; 
        }
        else
        {
            bool res = Db_Instance.del(tokens[1]);
            response << ":"  << (res ? 1 : 0) << "\r\n";
        }
    }
    else if( cmd == "EXPIRE")
    {
        if(tokens.size() < 3)
        {
            response << "-Error : EXPIRE Requires Key and time\r\n";
        }
        else
        {   if( Db_Instance.expire(tokens[1] , tokens[2]))
            {
                response << "+OK\r\n";
            }

        }
    }
    else if(cmd == "RENAME")
    {
        if( tokens.size() < 3)
        {
            response << "-Error : RENAME Requires Old and New Key\r\n";
        }
        else
        {  if( Db_Instance.rename(tokens[1] , tokens[2]))
                response << "+OK\r\n";

        }  
    }
    else
    {
        response << "- Error -Wrong Command\r\n";
    }


    return response.str(); // Returns any generic Pattern

}


// Test Code for Parser
// int main()
// {
//     std::string ip = "*2\r\n$4\r\n\PING\r\n$4\r\nTEST\r\n"; -- > PING TEST OUTPUT
//     for(auto i : parseRespCommand(ip))
//     {
//         std::cout<<i<<" ";
//     }

//     return 0;
// }
