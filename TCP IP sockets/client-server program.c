PROGRAM : Client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>   // for read(), write(), close()

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    char buffer[256], c[20000];
    struct sockaddr_in serv;
    struct hostent *server;

    if (argc < 3)
    {
        printf("Error: insufficient arguments.\nUsage:\n./client <hostname> <port>\nExample: ./client 127.0.0.1 7777\n");
        exit(1);
    }

    portno = atoi(argv[2]);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Get host
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error: no such host\n");
        exit(1);
    }

    bzero((char *)&serv, sizeof(serv));
    serv.sin_family = AF_INET;

    // Copy IP address
    bcopy((char *)server->h_addr,
          (char *)&serv.sin_addr.s_addr,
          server->h_length);

    serv.sin_port = htons(portno);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0)
    {
        perror("Error connecting");
        exit(1);
    }

    printf("Enter the file path to request from server:\n");

    // Use fgets instead of scanf to avoid overflow
    bzero(buffer, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';  // remove newline

    // Send file name to server
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Error writing to socket");
        exit(1);
    }

    // Receive content
    bzero(c, sizeof(c));
    n = read(sockfd, c, sizeof(c) - 1);
    if (n < 0)
    {
        perror("Error reading from socket");
        exit(1);
    }

    printf("\nClient: Displaying content of %s\n\n", buffer);
    printf("%s\n", c);

    close(sockfd);
    return 0;
}


PROGRAM : Server.c

  #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, len, n;
    char buffer[256], c[2000], cc[20000];
    struct sockaddr_in serv, cli;
    FILE *fd;

    if (argc < 2)
    {
        printf("Error: no port number provided.\nUsage: ./server <port>\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    portno = atoi(argv[1]);

    bzero((char *)&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0)
    {
        perror("Error on binding");
        exit(1);
    }

    listen(sockfd, 10);
    len = sizeof(cli);

    printf("Server: Waiting for connection...\n");

    newsockfd = accept(sockfd, (struct sockaddr *)&cli, (socklen_t *)&len);
    if (newsockfd < 0)
    {
        perror("Error on accept");
        exit(1);
    }

    bzero(buffer, sizeof(buffer));
    n = read(newsockfd, buffer, sizeof(buffer) - 1);

    if (n < 0)
    {
        perror("Error reading from socket");
        exit(1);
    }

    printf("\nServer received filename: %s\n", buffer);

    fd = fopen(buffer, "r");
    if (fd != NULL)
    {
        printf("Server: %s found\nOpening and reading...\n", buffer);

        bzero(cc, sizeof(cc));
        bzero(c, sizeof(c));

        // Read file safely line by line
        while (fgets(c, sizeof(c), fd) != NULL)
        {
            strcat(cc, c);
        }

        fclose(fd);

        n = write(newsockfd, cc, strlen(cc));
        if (n < 0)
            perror("Error writing to socket");

        printf("Transfer complete.\n");
    }
    else
    {
        printf("Server: File not found.\n");

        char msg[] = "File not found";
        n = write(newsockfd, msg, strlen(msg));
        if (n < 0)
            perror("Error writing to socket");
    }

    close(newsockfd);
    close(sockfd);

    return 0;
}
