#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>      // For exit(), stoi()
#include <cstdint>      // For uint16_t, uint32_t
#include <sys/socket.h>
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close(), send(), recv()

const int BUFFER_SIZE = 1024;

// Manual implementation of htons (host-to-network short)
// Converts a 16-bit value from host byte order to network byte order (big-endian).
uint16_t htons_manual(uint16_t hostshort) {
    uint16_t i = 1;
    char *c = (char*)&i;
    if (*c) { // System is little-endian, need to swap
        return (hostshort >> 8) | (hostshort << 8);
    } else { // System is big-endian, no swap needed
        return hostshort;
    }
}

// Manual implementation of inet_addr.
// Parses "a.b.c.d" and constructs the big-endian integer.
// Returns INADDR_NONE ((uint32_t)-1) on error.
uint32_t inet_addr_manual(const char* ip_str) {
    uint32_t parts[4];
    uint32_t current_part = 0;
    int part_idx = 0;

    const char* p = ip_str;
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            current_part = current_part * 10 + (*p - '0');
            if (current_part > 255) return (uint32_t)-1;
        } else if (*p == '.') {
            if (part_idx >= 3) return (uint32_t)-1; // more than 3 dots
            parts[part_idx++] = current_part;
            current_part = 0;
        } else {
            return (uint32_t)-1; // invalid character
        }
        p++;
    }

    if (part_idx != 3) return (uint32_t)-1; // must have 4 parts
    parts[part_idx] = current_part;

    return (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
}

void die(const char *s) {
    perror(s);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
       std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
       return 1;
    }

    const char *server_ip = argv[1];
    int port_num = std::stoi(argv[2]);
    if (port_num < 0 || port_num > 65535) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }
    uint16_t port = (uint16_t)port_num;

    int sock;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        die("socket");
    }

    uint32_t ip_addr = inet_addr_manual(server_ip);
    if (ip_addr == (uint32_t)-1) {
        std::cerr << "Invalid IP address format" << std::endl;
        return 1;
    }

    server.sin_addr.s_addr = ip_addr;
    server.sin_family = AF_INET;
    server.sin_port = htons_manual(port);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        die("connect");
    }

    // Create a simple HTTP GET request
    std::string http_request = "GET / HTTP/1.1\r\nHost: " + std::string(server_ip) + "\r\nConnection: close\r\n\r\n";

    // Send the request
    if (send(sock, http_request.c_str(), http_request.length(), 0) < 0) {
        die("send");
    }

    // Receive a reply from the server
    std::cout << "Response from server:" << std::endl;
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the buffer
        std::cout << buffer;
    }

    if (bytes_received < 0) {
        die("recv");
    }

    close(sock);
    return 0;
}