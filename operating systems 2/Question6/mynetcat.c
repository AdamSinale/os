#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <asm-generic/socket.h>

#define BUFFER_SIZE 1024


int start_tcp_server(int port, char mode, char args[9]){
    printf("server received mode %c\n", mode);
    printf("bot will play %s\n", args);

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("accepted with mode %c\n", mode);
    sleep(1);

    // write(new_socket, mode, 1);

    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        switch (mode) {
            case 'i':
                dup2(new_socket, STDIN_FILENO);
                break;
            case 'o':
                dup2(new_socket, STDOUT_FILENO);
                break;
            case 'b':
                dup2(new_socket, STDIN_FILENO);
                dup2(new_socket, STDOUT_FILENO);
                break;
        }
        close(server_fd);
        
        char *args1[] = {"ttt", args, NULL};
        char *path = "../Question1/ttt";
        execvp(path, args1);
        printf("child came back\n");

        // If execvp fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        waitpid(pid, NULL, 0);
        printf("game finished\n");
        write(new_socket, "1",1);
        close(new_socket);
    }

    close(server_fd);
    return 0;    
}

int start_tcp_client(const char *hostname, int port, char mode) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *he;
    char buffer[255];
    fd_set readfds;
    struct timeval tv;
    int retval;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    if ((he = gethostbyname(hostname)) == NULL) {
        printf("\nInvalid hostname\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }
    printf("connection made, put input:\n");
    // Make the socket non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        retval = select(sock + 1, &readfds, NULL, NULL, &tv);
        tv.tv_usec = 0;
        if (retval == -1) { perror("select()"); } 
        else if (retval) { 
            memset(buffer, 0, sizeof(buffer));
            size_t bytesRead = read(sock, buffer, sizeof(buffer)-1);
            buffer[bytesRead] = '\0';
            if(bytesRead <= 0){
                printf("problem receiving");
                return -1;
            }
            if(bytesRead == 1){ break; }
            printf("%s\n",buffer);
            if(buffer[bytesRead-2] == 'n' || buffer[bytesRead-2] == 'W' || buffer[bytesRead-2] == 't'){ break; }
        }
    
        printf("Enter position to play (single character). (0 for exit)\n");
        char input = getchar();
        getchar(); // Consume the newline character
        write(sock, &input, 1); // Send a single character
        if (input == '0') { // Check for exit condition
            break;
        }
        printf("Sent: %c\n", input);
        fflush(stdout);
    }
    close(sock);
    return 0;
}


int start_udp_server(int port, char mode, char args[9]) {
    printf("server received mode %c\n", mode);
    printf("bot will play %s\n", args);

    int server_fd;
    struct sockaddr_in server_address, client_address;
    char buffer[1024];
    socklen_t client_len = sizeof(client_address);

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for a client to connect...\n");

    // Receive initial connection from client
    recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_len);

    printf("accepted with mode %c\n", mode);
    sleep(1);

    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        switch (mode) {
            case 'i':
                dup2(server_fd, STDIN_FILENO);
                break;
            case 'o':
                dup2(server_fd, STDOUT_FILENO);
                break;
        }
        close(server_fd);

        char *args1[] = {"ttt", args, NULL};
        char *path = "../Question1/ttt";
        execvp(path, args1);
        printf("child came back\n");

        // If execvp fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        waitpid(pid, NULL, 0);
        printf("game finished\n");
        sendto(server_fd, "1", 1, 0, (struct sockaddr *)&client_address, client_len);
        close(server_fd);
    }

    return 0;
}

int start_udp_client(const char *hostname, int port, char mode) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *he;
    char buffer[255];
    fd_set readfds;
    struct timeval tv;
    int retval;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    if ((he = gethostbyname(hostname)) == NULL) {
        printf("\nInvalid hostname\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    // Send initial connection message to server
    sendto(sock, "connect", 7, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("connection made, put input:\n");

    // Make the socket non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    socklen_t addr_len = sizeof(serv_addr);
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        retval = select(sock + 1, &readfds, NULL, NULL, &tv);
        tv.tv_usec = 0;
        if (retval == -1) {
            perror("select()");
        } else if (retval) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&serv_addr, &addr_len);
            buffer[bytesRead] = '\0';
            if (bytesRead <= 0) {
                printf("problem receiving");
                return -1;
            }
            if (bytesRead == 1) {
                break;
            }
            printf("%s\n", buffer);
        }

        printf("Enter position to play (single character). (0 for exit)\n");
        char input = getchar();
        getchar(); // Consume the newline character
        sendto(sock, &input, 1, 0, (struct sockaddr *)&serv_addr, addr_len); // Send a single character
        if (input == '0') { // Check for exit condition
            break;
        }
        printf("Sent: %c\n", input);
        fflush(stdout);
    }
    close(sock);
    return 0;
}


void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void UDSSD(const char *path, int input_fd) {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        handle_error("socket");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    unlink(path);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        handle_error("bind");
    }

    while (1) {
        ssize_t num_read = read(input_fd, buffer, BUFFER_SIZE);
        if (num_read == -1) {
            handle_error("read");
        } else if (num_read == 0) {
            break;
        }

        ssize_t num_sent = sendto(sockfd, buffer, num_read, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
        if (num_sent == -1) {
            handle_error("sendto");
        }
    }

    close(sockfd);
    unlink(path);
}

void UDSCD(const char *path, int output_fd) {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        handle_error("socket");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    while (1) {
        ssize_t num_recv = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (num_recv == -1) {
            handle_error("recvfrom");
        }

        ssize_t num_written = write(output_fd, buffer, num_recv);
        if (num_written == -1) {
            handle_error("write");
        }
    }

    close(sockfd);
}

