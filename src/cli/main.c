#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"

int send_hello(int fd) {
    char buf[4096] = {0};

    // Removed for debugging
    // dbproto_hdr_t *hdr = buf;
    dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
    hdr->type = MSG_HELLO_REQ;
    hdr->len = 1;

    // Send the hello request with the version
    dbproto_hello_req* hello = (dbproto_hello_req*)&hdr[1];
    hello->proto = PROTO_VER;

    // Write the hello message
    write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_req));

    // Receive the response
    read(fd, buf, sizeof(buf));

    hdr->type = htonl(hdr->type);
    hdr->len = htons(hdr->len);
    // Removed for debugging
    // hello->proto = htons(hello->proto);

    /// Removed for debugging
    // write(fd, buf, sizeof(dbproto_hdr_t));

    if (hdr->type == MSG_ERROR) {
        printf("Protocol mismatch.\n");
        close(fd);
        return STATUS_ERROR;
    }

    printf("Server connected, protocol v1.\n");
    return STATUS_SUCCESS;
}


int main(int argc, char *argv[]) {

    // Removed for debugging
    // if (argc != 2) {
    //     printf("Usage: %s <ip of the host>\n", argv[0]);
    //     return 0;
    // }

    char *addarg = NULL;
    char *portarg = NULL;
    char *hostarg = NULL;
    unsigned short port = 0;

    // Added for debugging
    char *optarg = NULL;

    // TODO define optarg
    int c;
    while ((c = getopt(argc, argv, "p:h:a:")) != -1) {
        switch (c) {
            case 'a':
                addarg = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                break;
            case 'h':
                hostarg = optarg;
                break;
            case '?':
                printf("Unknown option =%c\n", c);
            default:
                return -1;
        }
    }

    if (port == 0) {
        printf("Bad port: %s\n", portarg);
        return -1;
    }

    if (hostarg == NULL) {
        printf("Must specify host with -h\n");
        return -1;
    }

    struct sockaddr_in serverInfo = {0};
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(hostarg);
    serverInfo.sin_port = htons(port);

    // Removed for debugging
    // serverInfo.sin_addr.s_addr = inet_addr(argv[1]);


    // Removed for debugging
    // serverInfo.sin_addr.s_addr = in6_addr(argv[1]);

    // Removed for debugging
    // serverInfo.sin_port = htons(8080);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    // Removed for debugging
    // if (connect(fd, (struct socketaddr*)&serverInfo, sizeof(serverInfo)) == -1) {
    if (connect(fd, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == -1) {
        perror("connect");
        close(fd);
        return 0;
    }

    if(send_hello(fd) != STATUS_SUCCESS) {
        return -1;
    }

    close(fd);

}