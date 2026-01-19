#include "server.hpp"
#include "parser.hpp"

#include <iostream> 
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

db::TCPServer::TCPServer(int port_): server_fd_(-1), port_(port_) {}
db::TCPServer::~TCPServer() {
    if(server_fd_ >= 0) { close(server_fd_); }
}

void db::TCPServer::setup_socket() {
    if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) { perror("socket failed"); exit(EXIT_FAILURE); }
    
    int opt = 1; 
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd_, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        perror("bind failed"); exit(EXIT_FAILURE); 
    }

    if (listen(server_fd_, 5) < 0) { perror("listen failed"); exit(EXIT_FAILURE); }

    std::cout << "Server listening on port " << port_ << std::endl;
}

void db::TCPServer::start_server(){
    setup_socket();

    while (true) { 
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len); 
        if (client_fd < 0) { perror("accept failed"); continue; }

        handle_request(client_fd);

        close(client_fd);
    }
}

void db::TCPServer::handle_request(int client_fd){
    char buffer[1024]; 
    size_t buffsize = sizeof(buffer);
    std::string request;

    ssize_t received_size{};
    for (size_t i = 0; ; i += received_size) {
        received_size = recv(client_fd, buffer + i, buffsize - i, 0);

        if(received_size > 0){
            request.append(buffer + i, received_size);

            if(Request entry = parse_request(request); entry.status == PARSE_OK){
                request.clear(); //still buggy when partial inputs come but works for now
                //save in db later
            }
            // Example response, temp
            std::string response = "OK\n" + request;
            if (send(client_fd, response.c_str(), response.size(), 0) < 0) { perror("send failed"); break; }
        } else if(received_size == 0){
            std::cout << "Client disconnected." << std::endl; break;
        } else {
            std::cerr << "recv failed: " << strerror(errno) << std::endl; break;
        }

        if (i + received_size >= buffsize) { i = 0; }
    }
}
