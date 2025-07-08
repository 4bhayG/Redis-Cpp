#include "../include/RedisCommandHanlder.h"

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


    std::cout<< "\n Input Revieved : \n";
    std::cout<<input<<"\n";

    // Printing - Debugging
    for(auto& t : tokens )
    {
        std::cout<<t << "\n";
    }

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
    
    // TODO
    
    // Check Commands


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
