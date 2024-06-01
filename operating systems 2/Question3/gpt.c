#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

char *path = "../Question1/ttt";
// Function to start a TCP server
int start_tcp_server(int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

// Function to start a TCP client
int start_tcp_client(const char *hostname, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *he;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    if ((he = gethostbyname(hostname)) == NULL) {
        perror("Invalid hostname");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    return sock;
}

// Function to execute the given command with input/output redirection
void execute_command(char *cmd[], int input_fd, int output_fd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        char *args1[] = {"ttt", "123456789", NULL};
        char *path1 = "../Question1/ttt";
        execvp(path1, args1);
        exit(EXIT_FAILURE);
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, NULL, 0);
        if (input_fd != -1) close(input_fd);
        if (output_fd != -1) close(output_fd);
    }
}

int main(int argc, char *argv[]) {
    int input_fd = -1;
    int output_fd = -1;
    char *command[argc - 1];
    int cmd_index = 0;
    int isServer = 0;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            char *token = strtok(argv[++i], "TCPS");
            int port = atoi(token);
            input_fd = start_tcp_server(port);
            isServer = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            char *token = strtok(argv[++i], "TCPC");
            char *hostname = strtok(token, ",");
            int port = atoi(strtok(NULL, ","));
            output_fd = start_tcp_client(hostname, port);
        } else if (strcmp(argv[i], "-b") == 0) {
            char *token = strtok(argv[++i], "TCPS");
            int port = atoi(token);
            input_fd = start_tcp_server(port);
            output_fd = input_fd;
            isServer = 1;
        } else if (strcmp(argv[i], "-e") == 0) {
            i++;
            while (i < argc) {
                command[cmd_index++] = argv[i++];
            }
            command[cmd_index] = NULL;
        }
    }

    // Execute the command with the necessary redirections
    if (isServer) {
        execute_command(command, input_fd, output_fd);
    } 
    else{
        while (1)
        {
            char buffer[1024];
            ssize_t bytes_read = read(input_fd, buffer, 1024);
            if (bytes_read <= 0) {
                break;
            }
            buffer[bytes_read] = '\0';
            printf("Received from client: %s\n", buffer);
        }
        
    }

    return 0;
}
