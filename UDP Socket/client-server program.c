PROGRAM : Server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    char buffer[256], sendbuf[20000];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    FILE *fp;

    if (argc < 2)
    {
        printf("Usage: ./server <port>\n");
        exit(1);
    }

    portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    printf("UDP Server listening...\n");

    clilen = sizeof(cliaddr);
    while (1)
    {
        bzero(buffer, sizeof(buffer));

        // Receive filename
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&cliaddr, &clilen);
        if (n < 0)
        {
            perror("Error receiving data");
            continue;
        }

        printf("Client requested file: %s\n", buffer);

        // Try opening file
        fp = fopen(buffer, "r");
        if (fp == NULL)
        {
            strcpy(sendbuf, "Error: File not found.");
        }
        else
        {
            bzero(sendbuf, sizeof(sendbuf));
            fread(sendbuf, 1, sizeof(sendbuf) - 1, fp);
            fclose(fp);
        }

        // Send content back
        n = sendto(sockfd, sendbuf, strlen(sendbuf), 0,
                   (struct sockaddr *)&cliaddr, clilen);
        if (n < 0)
            perror("Error sending response");

        printf("Response sent.\n");
    }

    close(sockfd);
    return 0;
}



PROGRAM : Client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    char filename[256], buffer[20000];
    struct sockaddr_in servaddr;
    struct hostent *server;
    socklen_t servlen;

    if (argc < 3)
    {
        printf("Usage: ./client <server-ip> <port>\n");
        exit(1);
    }

    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        printf("Error: no such host.\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    bcopy((char *)server->h_addr,
          (char *)&servaddr.sin_addr.s_addr,
          server->h_length);

    servlen = sizeof(servaddr);

    printf("Enter filename to request: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0'; // remove newline

    // Send filename
    n = sendto(sockfd, filename, strlen(filename), 0,
               (struct sockaddr *)&servaddr, servlen);
    if (n < 0)
    {
        perror("Error sending filename");
        exit(1);
    }

    // Receive file contents
    bzero(buffer, sizeof(buffer));
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&servaddr, &servlen);
    if (n < 0)
    {
        perror("Error receiving data");
        exit(1);
    }

    printf("\n--- FILE CONTENT ---\n");
    printf("%s\n", buffer);

    close(sockfd);

    return 0;
}
