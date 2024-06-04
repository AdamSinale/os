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

#define SOCKET_PATH "/tmp/unix_socket_path"

int start_unix_server(char mode, char args[9]) {
    printf("server received mode %c\n", mode);
    printf("bot will play %s\n", args);

    int server_fd, new_socket;
    struct sockaddr_un address;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    unlink(SOCKET_PATH); // Remove any existing socket file

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("accepted with mode %c\n", mode);
    sleep(1);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
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
        write(new_socket, "1", 1);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}

int start_unix_client(char mode) {
    int sock;
    struct sockaddr_un serv_addr;
    char buffer[255];
    fd_set readfds;
    struct timeval tv;
    int retval;

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        close(sock);
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
        if (retval == -1) {
            perror("select()");
        } else if (retval) {
            memset(buffer, 0, sizeof(buffer));
            size_t bytesRead = read(sock, buffer, sizeof(buffer) - 1);
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s -e <command> [-i UNIX] [-o UNIX]\n", argv[0]);
        return 1;
    }

    int both_fd = -1;
    char mode = *(argv[3] + 1);
    printf("mode: %c\n", mode);
    fflush(stdout);
    char command[9];

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            strncpy(command, argv[2] + 4, 10);
            command[9] = '\0';
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            printf("received i\n");
            if (strncmp(argv[i + 1], "UNIX", 4) == 0) {
                start_unix_server(mode, command);
                printf("starting server\n");
            }
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            if (strncmp(argv[i + 1], "UNIX", 4) == 0) {
                start_unix_client(mode);
            }
        }
    }
    dup2(both_fd, STDIN_FILENO);
    dup2(both_fd, STDOUT_FILENO);
}
