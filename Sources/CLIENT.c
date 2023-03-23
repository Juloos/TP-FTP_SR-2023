#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include <string.h>

#define PORT 4242

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE], *arg;
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
        Requete req = {OP_BYE, 0}; arg = NULL;

        if (strncmp(buf, "get ", 4) == 0) {
            req.code = OP_GET;
            if (strlen(buf) < 5) {
                fprintf(stderr, "Il manque un argument\n");
                continue;
            }
            req.arg_len = strlen(buf) - 4;
            arg = buf + 4;
        } else if (strncmp(buf, "bye ", 4) != 0) {
            printf("Commande non implémentée\n");
            continue;
        }

        Requete_hton(&req);
        Rio_writen(clientfd, &req, sizeof(Requete));
        Rio_writen(clientfd, arg, req.arg_len);

        Reponse rep;
        Rio_readnb(&rio, &rep, sizeof(Reponse));
        Reponse_ntoh(&rep);
        switch (rep.code) {
            case OK:
                switch (req.code) {
                    case OP_GET:
                        if (rep.res_len) {
                            char *res = (char *) Malloc(rep.res_len);
                            Rio_readnb(&rio, res, rep.res_len);
                            // write to file
                            FILE *f = fopen(buf + 4, "w");
                            fwrite(res, 1, rep.res_len, f);
                        }
                        break;
                    case OP_BYE:
                        break;
                }
                break;
            case ERREUR:
                break;
        }
    }
    Close(clientfd);
    exit(EXIT_SUCCESS);
}
