#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    PROTO_HELLO,
} proto_type_e;

// Type Length Value
typedef struct {
    proto_type_e type;
    unsigned short len;
} proto_hdr_t;

void handle_client(int fd) {
    char buf[4096] = {0};
    proto_hdr_t *hdr = (proto_hdr_t*)buf;
    hdr->len = sizeof(int);
    hdr->type = htonl(PROTO_HELLO);
    int reallen = hdr->len;
    hdr->len = htons(hdr->len);

    int *data = (int*)&hdr[1];
    *data = htonl(1);

    write(fd, hdr, sizeof(proto_hdr_t) + reallen);
}

int main() {

    struct sockaddr_in serverInfo = {0};
    struct sockaddr_in clientInfo = {0};
    int clientSize = 0;

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = 0;
    serverInfo.sin_port = htons(5555);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    // Bind
    if (bind(fd, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == -1 ) {
        perror("bind");
        close(fd);
        return -1;
    }

    // Listen
    if (listen(fd, 0) == -1 ) {
        perror("listen");
        close(fd);
        return -1;
    }

    // Infinite loop made for testing
    while(1) {
        // Accept
        int cfd = accept(fd, (struct sockaddr*)&clientInfo, &clientSize);
        if (cfd == -1) {
            perror("accept");
            return -1;
        }

        handle_client(cfd);

        close(cfd);
    }

}