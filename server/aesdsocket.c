#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#define PORT 9000
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define MAX_BUFFER_SIZE 1024

int server_fd;

void handle_signal(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");

        // close server socket
        close(server_fd);

        // delete the file
        unlink(FILE_PATH);

        // close syslog
        closelog();

        exit(EXIT_SUCCESS);
    }
}

void send_file(int client_socket) {
    char *buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_read;
    int fd = open(FILE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("File open failed");
        return;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    int new_socket;
    struct sockaddr_in addr;
    int addrlen;
    int daemon_mode = 0;

    // handle option
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // open syslog
    openlog("aesdsocket", LOG_PID, LOG_USER);

    // setup signal handling
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = handle_signal;
    if (sigaction(SIGINT, &sigact, NULL) == -1 || sigaction(SIGTERM, &sigact, NULL) == -1) {
        perror("Signal handler setup failed");
        return -1;
    }

    // create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket failed");
        return -1;
    }

    // setup socket option
    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(optval)) == -1) {
        perror("Socket set options failed");
        return -1;
    }

    // bind
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Bind failed");
        return -1;
    }

    // listen
    if (listen(server_fd, 3) == -1) {
        perror("Listen failed");
        return -1;
    }

    // daemonize
    if (daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            return -1;
        }

        // if success, exit parent process
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        // change file mode mask
        umask(0);

        if (setsid() < 0) {
            perror("Setsid failed");
            return -1;
        }

        if (chdir("/") < 0) {
            perror("Chdir failed");
            return -1;
        }

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    // accept forever
    while (1) {
        // addr will store the address of client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen)) == -1) {
            perror("Accept failed");
            continue;
        }

        // get client ip
        char *client_ip = inet_ntoa(addr.sin_addr);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // receive data and append to file
        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytes_read;
        int fd = open(FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            perror("Open file failed");
            close(new_socket);
            continue;
        }

        while ((bytes_read = recv(new_socket, buffer, sizeof(buffer), 0)) > 0) {
            if (write(fd, buffer, bytes_read) == -1) {
                perror("Write to file failed");
                break;
            }

            char *newline_ptr = strchr(buffer, '\n');
            if (newline_ptr != NULL) {
                break;
            }
        }

        if (bytes_read < 0) {
            perror("Receive failed");
        }

        // close file
        close(fd);

        // send file
        send_file(new_socket);

        // log closing message
        syslog(LOG_INFO, "Closed connection from %s", client_ip);

        // close socket
        close(new_socket);
    }

    return 0;
}