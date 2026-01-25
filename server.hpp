#pragma once

#include <iostream>
#include <unordered_map>

namespace db{
    class TCPServer{
        public:
            TCPServer(int port);
            ~TCPServer();
        private:
            int port_; 
            int server_fd_;
            int epoll_fd_;

            std::unordered_map<int, std::string> client_buffer;

            void setup_socket();
            void handle_request(int client_fd);
        public:
            void start_server();
    };
} //namespace db
