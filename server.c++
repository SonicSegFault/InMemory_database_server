#include "server.hpp"
#include "parser.hpp"

#include <iostream> 
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

db::TCPServer::TCPServer(int port_): port_(port_), server_fd_(-1), epoll_fd_(-1) {}
db::TCPServer::~TCPServer() {
    if(server_fd_ >= 0) { close(server_fd_); }
}

void db::TCPServer::setup_socket() {
    if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) { perror("socket failed"); exit(EXIT_FAILURE); }
    fcntl(server_fd_, F_SETFL, O_NONBLOCK);

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

    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) { perror("epoll_create1 failed"); exit(EXIT_FAILURE); }

    epoll_event server_event{};
    server_event.events = EPOLLIN;
    server_event.data.fd = server_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &server_event) == -1) {
        perror("epoll_ctl failed"); exit(EXIT_FAILURE);
    }
}

void db::TCPServer::start_server(){
    setup_socket();

    const int MAX_EVENTS = 64; epoll_event events[MAX_EVENTS];

    while (true) { 
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (n == -1) { 
            if(errno == EINTR) continue;
            perror("epoll_wait failed"); 
            break;
        }
        for(int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == server_fd_) {
                while(true){
                    sockaddr_in client_address{};
                    socklen_t client_len = sizeof(client_address);
                    int client_fd{};
                    if((client_fd = accept(server_fd_, (struct sockaddr*)&client_address, &client_len)) < 0){
                        if(errno == EAGAIN || errno == EWOULDBLOCK) break; 
                        else { perror("accept failed"); break; }
                    }
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    
                    epoll_event client_event{};
                    client_event.events = EPOLLIN | EPOLLET;
                    client_event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
                        perror("epoll_ctl failed"); exit(EXIT_FAILURE); close(client_fd);
                    }
                    std::cout << "Accepted new connection, fd: " << client_fd << std::endl;
                }
            } else {
                handle_request(fd);
            }
        }
    }
}

void db::TCPServer::handle_request(int client_fd){
    char buffer[1024];
    
    while(true){
        ssize_t received_size = recv(client_fd, buffer, sizeof(buffer), 0);
        if(received_size > 0){
            client_buffer[client_fd].append(buffer, received_size);

            while(true){
                Request entry = parse_request(client_buffer[client_fd]);
                if(entry.status == PARSE_OK) client_buffer[client_fd].erase(0, entry.offset); 
                else break;
                //save in db later
            }
            // Example response, temp
            std::string response = "OK\n";
            if (send(client_fd, response.c_str(), response.size(), 0) < 0) { perror("send failed"); break; }
        } else if(received_size == 0){
            std::cout << "Client disconnected, fd: " << client_fd << std::endl; 
            close(client_fd); 
            client_buffer.erase(client_fd);
            break; 
        } else {
            if(errno == EAGAIN || errno == EWOULDBLOCK) break; 
            std::cerr << "recv failed: " << strerror(errno) << std::endl; 
            close(client_fd); 
            client_buffer.erase(client_fd);
            break; 
        }
    }
}
