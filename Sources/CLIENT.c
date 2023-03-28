#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include <string.h>
#include <time.h>

#define PORT 4242

void handler_SIGPIPE(int sig) {
    printf("Le serveur a fermé la connexion\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    size_t len;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    host = argv[1];

    Signal(SIGPIPE, handler_SIGPIPE);
    clientfd = Open_clientfd(host, PORT);

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

        time_t start = time(NULL);
        len = req.arg_len;  // Avoid endianess issues with self
        Requete_hton(&req);
        Rio_writen(clientfd, &req, sizeof(Requete));
        Rio_writen(clientfd, arg, len);

        Reponse rep;
        Rio_readn(clientfd, &rep, sizeof(Reponse));
        Reponse_ntoh(&rep);
        switch (rep.code) {
            case REP_OK:
                switch (req.code) {
                    case OP_GET:
                        if (rep.res_len) {
                            char *res = (char *) malloc(rep.res_len);
                            Rio_readn(clientfd, res, rep.res_len);
                            printf("%u bytes transferred in %lu sec\n", rep.res_len, time(NULL) - start);
                            int f = open(arg, O_CREAT | O_WRONLY | O_TRUNC);
                            write(f, res, rep.res_len);
                            close(f);
                        }
                        break;
                    case OP_BYE:
                        break;
                }
                break;
            case REP_ERREUR_FICHIER:
                printf("Serveur: le fichier n'existe pas\n");
                break;
            case REP_ERREUR_MEMOIRE:
                printf("Serveur: erreur de mémoire\n");
                break;
            case REP_ERREUR:
                printf("Serveur: erreur\n");
                break;
        }

    Close(clientfd);
    exit(EXIT_SUCCESS);
}
