#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <signal.h>

// Function prototypes
int start_tcp_server(int port);
int start_tcp_client(const char *hostname, int port);
int start_udp_server(int port);
int start_udp_client(const char *hostname, int port);
void relay_data(int input_fd, int output_fd);
void timeout_handler(int sig);

// Global timeout variable
int timeout = 0;
pid_t child_pid = 0;

// Function to start a UDP server and return the socket descriptor
int start_udp_server(int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Function to start a UDP client and return the socket descriptor
int start_udp_client(const char *hostname, int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *he;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if ((he = gethostbyname(hostname)) == NULL)
    {
        perror("Invalid hostname");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    return sockfd;
}

int start_tcp_server(int port)
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

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
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

    if (listen(server_fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return new_socket;
}

// Function to start a TCP client and return the socket descriptor
int start_tcp_client(const char *hostname, int port)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *he;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    if ((he = gethostbyname(hostname)) == NULL)
    {
        perror("Invalid hostname");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        return -1;
    }

    return sock;
}

// Function to relay data between input and output file descriptors
void relay_data(int input_fd, int output_fd)
{
    char buffer[256];
    fd_set read_fds;
    int max_sd = input_fd > output_fd ? input_fd : output_fd;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(input_fd, &read_fds);
        FD_SET(output_fd, &read_fds);

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("select error");
            break;
        }

        if (FD_ISSET(input_fd, &read_fds))
        {
            int bytes_read = read(input_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0)
            {
                break;
            }
            buffer[bytes_read] = '\0';
            write(output_fd, buffer, bytes_read);
        }

        if (FD_ISSET(output_fd, &read_fds))
        {
            int bytes_read = read(output_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0)
            {
                break;
            }
            buffer[bytes_read] = '\0';
            write(input_fd, buffer, bytes_read);
        }
    }
}

// Signal handler for timeout
void timeout_handler(int sig)
{
    fprintf(stderr, "Timeout reached, terminating process\n");
    if (child_pid > 0)
    {
        kill(child_pid, SIGKILL);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int fd = -1;
    int port;
    char *input_param = NULL;
    char *output_param = NULL;
    char *exec_param = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-i") == 0)
        {
            input_param = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            output_param = argv[++i];
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            exec_param = argv[++i];
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            timeout = atoi(argv[++i]);
        }
    }

    if (timeout > 0)
    {
        signal(SIGALRM, timeout_handler);
        alarm(timeout);
    }

    if (input_param)
    {
        if (strncmp(input_param, "TCPS", 4) == 0)
        {
            port = atoi(&input_param[4]);
            fd = start_tcp_server(port);
        }
        else if (strncmp(input_param, "UDPS", 4) == 0)
        {
            port = atoi(&input_param[4]);
            fd = start_udp_server(port);
        }
    }

    if (output_param)
    {
        if (strncmp(output_param, "TCPC", 4) == 0)
        {
            char *args = strdup(&output_param[4]);
            char *hostname = strtok(args, ",");
            char *port_str = strtok(NULL, ",");
            if (!hostname || !port_str)
            {
                fprintf(stderr, "Invalid argument. Usage: %s <TCPS<PORT>|TCPC<IP>,<PORT>>\n", argv[0]);
                free(args);
                exit(EXIT_FAILURE);
            }
            port = atoi(port_str);
            fd = start_tcp_client(hostname, port);
            free(args);
        }
        else if (strncmp(output_param, "UDPC", 4) == 0)
        {
            printf("im here\n");
            char *args = strdup(&output_param[4]);
            printf("im here\n");
            port = atoi(&argv[4][13]);
            printf("im here\n");
            if (!port)
            {
                fprintf(stderr, "Invalid argument. Usage: %s <UDPS<PORT>|UDPC<IP>,<PORT>>\n", argv[0]);
                free(args);
                exit(EXIT_FAILURE);
            }
            printf("Starting UDP client on %s:%d\n", "127.0.0.1", port);
            fd = start_udp_client("localhost", port);
            printf("im here\n");
            free(args);
        }
    }

    if (exec_param && input_param)
    {
        child_pid = fork();
        if (child_pid == 0)
        {
            // Child process to execute the command
            char *args1[] = {"ttt", "123456789", NULL};
            char *path = "../Question1/ttt";
            execvp(path, args1);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        }
        else if (child_pid > 0)
        {
            // Parent process to relay data
            if (input_param && strncmp(input_param, "UDPS", 4) == 0)
            {
                relay_data(fd, STDOUT_FILENO);
            }
            else if (output_param && strncmp(output_param, "UDPC", 4) == 0)
            {
                relay_data(STDIN_FILENO, fd);
            }
        }
        else
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s -e <command> -i <input> -o <output> [-t timeout]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (fd != -1)
    {
        close(fd);
    }

    return 0;
}
