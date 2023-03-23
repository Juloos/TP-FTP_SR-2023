#include "csapp.h"
#include "protocoles.h"
#include <string.h>

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

        Requete req = {OP_BYE, ""};
        if (strncmp(buf, "get ", 4) == 0) {
            req.code = OP_GET;
        } else if (strncmp(buf, "bye ", 4) == 0) {
            req.code = OP_BYE;
        } else {
            printf("Commande non implémentée\n");
            continue;
        }

        switch(req.code) {
            case OP_GET:
                if(strlen(buf) < 5) {
                    fprintf(stderr, "Il manque un argument\n");
                    break;
                }
                strcpy(req.arg, buf + 4);
                break;
            case OP_BYE:
                strcpy(req.arg, "");
                break;
            default:
                fprintf(stderr, "Commande non implémentée\n");
        }

        Rio_writen(clientfd, buf, strlen(buf));
        if (Rio_readlineb(&rio, buf, MAXLINE) > 0)
            Fputs(buf, stdout);
        else
            break;
    }
    Close(clientfd);
    exit(EXIT_SUCCESS);
}
