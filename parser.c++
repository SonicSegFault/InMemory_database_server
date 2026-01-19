#include "parser.hpp"

#include <sstream>

Request parse_request(const std::string& request){
    std::istringstream iss(request); 
    char type; iss >> type;
    
    if (type != '*') { throw std::runtime_error("Invalid RESP: expected array"); }

    int numElems; iss >> numElems; iss.ignore(2); // skip \r\n

    Request entry;
    entry.status = PARSE_INCOMPLETE; 
    for(int i{}; i < numElems; i++){
        iss >> type;
        if(type != '$') { throw std::runtime_error("Invalid RESP: expected bulk string"); }

        int len; iss >> len; iss.ignore(2);

        std::size_t curr_idx = static_cast<size_t>(iss.tellg());
        if (request.size() <  curr_idx + len + 2) { return entry; }
        
        std::string arg(len, '\0');
        iss.read(&arg[0], len);
        iss.ignore(2);

        if(i == 0) { entry.name = arg; }
        else { entry.args.push_back(arg); }
    }

    entry.status = PARSE_OK;
    return entry;
}

