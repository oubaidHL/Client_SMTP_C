#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 1024
#define DEFAULT_SMTP_PORT 25

typedef enum {
    CONNECTING,
    WAITING_HELO_RESPONSE,
    WAITING_MAIL_FROM_RESPONSE,
    WAITING_RCPT_TO_RESPONSE,
    WAITING_DATA_RESPONSE,
    WAITING_CONTENT_RESPONSE,
    WAITING_QUIT_RESPONSE,
    ERROR,
    SUCCESS
} SMTPState;

int readResponse(int sockfd, char* buffer, int bufferSize) {
    memset(buffer, 0, bufferSize);
    ssize_t bytesRead = read(sockfd, buffer, bufferSize - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
    }
    return bytesRead;
}

int sendCommand(int sockfd, const char* format, ...) {
    char buffer[MAX_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, MAX_BUFFER_SIZE, format, args);
    va_end(args);
    size_t len = strlen(buffer);

    if (write(sockfd, buffer, len) != len) {
        return -1;
    }
    return 0;
}

int handleState(int sockfd, SMTPState *state, const char* sender, const char* recipient, const char* subject, const char* message) {
    char buffer[MAX_BUFFER_SIZE];
    int responseCode;
    int bytesReceived;

    switch (*state) {
        case CONNECTING:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 220) {
                    *state = WAITING_HELO_RESPONSE;
                    sendCommand(sockfd, "HELO client.example.com\r\n");
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_HELO_RESPONSE:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 250) {
                    *state = WAITING_MAIL_FROM_RESPONSE;
                    sendCommand(sockfd, "MAIL FROM:<%s>\r\n", sender);
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_MAIL_FROM_RESPONSE:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 250) {
                    *state = WAITING_RCPT_TO_RESPONSE;
                    sendCommand(sockfd, "RCPT TO:<%s>\r\n", recipient);
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_RCPT_TO_RESPONSE:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 250) {
                    *state = WAITING_DATA_RESPONSE;
                    sendCommand(sockfd, "DATA\r\n");
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_DATA_RESPONSE:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 354) {
                    *state = WAITING_CONTENT_RESPONSE;
                    sendCommand(sockfd, "Subject: %s\r\n\r\n%s\r\n.\r\n", subject, message);
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_CONTENT_RESPONSE:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 250) {
                    *state = WAITING_QUIT_RESPONSE;
                    sendCommand(sockfd, "QUIT\r\n");
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_QUIT_RESPONSE:
            bytesReceived = readResponse(sockfd, buffer, MAX_BUFFER_SIZE);
            if (bytesReceived > 0 && sscanf(buffer, "%d", &responseCode) == 1) {
                printf("Server response: %s", buffer);
                if (responseCode == 221) {
                    *state = SUCCESS;
                } else {
                    *state = ERROR;
                }
            } else {
                *state = ERROR;
            }
            break;

        case ERROR:
            printf("An error occurred while communicating with the server.\n");
            return -1;

        case SUCCESS:
            printf("Email sent successfully!\n");
            return 0;
    }

    return 1;
}

int tcp_connect(const char *server, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    host = gethostbyname(server);
    if (host == NULL) {
        fprintf(stderr, "Unknown server: %s\n", server);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return -1;
    }

    return sockfd;
}

int sendEmail(const char* server, int port, const char* sender, const char* recipient, const char* subject, const char* message) {
    SMTPState state = CONNECTING;
    int sockfd = tcp_connect(server, port);

    if (sockfd < 0) {
        return -1;
    }

    while (state != SUCCESS) {
        if (handleState(sockfd, &state, sender, recipient, subject, message) < 0) {
            close(sockfd);
            return -1;
        }
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        printf("Usage: %s <server> <port> <sender> <recipient> <subject> <message>\n", argv[0]);
        return 1;
    }

    const char* server = argv[1];
    int port = atoi(argv[2]);
    const char* sender = argv[3];
    const char* recipient = argv[4];
    const char* subject = argv[5];
    const char* message = argv[6];

    int result = sendEmail(server, port, sender, recipient, subject, message);
    return result;
}
