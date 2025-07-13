#include "../include/RedisCommandHanlder.h"

#include "../include/RedisDatabase.h"

#include<vector>
#include<unordered_map>
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


static std::string HandleLen(const std::vector<std::string>& tokens , RedisDatabase& db)
{    if(tokens.size() < 2)
    {
        return "-Error : LLEN Requires Key\r\n";
    }
    size_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len)  +  "\r\n" ;
}


static std::string handleLpush(const std::vector<std::string>& tokens , RedisDatabase& db)
{   if(tokens.size() < 2)
    {
        return "-Error : LPUSH Requires Value\r\n";
    }
    else
    {   std::vector<std::string> values;
        for(int index = 2 ; index < tokens.size() ; index++)
        {
            values.push_back(tokens[index]);
        }
        db.lpush(tokens[1] , values);
        size_t len = db.llen(tokens[1]);
        return ":"  +  std::to_string(len)  +  "\r\n" ;
    }
}

static std::string handleRpush(const std::vector<std::string>& tokens , RedisDatabase& db)
{   if(tokens.size() < 3)
    {
        return "-Error : RPUSH Requires Key and Value\r\n";
    }
    else
    {   std::vector<std::string> values;
        for(int index = 2 ; index < tokens.size() ; index++)
        {
            values.push_back(tokens[index]);
        }
        db.rpush(tokens[1] , values);
        size_t len = db.llen(tokens[1]);
        return ":"  +  std::to_string(len) +  "\r\n" ;
    }
    
}

static std::string handleLpop(const std::vector<std::string>& tokens , RedisDatabase& db)
{   if(tokens.size() < 2)
    {
        return "-Error : LPOP Requires Key\r\n";
    }
    else
    {
        std::string value ;
        if(db.lpop(tokens[1] , value))
        {
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        }
        else
        {
             return "$-1\r\n"; 
        }
    }
}

static std::string handleRpop(const std::vector<std::string>& tokens , RedisDatabase& db)
{    if(tokens.size() < 2)
    {
        return "-Error : RPOP Requires Key\r\n";
    }
    else
    {
        std::string value ;
        if(db.rpop(tokens[1] , value))
        {
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        }
        else
        {
             return "$-1\r\n"; 
        }
    }
}

static std::string handleLrem(const std::vector<std::string>& tokens , RedisDatabase& db)
{   if(tokens.size() < 4 )
    {
        return "-Error : LREM Requires Key , count and Value\r\n";
    }
    else
    {   
        try
        {
             
        int cnt = std::stoi(tokens[2]);
        int removed = db.lrem(tokens[1] , tokens[3] ,  cnt  );
        return ":" + std:: to_string(removed) + "\r\n";

        }
        catch(const std::exception& e)
        {
           return "-Error : Invalid Count\r\n";
        }
    }   
}

static std::string handleLindex(const std::vector<std::string>& tokens , RedisDatabase& db)
{   if(tokens.size() < 3)
    {
        return "-Error : LINDEX Requires Key , Index\r\n";
    }
    else
    {
        try
        {
            int index = std::stoi(tokens[2]);
            std::string Value;
            if(db.lindex(tokens[1] , index , Value))
            {
                return "$" +  std::to_string(Value.size()) + "\r\n" + Value + "\r\n";
                
            }
            else
            {
                return "$-1\r\n";
            }
        }
        catch(const std::exception& e)
        {
           return "-Error : Invalid Index\r\n";
        }
    }
}

static std::string handleLset(const std::vector<std::string>& tokens , RedisDatabase& db)
{   if(tokens.size() < 4)
    {
        return "-Error : LSET Requires Key , Index and Value\r\n";
    }

    try
    {
        int Index = std::stoi(tokens[2]);
        if(db.lset(tokens[1] , Index , tokens[3]))
        {
            return "+OK\r\n";
        }
        else
        {
            return "-Error : Invalid Index Out of Range";
        }

    }
    catch(const std::exception& e)
    {
       return "-Error : INVALID OR INTERNAL SERVER ERROR OCCURED\r\n";
    }
    
}


// Hash Operations

static std::string handleHset(const std::vector<std::string>& tokens , RedisDatabase& db )
{
    if(tokens.size() < 4 )
    {
        return "-Error : HSET Requires key , field , value\r\n";
    }

    bool possible = db.hset(tokens[1] , tokens[2] , tokens[3]);

    return "+1\r\n";
}


static std::string handleHget(std::vector<std::string>& tokens , RedisDatabase&db)
{
    if(tokens.size() != 3)
    {
        return "-Error : HGET Requires Key and Field.\r\n";
    }

    std::string value;
    bool valGet = db.hget(tokens[1] , tokens[2] , value);
    if(!valGet)
    {
        return "$-1\r\n";
    }

    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
}


static std::string HanldeHexist(std::vector<std::string>& tokens , RedisDatabase& db)
{
     if(tokens.size() < 3 )
    {
        return "-Error : HEXIST Requires key and field.\r\n";
    }

    bool exists = db.hexist(tokens[1] , tokens[2]);

    return ":" + std::to_string(exists ? 1 : 0 ) + "\r\n";

} 

