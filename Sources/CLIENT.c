#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include <string.h>

#define PORT 4242

void handler_SIGPIPE(int sig) {
    printf("Le serveur a fermé la connexion\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    size_t len;
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    host = argv[1];

    Signal(SIGPIPE, handler_SIGPIPE);
    clientfd = Open_clientfd(host, PORT);

    Rio_readinitb(&rio, clientfd);

    // à placer dans une boucle par la suite
        printf("ftp> ");
        Fgets(buf, MAXLINE, stdin);
        len = strlen(buf) - 1;
        if (len) buf[len] = '\0';  // Enlever le '\n'

        Requete req = {OP_BYE, 0};
        char *arg = NULL;

        if (strncmp(buf, "get ", 4) == 0) {
            req.code = OP_GET;
            if (len < 5) {
                fprintf(stderr, "Il manque un argument\n");
                Close(clientfd);
                exit(EXIT_FAILURE);
            }
            req.arg_len = len - 4;
            arg = buf + 4;
        } else if (strncmp(buf, "bye ", 4) != 0) {
            fprintf(stderr, "Commande non implémentée\n");
            Close(clientfd);
            exit(EXIT_FAILURE);
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
                            FILE *f = fopen(arg, "w");
                            fwrite(res, 1, rep.res_len, f);
                        }
                        break;
                    case OP_BYE:
                        break;
                }
                break;
            case ERREUR_FICHIER:
                printf("Serveur: le fichier n'existe pas\n");
                break;
            case ERREUR_MEMOIRE:
                printf("Serveur: erreur de mémoire\n");
                break;
            case ERREUR:
                printf("Serveur: erreur\n");
                break;
        }

    Close(clientfd);
    exit(EXIT_SUCCESS);
}
