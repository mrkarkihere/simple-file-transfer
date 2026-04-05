#include "shared.h"

// util fnctions
ssize_t getFileSize(const char* file_name){
    FILE* file_ptr = fopen(file_name, "rb");
    if(file_ptr == nullptr) return -1;
    
    fseek(file_ptr, 0, SEEK_END);
    ssize_t size = ftell(file_ptr);
    fclose(file_ptr);
    return size;
}

int connect_to_server(const char* ip, int port) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) return -1;

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    if(connect(client_fd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) == -1) {
        close(client_fd);
        return -1;
    }

    return client_fd;
}

// transmit bytes to destination
void stream_file_bytes(int fd, const char* source_path, ssize_t file_size) {
    FILE* file_ptr = fopen(source_path, "rb");
    if (!file_ptr) {
        std::cout << "Failed to open source file for reading" << std::endl;
        return;
    }

    ssize_t total_bytes = 0;
    char buffer[CHUNK_SIZE];

    while(total_bytes < file_size) {
        // throw all bytes in buffer up to chhunk size
        ssize_t bytes_read = fread(buffer, sizeof(char), CHUNK_SIZE, file_ptr);
        
        if (bytes_read <= 0) break; // either failed to raed (-1) or done (0)

        if(send(fd, buffer, bytes_read, 0) <= 0) {
            std::cout << "Socket send failed" << std::endl;
            break;
        }

        total_bytes += bytes_read;
        std::cout << "Progress: " << total_bytes << "/" << file_size << " bytes..." << std::endl;
    }

    fclose(file_ptr);
}

//
int validate_cli(int argc, char** argv){
    // need source + destination
    if(argc < 3){
        std::cout << "Usage: " << argv[0] << " <source> <destination>" << std::endl;
        return -1;
    }
    const char* source = argv[1];
    FILE* file_ptr = fopen(source, "rb");
    if(file_ptr == nullptr){
        std::cerr << "Source file not found: " << source << std::endl;
        return -1;
    }

    fclose(file_ptr);
    return 0;
}

//
int main(int argc, char* argv[]) {
    /*
        i guess i'll do it like scp, template:
        scp source destination

        EXAMPLE: ./client ../plaintext.txt /destination/output.txt
    */
    if(validate_cli(argc, &argv[0]) != 0){
        return 1;
    }

    const char* source = argv[1]; // "source/plaintext.txt"
    const char* destination = argv[2]; // "destination/output.txt"
    
    const char* server_ip = "127.0.0.1";
    int port = 8080;

    int client_fd = connect_to_server(server_ip, port);
    
    if (client_fd != -1) {
        std::cout << "Successfully connected to server!" << std::endl;

        // file metadat, update this to use FTP standrd protocol
        struct file_header file_metadata;
        strncpy(file_metadata.source, source, sizeof(file_metadata.source));
        strncpy(file_metadata.destination, destination, sizeof(file_metadata.destination));
        file_metadata.file_size = getFileSize(source);

        if (file_metadata.file_size == -1) {
            std::cout << "Source file not found" << std::endl;
            close(client_fd);
            return 1;
        }

        // handshake
        send(client_fd, &file_metadata, sizeof(file_metadata), 0);
        
        // transit data
        stream_file_bytes(client_fd, source, file_metadata.file_size);

    } else {
        std::cout << "Connection to server failed" << std::endl;
    }

    close(client_fd);
    return 0;
}