static std::string HandleHdel(std::vector<std::string>& tokens , RedisDatabase& db)
{
    if(tokens.size() < 3 )
    {
        return "-Error : HDEL Requires key and field.\r\n";
    }
        bool h = db.hdel(tokens[1] , tokens[2]);
    return ":" + std::to_string(h ? 1 : 0 ) + "\r\n";
}

static std::string HandleHgetAll(std::vector<std::string>& tokens , RedisDatabase& db)
{

    if(tokens.size() < 2 )
    {
        return "-Error : HGETALL Requires key.\r\n";
    }
    
    std::unordered_map<std::string , std::string> results = db.hgetAll(tokens[1]);
  std::ostringstream oss;
  oss << "*" << results.size()*2 << "\r\n";
  for(const auto& p : results)
  {
    oss << "$" << p.first.size() << "\r\n" << p.first << "\r\n";
    oss << "$" << p.second.size() << "\r\n" << p.second << "\r\n";
  }
   return oss.str();

}
static std::string HandleHkeys(std::vector<std::string>& tokens ,  RedisDatabase& db)
{
     
    if(tokens.size() < 2 )
    {
        return "-Error : HKEYS Requires key.\r\n";
    }
    std::vector<std::string> keys = db.hkeys(tokens[1]);
    std::ostringstream oss;
    oss << "*" << keys.size() << "\r\n";
    for(auto i : keys)
    {
        oss << "$" << i.length() << "\r\n" << i << "\r\n";
    }

    return oss.str();
}

static std::string HandleHVals(std::vector<std::string>& tokens , RedisDatabase& db)
{
    if(tokens.size() < 2 )
    {
        return "-Error : HVALS Requires key.\r\n";
    }
     std::vector<std::string> vals = db.hvals(tokens[1]);
    std::ostringstream oss;
    oss << "*" << vals.size() << "\r\n";
    for(auto i : vals)
    {
        oss << "$" << i.length() << "\r\n" << i << "\r\n";
    }

    return oss.str();
}

static std::string HandleHmSet(std::vector<std::string>& tokens , RedisDatabase& db)
{
    if(tokens.size() < 4 || (tokens.size() % 2 == 1) )
    {
        return "-Error : HMSET Requires key followd by field and Value as a pair.\r\n";
    }

    std::vector<std::pair<std::string , std::string > > fieldVals;
    for(size_t i = 2 ; i < tokens.size() ; i +=2 )
    {
        fieldVals.push_back({tokens[i] , tokens[i + 1]});
    }

    return "+OK\r\n";

}

static std::string HandleHlen(std::vector<std::string>& tokens , RedisDatabase& db)
{
  if( tokens.size() < 2 ) 
  {
    return "-Error : HLEN Requires Key\r\n";
  }

  int sizeLen  = db.hlen(tokens[1]);

  return ":" + std::to_string(sizeLen) +  "\r\n";
  
}

static std::string handleLGET(std::vector<std::string>&tokens , RedisDatabase& db)
{
    if(tokens.size() < 2 )
    {
        return "-Error : LGET Requires Key\r\n";
    }
    std::vector<std::string>  elems = db.lget(tokens[1]);
    std::ostringstream oss;
    if(elems.size() == 0) return "-Error : Invalid Key List Doesn't Exist\r\n";
    oss << "*" << elems.size() << "\r\n";
    for(std::string elem : elems )
    {
        oss << "$" << std::to_string(elem.length()) << "\r\n" << elem << "\r\n";
    }

    return oss.str();
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
    else if( cmd == "LLEN" )
    {
        return HandleLen(tokens , Db_Instance);
    }
    else if( cmd == "LPUSH")
    {
        return handleLpush(tokens , Db_Instance);
    }
    else if( cmd == "RPUSH")
    {
        return handleRpush(tokens , Db_Instance);
    }
    else if( cmd == "LPOP")
    {
        return handleLpop(tokens , Db_Instance);
    }
    else if( cmd == "RPOP")
    {
        return handleRpop(tokens , Db_Instance);
    }
    else if( cmd == "LREM")
    {
        return handleLrem(tokens , Db_Instance);
    }
    else if( cmd == "LINDEX")
    {
        return handleLindex(tokens , Db_Instance);
    }
    else if( cmd == "LSET")
    {
        return handleLset(tokens , Db_Instance);
    }
    else if( cmd == "LGET")
    {
        return handleLGET(tokens , Db_Instance);
    }




    // Hash Operation Commands
    else if(cmd == "HSET")
    {
        return handleHset(tokens , Db_Instance);
    }
    else if( cmd == "HGET" )
    {
        return handleHget(tokens , Db_Instance);
    }
    else if(cmd == "HEXISTS")
    {
        return HanldeHexist(tokens , Db_Instance);
    }
    else if(cmd == "HDEL")
    {
        return HandleHdel(tokens , Db_Instance);
    }
    else if(cmd == "HGETALL")
    {
        return HandleHgetAll(tokens , Db_Instance);
    }
    else if( cmd == "HKEYS") return HandleHkeys(tokens , Db_Instance);
    else if(cmd == "HVALS") return HandleHVals(tokens , Db_Instance);
    else if( cmd == "HLEN") return HandleHlen(tokens , Db_Instance);
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
