#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
	// int sfd = -1;
    int sfd;
    struct sockaddr_in serverAddr = {0};
    int opt = 1;

	// Create the socket
    // The socket() function is returning zero, which indicates that a nonsocket
	// if ((sfd = socket(AF_INET, SOCK_STREAM, 0) == 0)) {
    // if ((sfd = socket(AF_INET, SOCK_STREAM, 0) < 0)) {
    //     perror("socket failed");
    //     exit(EXIT_FAILURE);
    // }
    // sfd;

    // This fixed the issue
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    sfd;

	// Expected setsockopt(3, SOL_SOCKET, SO_REUSEADDR, [1], 4) = 0
	// setsockopt(0, SOL_SOCKET, SO_REUSEADDR, [1], 4) = -1 ENOTSOCK (Socket operation on non-socket)
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(sfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	if (listen(sfd, 10) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("Listening on port %d\n", PORT);

	// int cfd = -1;
	// int addrlen = sizeof(serverAddr);
    int cfd;
    socklen_t addrlen = sizeof(serverAddr);

	// Why would cfd change from -1 to accept()?
	// if ((cfd = accept(sfd, (struct sockaddr *)&serverAddr, &addrlen)) < 0) {
	// 	perror("accept");
	// 	exit(EXIT_FAILURE);
	// }
    if ((cfd = accept(sfd, (struct sockaddr *)&serverAddr, &addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

	printf("Connection established\n");
	close(cfd);



    return 0;
}