#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>

#define MAX_CLIENTS 256
#define PORT 8080
#define BUFF_SIZE 4096

typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED
} state_e;

// Structure to hold client state
typedef struct {
    int fd;
    state_e state;
    char buffer[4096];
} clientstate_t;

// Array of type clientstate_t that contains the maximum number of clients 256
clientstate_t clientStates[MAX_CLIENTS];

// Initialize the clients state, check for an open slot, and set client structure to zero
void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clientStates[i].fd = -1;
        clientStates[i].state = STATE_NEW;
        memset(&clientStates[i].buffer, '\0', BUFF_SIZE);
    }
}

// Iterate and to find out if the fd is free
int find_free_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientStates[i].fd == -1) {
            return i;
        }
    }
    return -1; // No free slot
}

// Find slot by the file descriptor
int find_slot_by_fd(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientStates[i].fd == fd) {
            return i;
        }
    }
    return -1; // No file descriptor found
}


int main() {
    // Listen FD
    int listen_fd;
    // Connection FD
    int conn_fd;
    // Freeslot fd
    int freeSlot;

    // IPv4 socket address
    struct sockaddr_in server_addr;
    // IPv4 socket address
    struct sockaddr_in client_addr;
    // IPv4 client socket address size
    socklen_t client_len = sizeof(client_addr);

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;
    int opt = 1;

    // Initialize client states
    init_clients();

    // Create listening socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(listen_fd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Set the memory for file descriptors to zero
    memset(fds, 0, sizeof(fds));
    // Assign the value of index 0 as a listen_fd
    fds[0].fd = listen_fd;
    // Check for a POLLIN listen event for data is available for reading
    fds[0].events = POLLIN;
    nfds = 1;


    while (1) {

        int ii = 1; // To offset the index to 1 instead of 0
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientStates[i].fd != -1) {
                fds[ii].fd = clientStates[i].fd; // Offset by 1 for listen_fd
                fds[ii].events = POLLIN;
                i++;
            }
        }

        // Wait for an event on one of the sockets
        int n_events = poll(fds, nfds, -1); // -1 blocks, meaning no timeout
        if (n_events == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        // Check for new connections
        if (fds[0].revents & POLLIN) {
            if ((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len)) == -1) {
                perror("listen");
                continue;
            }
        

            printf("New connection from %s:%d\n",
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            freeSlot = find_free_slot();
            if (freeSlot == -1) {
                printf("Server full: closing new connection\n");
                close(conn_fd);
            } else {
                clientStates[freeSlot].fd = conn_fd;
                clientStates[freeSlot].state = STATE_CONNECTED;
                nfds++;
                printf("Slot %d has fd %d\n", freeSlot, clientStates[freeSlot].fd);
            }
            
            n_events--;
        }

        // Check each client for read/write activity
        for (int i = 1; i <= nfds && n_events > 0; i++) { // Start from 1 to skip the listen
            if (fds[i].revents & POLLIN) {
                n_events--;

                int fd = fds[i].fd;
                int slot = find_free_slot(fd);
                ssize_t bytes_read = read(fd, &clientStates[slot].buffer, sizeof(clientStates[slot].buffer)); // Not sure about the last arugment in the read()
                if (bytes_read <= 0) {
                    // Connection closed or error
                    close(fd);
                    if (slot == -1) {
                        printf("Tried to close fd that doesn't exist?\n");
                    } else {
                        clientStates[slot].fd = -1; // Free up the slot
                        clientStates[slot].state = STATE_DISCONNECTED;
                        printf("Client disconnected or error\n");
                        nfds--;
                    }
                } else {
                    printf("Received data from client: %s\n", clientStates[slot].buffer);
                }
            }
        }
    }

    return 0;
}
