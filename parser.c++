#include "parser.hpp"

#include <sstream>

Request parse_request(std::string& request) {
    std::istringstream iss(request);
    size_t current_pos = 0;

    // Parse array header 
    std::string arrayline;
    if (!std::getline(iss, arrayline, '\n')) {
        return {PARSE_INCOMPLETE}; // not enough data yet
    }
    current_pos += arrayline.size() + 1; // +1 for '\n'
    if (!arrayline.empty() && arrayline.back() == '\r') arrayline.pop_back();

    if (arrayline.empty() || arrayline[0] != '*')
        perror("Invalid RESP: expected array");

    int numElems = std::stoi(arrayline.substr(1));

    Request entry;
    entry.status = PARSE_INCOMPLETE;

    // Parse each bulk string
    for (int i = 0; i < numElems; i++) {
        std::string line;
        if (!std::getline(iss, line, '\n')) return entry; // incomplete
        current_pos += line.size() + 1; 
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.empty() || line[0] != '$') perror("Invalid RESP: expected bulk string");

        int len = std::stoi(line.substr(1));

        std::string arg(len, '\0');
        if (!iss.read(&arg[0], len)) return entry; // incomplete bulk string
        current_pos += len;

        // Consume trailing CRLF
        char cr, lf;
        if (!iss.get(cr) || !iss.get(lf)) return entry; // incomplete
        current_pos += 2;
        if (cr != '\r' || lf != '\n') perror("Invalid RESP: expected CRLF after bulk string");

        if (i == 0) entry.name = arg;
        else entry.args.push_back(arg);
    }

    entry.offset = current_pos;
    entry.status = PARSE_OK;
    return entry;
}

