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

void sendCommand(int sockfd, const char* command) {
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "%s\r\n", command);
    write(sockfd, buffer, strlen(buffer));
}

int receiveResponse(int sockfd) {
    char buffer[MAX_BUFFER_SIZE];
    int responseCode = 0;
    ssize_t bytesRead = read(sockfd, buffer, MAX_BUFFER_SIZE - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        sscanf(buffer, "%d", &responseCode);
        printf("Server response: %s", buffer);
    }
    return responseCode;
}

void handleState(int sockfd, SMTPState *state, const char* sender, const char* recipient, const char* subject, const char* message) {
    char buffer[MAX_BUFFER_SIZE];
    int responseCode;

    switch (*state) {
        case CONNECTING:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 220) {
                *state = WAITING_HELO_RESPONSE;
                sendCommand(sockfd, "HELO client.example.com");
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_HELO_RESPONSE:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 250) {
                *state = WAITING_MAIL_FROM_RESPONSE;
                sprintf(buffer, "MAIL FROM:<%s>", sender);
                sendCommand(sockfd, buffer);
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_MAIL_FROM_RESPONSE:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 250) {
                *state = WAITING_RCPT_TO_RESPONSE;
                sprintf(buffer, "RCPT TO:<%s>", recipient);
                sendCommand(sockfd, buffer);
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_RCPT_TO_RESPONSE:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 250) {
                *state = WAITING_DATA_RESPONSE;
                sendCommand(sockfd, "DATA");
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_DATA_RESPONSE:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 354) {
                *state = WAITING_CONTENT_RESPONSE;
                sprintf(buffer, "From: <%s>\r\nTo: <%s>\r\nSubject: %s\r\n\r\n%s\r\n.\r\n", sender, recipient, subject, message);
                write(sockfd, buffer, strlen(buffer));
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_CONTENT_RESPONSE:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 250) {
                *state = WAITING_QUIT_RESPONSE;
                sendCommand(sockfd, "QUIT");
            } else {
                *state = ERROR;
            }
            break;

        case WAITING_QUIT_RESPONSE:
            responseCode = receiveResponse(sockfd);
            if (responseCode == 221) {
                *state = SUCCESS;
            } else {
                *state = ERROR;
            }
            break;

        default:
            *state = ERROR;
            break;
    }
}

void sendEmail(const char* sender, const char* subject, const char* message, const char* server, const char* recipient, int port) {
    int sockfd;
    struct sockaddr_in serverAddr;
    struct hostent* host;
    SMTPState state = CONNECTING;

    // Get the host information
    host = gethostbyname(server);
    if (host == NULL) {
        perror("Invalid hostname");
        return;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    // Set up server information
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return;
    }

    while (state != SUCCESS && state != ERROR) {
        handleState(sockfd, &state, sender, recipient, subject, message);
    }

    if (state == SUCCESS) {
        printf("Email sent successfully!\n");
    } else {
        printf("An error occurred while sending the email.\n");
    }

    // Close the socket
    close(sockfd);
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        printf("Usage: %s sender subject message server recipient [port]\n", argv[0]);
        return 1;
    }

    const char* sender = argv[1];
    const char* subject = argv[2];
    const char* message = "";
    const char* server = argv[4];
    const char* recipient = argv[5];
    int port = (argc >= 7) ? atoi(argv[6]) : DEFAULT_SMTP_PORT;

    // Check if the message is a file or a direct text
    if (access(argv[3], F_OK) != -1) {
        FILE* file = fopen(argv[3], "r");
        if (file == NULL) {
            perror("Failed to open message file");
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* buffer = malloc(fileSize + 1);
        fread(buffer, 1, fileSize, file);
        fclose(file);
        buffer[fileSize] = '\0';
        message = buffer;
    } else {
        message = argv[3];
    }

    sendEmail(sender, subject, message, server, recipient, port);

    return 0;
}
