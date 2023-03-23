#include "csapp.h"

#define PORT 4242

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    host = argv[1];

    clientfd = Open_clientfd(host, PORT);

    printf("Connected\n");

    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        if (Rio_readlineb(&rio, buf, MAXLINE) > 0)
            Fputs(buf, stdout);
        else
            break;
    }
    Close(clientfd);
    exit(EXIT_SUCCESS);
}
