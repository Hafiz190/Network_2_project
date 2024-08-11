#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_USERNAME_SIZE 50
#define MAX_PASSWORD_SIZE 50
#define MAX_FILE_PATH_SIZE 150
#define MAX_CREDENTIALS 100

struct user {
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
};

struct user credentials[MAX_CREDENTIALS];
int num_users = 0;
char base_directory[MAX_FILE_PATH_SIZE];
int server_port;

void load_credentials(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open password file");
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%49[^:]:%49s", credentials[num_users].username, credentials[num_users].password) == 2) {
        num_users++;
        if (num_users >= MAX_CREDENTIALS) break;
    }
    fclose(file);
}

int authenticate(const char *username, const char *password) {
    for (int i = 0; i < num_users; ++i) {
        if (strcmp(credentials[i].username, username) == 0 && strcmp(credentials[i].password, password) == 0) {
            return 1; // Authentication successful
        }
    }
    return 0; // Authentication failed
}
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[MAX_BUFFER_SIZE];
    int bytes_read;

    // Send a welcome message to the client
    send(client_socket, "Welcome to Bob's file server.\n", 29, 0);

    char username[MAX_USERNAME_SIZE] = "";
    int authenticated = 0;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_socket, buffer, MAX_BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("Client disconnected or error occurred\n");
            break;
        }

        printf("Received command: %s\n", buffer);
        char command[50];
        sscanf(buffer, "%s", command);

        if (strcmp(command, "USER") == 0 && !authenticated) {
    char user[MAX_USERNAME_SIZE], pass[MAX_PASSWORD_SIZE];
    sscanf(buffer, "%*s %s %s", user, pass);
    if (authenticate(user, pass)) {
        authenticated = 1;
        strcpy(username, user);
        send(client_socket, "200 User granted access.\n", 25, 0);
    } else {
        send(client_socket, "400 User not found.\n", 20, 0);
    }
}else if (authenticated && strcmp(command, "LIST") == 0) {
    FILE *pipe;
    char cmd[MAX_BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "ls -l %s", base_directory);
    pipe = popen(cmd, "r");
    if (pipe == NULL) {
        send(client_socket, "400 Command failed.\n", 20, 0);
    } else {
        char line[256];
        while (fgets(line, sizeof(line), pipe)) {
            send(client_socket, line, strlen(line), 0);
        }
        pclose(pipe);
        send(client_socket, ".\n", 2, 0);
    }
}else if (authenticated && strcmp(command, "GET") == 0) {
    char filename[MAX_USERNAME_SIZE];
    sscanf(buffer, "%*s %s", filename);
    char filepath[MAX_FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/%s", base_directory, filename);
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        send(client_socket, "404 File not found.\n", 20, 0);
    } else {
        char file_buffer[MAX_BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
            send(client_socket, file_buffer, bytes_read, 0);
        }
        fclose(file);
        send(client_socket, "\r\n.\r\n", 5, 0);
    }
}
    }
    close(client_socket);
    return NULL;
}



void load_credentials(const char *filename);
int authenticate(const char *username, const char *password);

int main(int argc, char *argv[]) {
    int opt;
    char password_file[MAX_FILE_PATH_SIZE] = "";

    int server_socket, client_socket;
struct sockaddr_in server_address, client_address;
socklen_t client_address_len = sizeof(client_address);

server_socket = socket(AF_INET, SOCK_STREAM, 0);
if (server_socket < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
}

server_address.sin_family = AF_INET;
server_address.sin_addr.s_addr = INADDR_ANY;
server_address.sin_port = htons(server_port);

if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
    perror("Bind failed");
    close(server_socket);
    exit(EXIT_FAILURE);
}

if (listen(server_socket, 5) < 0) {
    perror("Listen failed");
    close(server_socket);
    exit(EXIT_FAILURE);
}

printf("Server listening on port %d...\n", server_port);

    while ((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch (opt) {
            case 'd':
                strcpy(base_directory, optarg);
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'u':
                strcpy(password_file, optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -d directory -p port -u password_file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (base_directory[0] == '\0' || server_port == 0 || password_file[0] == '\0') {
        fprintf(stderr, "All arguments are required.\n");
        exit(EXIT_FAILURE);
    }

    load_credentials(password_file);

}
