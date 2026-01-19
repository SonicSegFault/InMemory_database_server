#include "server.hpp"
#include <iostream>

int main() {
    try {
        db::TCPServer server(6666);

        std::cout << "Starting database server on port 6666..." << std::endl;
        server.start_server();  
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
