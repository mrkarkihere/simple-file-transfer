#include "shared.h"
#include <thread>


int init_server(int port) {
    // create the socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, TCP, default (tcp)
    if (server_fd == -1) return -1;

    // server address
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET; // tcp
    serveraddr.sin_port = htons(port); 
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // cli arg in the future

    // assign an address to listen in
    if(bind(server_fd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) == -1){
        std::cout << "Binding failed!\n";
        return -1;
    }

    // which socket to listen to, queue length on backlog
    if(listen(server_fd, 3) == -1){
        std::cout << "Listen call failed!\n";
        return -1;
    }    
    return server_fd;
}


void receive_file_content(int fd, const struct file_header& file_metadata) {
    // just create the output file, dont write to it yet
    FILE* file_ptr = fopen(file_metadata.destination, "wb");
    if (!file_ptr) {
        std::cout << "Failed to open destination file." << std::endl;
        return;
    }

    ssize_t total_bytes = 0;
    // receive bytes
    while(total_bytes < file_metadata.file_size){
        char buffer[CHUNK_SIZE];
        ssize_t bytes_recv = recv(fd, &buffer[0], sizeof(buffer), 0);
        
        // connection ended or receive failed
        if(bytes_recv <= 0){
            break;
        }
        
        fwrite(buffer, sizeof(char), bytes_recv, file_ptr);
        total_bytes += bytes_recv;
    }

    if(total_bytes == file_metadata.file_size){
        std::cout << "Received all bytes correctly" << std::endl;
    } else {
        std::cout << "Transfer incomplete: " << total_bytes << "/" << file_metadata.file_size << std::endl;
    }
    
    fclose(file_ptr);
}

//
void handle_client(int fd, struct sockaddr_in addr, socklen_t addrlen){
    struct file_header file_metadata;
    // handshake
    ssize_t header_bytes = recv(fd, &file_metadata, sizeof(file_metadata), 0);

    if(header_bytes > 0){
        std::cout << "Source: " << file_metadata.source << std::endl;
        std::cout << "Destination: " << file_metadata.destination << std::endl;
        std::cout << "File Size: " << file_metadata.file_size << " bytes" << std::endl;
        
        // header received, now move to receiving data
        receive_file_content(fd, file_metadata);
    }
    else{
        std::cout << "Didn't receive metadata correctly" << std::endl;
    }

    close(fd);
}


int main(){
    int server_fd = init_server(8080);
    
    if(server_fd < 0){
        std::cout << "Server init failed" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    while(true){
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        
        // blocking call
        int client_fd = accept(server_fd, (struct sockaddr*) &clientaddr , &addrlen); 

        if(client_fd != -1){
            std::cout << "Connection established with FD " << client_fd << "\n";
            // thread to handle each client
            std::thread t(handle_client, client_fd, clientaddr, addrlen);
            t.detach();
        }
    }

    close(server_fd);
    return 0;
}