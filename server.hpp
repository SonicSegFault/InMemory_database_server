#pragma once

namespace db{
    class TCPServer{
        public:
            TCPServer(int port);
            ~TCPServer();
        private:
            int port_; 
            int server_fd_;
            int epoll_fd_;

            void setup_socket();
            void handle_request(int client_fd);
        public:
            void start_server();
    };
} //namespace db
