#pragma once

#include <vector>
#include <string>

enum ParseStatus{
    PARSE_OK = 1, 
    PARSE_INCOMPLETE = 0,
    PARSE_ERROR = -1
};

typedef struct Request
{   
    int status;
    std::string name; // e.g. "SET", "GET", "PING"    
    std::vector<std::string> args; // arguments
};

Request parse_request(const std::string& request);