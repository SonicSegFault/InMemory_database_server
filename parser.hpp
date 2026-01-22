#pragma once

#include <vector>
#include <string>

enum ParseStatus{
    PARSE_OK = 1, 
    PARSE_INCOMPLETE = 0,
    PARSE_ERROR = -1
};

struct Request
{   
    int status;
    size_t offset; //to indicate how many bytes have been parsed
    std::string name; // e.g. "SET", "GET", "PING"    
    std::vector<std::string> args; // arguments
};

Request parse_request(std::string& request);