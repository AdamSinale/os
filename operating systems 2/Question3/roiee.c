#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdbool.h>

#define index 5

//mync -e “ttt 123456789” -i TCPS4050 -o TCPClocalhost,4455

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdbool.h>
#define BUFFER_SIZE 1024

int create_client(int port, char mode, const char* server_ip) {
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process: read from server
        while (1) {
            ssize_t bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break; // Exit loop on read error or server disconnect
            }
            buffer[bytes_read] = '\0';
            printf("Received from server: %s\n", buffer);
        }
        close(client_sock);
        exit(0);
    } else { // Parent process: write to server
        while (1) {
            printf("Enter message (-1 to quit): ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

            if (strcmp(buffer, "-1") == 0) {
                break; // Exit loop if user enters -1
            }

            write(client_sock, buffer, strlen(buffer));
        }
        close(client_sock);
        #include <signal.h>

        kill(pid, SIGKILL); // Kill child process reading from server
    }
    return 0;
}
int create_server(char mode, int port, const char* exe, const char* arg) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // Convert port to network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    socklen_t client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    exe = "../Question_1/ttt";
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        switch (mode) {
            case 'i':
                dup2(client_sock, 0);
                break;
            case 'o':
                dup2(client_sock, 1);
                break;
            case 'b':
                dup2(client_sock, 0);
                dup2(client_sock, 1);
                break;
        }
        execlp(exe, exe, "123456789", NULL);
        perror("Execution of exe failed");
        exit(EXIT_FAILURE);
    } else {
        char buffer[BUFFER_SIZE];
        while (1) {
            ssize_t bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break; // Exit loop on read error or client disconnect
            }
            buffer[bytes_read] = '\0';
            // printf("Received from client: %s\n", buffer);
        }
    }
    int status;
    waitpid(pid, &status, 0);
    close(client_sock);
    close(server_sock);
    return 0;
}

int main(int argc, char *argv[]){
    if (argc != 8  && argc != index+1){
        printf("%d",argc);
        perror("not good");
        exit(EXIT_FAILURE);
    }

    bool isServer = false;
    if(argc == index+1){ 
        char mode = *(argv[4]+1); //-lemur
        // printf("%c\n",mode);
        char *token; //contains the port 
        if (strchr(argv[index],'S')){//if it contains S, then it's a server
            char* delim = "S";        
            token = strtok(argv[index], delim);
            token = strtok(NULL, delim);
            isServer = true;
        }
        if (isServer){
            printf("Server\n");
            char *exe = strtok(argv[2]," ");
            printf("%s\n",exe);
            char *arg = strtok(NULL," ");
            create_server(mode ,atoi(token),exe,arg);
        }
        else{
            char* delim = "C";        
            token = strtok(argv[index], delim);
            token = strtok(NULL, delim);
            token = strtok(NULL, delim);
            int port = atoi(token);
            printf("%d\n",port);
            printf("Client\n");
            create_client(port, mode,"127.0.0.1");
        }

        return 0;
    }
    // if(argc == 8){ // Open client
    //     int client_sock;
    //     struct sockaddr_in server_addr;
    //     char mode = *(argv[3]+1); //-lemur
    //     client_sock = socket(AF_INET,SOCK_STREAM,0);
    //     if(client_sock < 0){
    //         perror("socket failed");
    //         exit(EXIT_FAILURE);
    //     }
    //     server_addr.sin_family = AF_INET;
    //     server_addr.sin_port = atoi(argv[4]);
    //     server_addr.sin_addr.s_addr = inet_addr(argv[5]);
    //     if(connect(client_sock,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
    //         perror("connect failed");
    //         exit(EXIT_FAILURE);
    //     }
    //     char *exe = strtok(argv[2]," ");
    //     char *arg = strtok(NULL," ");
    //     char *path = "../Question_1/ttt";
    //     if(!strcmp("ttt",exe)){
    //         exe = path;
    //     }
    //     pid_t pid = fork();
    //     if(pid < 0){
    //         perror("fork failed");
    //         exit(EXIT_FAILURE);
    //     }
    //     if(pid == 0){
    //         switch(mode){
    //             case'i':
    //                 {
    //                     dup2(client_sock,0);
    //                     break;
    //                 }
    //             case 'o':
    //                 {
    //                     dup2(client_sock,1);
    //                     break;
    //                 }
    //             case 'b':
    //                 {
    //                     dup2xasASDSDPF;FJNSAFJ](client_sock,0);
    //                     dup2(client_sock,1);
    //                     break;
    //                 }
    //         }
    //         // execlp(exe,exe,arg,NULL);
    //         perror("Execution of path failed");
    //         exit(EXIT_FAILURE);
    //     }
        int status;
        // waitpid(pid,&status,0);
        // close(client_sock);
        return 0;
    // }
    // return 0;
}