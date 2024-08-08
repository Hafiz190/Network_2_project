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

void load_credentials(const char *filename);
int authenticate(const char *username, const char *password);

int main(int argc, char *argv[]) {
    int opt;
    char password_file[MAX_FILE_PATH_SIZE] = "";

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

    // Server setup and other logic will be added in subsequent commits
}
