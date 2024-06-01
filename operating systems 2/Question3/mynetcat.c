#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <bits/getopt_core.h>
#include <asm-generic/socket.h>

char *path = "../Question1/ttt";
#define BUFFER_SIZE 1024

void start_tcp_server(int port, int *fd, char **args)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    *fd = new_socket;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        dup2(new_socket, STDIN_FILENO);
        dup2(new_socket, STDOUT_FILENO);
        printf("%s, %s\n", path, args[0]);
        execvp(path, args);
        perror("exec failed");
    }

    else
    {
        char buffer[BUFFER_SIZE];
        while (1)
        {
            ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0)
            {
                break; // Exit loop on read error or client disconnect
            }
            buffer[bytes_read] = '\0';
        }
    }
    waitpid(pid, NULL, 0);
    close(fd);
    close(server_fd);
    return;
}

void start_tcp_client(const char *hostname, int port, int *fd)
{
    int client_fd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    if ((server = gethostbyname(hostname)) == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    *fd = client_fd;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        dup2(client_fd, STDOUT_FILENO);
        while (1) {
            ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break; // Exit loop on read error or server disconnect
            }
            buffer[bytes_read] = '\0';
            printf("Received from server: %s\n", buffer);
        }
        close(fd);
        exit(0);
    }

    else
    {
        char buffer[BUFFER_SIZE];
        while (1)
        {
            ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0)
            {
                break; // Exit loop on read error or client disconnect
            }
            buffer[bytes_read] = '\0';
            write(fd, buffer, strlen(buffer));
        }
                close(fd);
        #include <signal.h>

        kill(pid, SIGKILL); // Kill child process reading from server
    }
    return;
}


char **split_command(const char *command)
{
    // Allocate space for the argument array
    char **argv = malloc(3 * sizeof(char *));
    if (!argv)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    int argc = 0;
    // Create a copy of the command because strtok modifies the original string
    char *cmd_copy = malloc(strlen(command) + 1);
    if (!cmd_copy)
    {
        perror("malloc failed");
        free(argv);
        exit(EXIT_FAILURE);
    }
    strcpy(cmd_copy, command);

    // Split the command using strtok
    char *token = strtok(cmd_copy, " ");
    while (token != NULL && argc < 2)
    {
        argv[argc] = malloc(strlen(token) + 1); // Allocate memory for the token
        if (!argv[argc])
        {
            perror("malloc failed");
            for (int i = 0; i < argc; i++)
            {
                free(argv[i]);
            }
            free(argv);
            free(cmd_copy);
            exit(EXIT_FAILURE);
        }
        strcpy(argv[argc], token); // Copy the token to the allocated memory
        token = strtok(NULL, " ");
        argc++;
    }

    argv[argc] = NULL; // Null-terminate the array

    free(cmd_copy);
    return argv;
}


int main(int argc, char *argv[])
{

    char *path = "../Question1/ttt";
    char *exec_cmd = NULL;
    char *input = NULL;
    char *output = NULL;
    char *both = NULL;
    int in_fd = -1, out_fd = -1;

    int opt;
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1)
    {
        switch (opt)
        {
        case 'e':
            exec_cmd = optarg;
            printf("optarg: %s\n", optarg);
            printf("exec_cmd: %s\n", exec_cmd);
            break;
        case 'i':
            input = optarg;
            printf("input: %s\n", input);
            break;
        case 'o':
            output = optarg;
            printf("output: %s\n", output);
            break;
        case 'b':
            both = optarg;
            printf("both: %s\n", both);
            break;
        default:
            fprintf(stderr, "Usage: %s -e exec_cmd [-i input] [-o output] [-b both]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("Starting netcat with command: %s\n", exec_cmd ? exec_cmd : "NULL");
    printf("Input: %s\n", input ? input : "NULL");
    printf("Output: %s\n", output ? output : "NULL");
    printf("Both: %s\n", both ? both : "NULL");

    printf("im here\n");
    char **args = split_command(exec_cmd);
    printf("Executing command: %s\n", exec_cmd);
    printf("im here\n");
    if (exec_cmd == NULL)
    {
        fprintf(stderr, "Usage: %s -e exec_cmd [-i input] [-o output] [-b both]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (input)
        start_tcp_server(atoi(input), &in_fd, args);
    if (output)
        start_tcp_client("localhost", atoi(output), &out_fd);
    if (both)
        start_tcp_server(atoi(both), &in_fd, args);
        start_tcp_client("", atoi(both), &out_fd);

    if (in_fd != -1) // if server socket needs to dup2
    {
        printf("Redirecting stdin to fd %d\n", in_fd);
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }

    if (out_fd != -1) // if client socket needs to dup2
    {
        printf("Redirecting stdout to fd %d\n", out_fd);
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }
    printf("%s", exec_cmd);
    if (exec_cmd)
    {
        printf("im here\n");
        char **args = split_command(exec_cmd);
        printf("Executing command: %s\n", exec_cmd);
        printf("im here\n");
        if (exec_cmd == NULL)
        {
            fprintf(stderr, "Usage: %s -e exec_cmd [-i input] [-o output] [-b both]\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            execvp(path, args);
            perror("exec failed");
        }

        else
        {
            char buffer[BUFFER_SIZE];
            while (1)
            {
                ssize_t bytes_read = read(out_fd, buffer, BUFFER_SIZE - 1);
                if (bytes_read <= 0)
                {
                    break; // Exit loop on read error or client disconnect
                }
                buffer[bytes_read] = '\0';
            }
        }
        waitpid(pid, NULL, 0);
        for (int i = 0; args[i] != NULL; i++)
        {
            free(args[i]);
        }
        free(args);
        return 0;
    }
}