void UDSSS(const char *path, int input_fd, int output_fd) {
    int sockfd, connfd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        handle_error("socket");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    unlink(path);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        handle_error("bind");
    }

    if (listen(sockfd, 5) == -1) {
        handle_error("listen");
    }

    if ((connfd = accept(sockfd, NULL, NULL)) == -1) {
        handle_error("accept");
    }

    while (1) {
        ssize_t num_read = read(input_fd, buffer, BUFFER_SIZE);
        if (num_read == -1) {
            handle_error("read");
        } else if (num_read == 0) {
            break;
        }

        ssize_t num_written = write(connfd, buffer, num_read);
        if (num_written == -1) {
            handle_error("write");
        }

        ssize_t num_recv = read(connfd, buffer, BUFFER_SIZE);
        if (num_recv == -1) {
            handle_error("read");
        } else if (num_recv == 0) {
            break;
        }

        num_written = write(output_fd, buffer, num_recv);
        if (num_written == -1) {
            handle_error("write");
        }
    }

    close(connfd);
    close(sockfd);
    unlink(path);
}

void UDSCS(const char *path, int input_fd, int output_fd) {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        handle_error("socket");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        handle_error("connect");
    }

    while (1) {
        ssize_t num_read = read(input_fd, buffer, BUFFER_SIZE);
        if (num_read == -1) {
            handle_error("read");
        } else if (num_read == 0) {
            break;
        }

        ssize_t num_written = write(sockfd, buffer, num_read);
        if (num_written == -1) {
            handle_error("write");
        }

        ssize_t num_recv = read(sockfd, buffer, BUFFER_SIZE);
        if (num_recv == -1) {
            handle_error("read");
        } else if (num_recv == 0) {
            break;
        }

        num_written = write(output_fd, buffer, num_recv);
        if (num_written == -1) {
            handle_error("write");
        }
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Server Usage1: %s -e \"ttt 123456789\" -i TCPS4050\n", argv[0]);
        fprintf(stderr, "Server Usage2: %s -e \"ttt 123456789\" -b TCPS4050\n", argv[0]);
        fprintf(stderr, "Client Usage: %s -e \"ttt 123456789\" -o TCPClocalhost,4050\n\n", argv[0]);
        fprintf(stderr, "Server Usage: %s -e \"ttt 123456789\" -i UDPS4050\n", argv[0]);
        fprintf(stderr, "Client Usage: %s -e \"ttt 123456789\" -o UDPClocalhost,4050\n\n", argv[0]);
        fprintf(stderr, "Server Usage1: %s -e \"ttt 123456789\" -i UDSSD\"file.txt\"\n", argv[0]);
        fprintf(stderr, "Client Usage1: %s -e \"ttt 123456789\" -o UDSCD\"file.txt\"\n", argv[0]);
        fprintf(stderr, "Server Usage2: %s -e \"ttt 123456789\" -i UDSSS\"file.txt\"\n", argv[0]);
        fprintf(stderr, "Client Usage2: %s -e \"ttt 123456789\" -o UDSCS\"file.txt\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    int both_fd = -1;
    char mode = *(argv[3] + 1);
    printf("mode: %c\n", mode);
    fflush(stdout);
    char command[9];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            strncpy(command, argv[2] + 4, 10);
            command[9] = '\0';
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            if (strncmp(argv[i + 1], "UDPS", 4) == 0) {
                int port = atoi(argv[++i] + 4);
                start_udp_server(port, mode, command);
                printf("starting server with port: %d", port);
                return 0;
            }
            if (strncmp(argv[i + 1], "TCPS", 4) == 0) {
                int port = atoi(argv[++i] + 4);
                start_tcp_server(port, mode, command);
                printf("starting server with port: %d", port);
                return 0;
            }
            if (strncmp(argv[i + 1], "UDSSD", 5) == 0) {
                UDSSD(&argv[++i][5], input_fd);
            } else if (strncmp(argv[i + 1], "UDSSS", 5) == 0) {
                UDSSS(&argv[++i][5], input_fd, output_fd);
            } else {
                fprintf(stderr, "Invalid -i option\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            if (strncmp(argv[i + 1], "UDPC", 4) == 0) {
                char *host_port = argv[++i] + 4;
                char *host = strtok(host_port, ",");
                if (host != NULL) {
                    int port = atoi(strtok(NULL, ","));
                    start_udp_client(host, port, mode);
                }
                return 0;
            }
            if (strncmp(argv[i + 1], "TCPC", 4) == 0){
                char *host_port = argv[++i] + 4;
                char *host = strtok(host_port, ",");
                if (host != NULL){
                    int port = atoi(strtok(NULL, ","));
                    start_tcp_client(host, port, mode);
                }
                return 0;
            }
            if (strncmp(argv[i + 1], "UDSCD", 5) == 0) {
                UDSCD(&argv[++i][5], output_fd);
            } else if (strncmp(argv[i + 1], "UDSCS", 5) == 0) {
                UDSCS(&argv[++i][5], input_fd, output_fd);
            } else {
                fprintf(stderr, "Invalid -o option\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc){
            if (strncmp(argv[i + 1], "TCPS", 4) == 0){
                int port = atoi(argv[++i] + 4);
                both_fd = start_tcp_server(port, mode, command);
                return 0;
            }
        }
    }


    // Execution logic for command
    pid_t pid = fork();
    if (pid < 0) {
        handle_error("fork");
    }

    if (pid == 0) { // Child process
        if (mode == 'i') {
            dup2(STDIN_FILENO, STDOUT_FILENO);
        } else if (mode == 'o') {
            dup2(STDOUT_FILENO, STDIN_FILENO);
        }
        execlp("ttt", command, (char *)NULL);
        handle_error("execlp");
    } else {
        wait(NULL);
    }

    return 0;
}
