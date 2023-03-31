#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include <string.h>
#include <time.h>

#define PORT 4242

char pathname[MAXLINE];


void envoyer_requete(int clientfd, Requete *req, char *arg) {
    Requete_hton(req);
    Rio_writen(clientfd, req, sizeof(Requete));
    Requete_ntoh(req);
    if (arg != NULL && req->arg_len > 0)
        Rio_writen(clientfd, arg, req->arg_len);
}

#define INTERPRETE_REPONSE_OK 0
#define INTERPRETE_REPONSE_PAS_OK (-1)
int interprete_reponse(int clientfd, Reponse *rep) {
    Rio_readn(clientfd, rep, sizeof(Reponse));
    Reponse_ntoh(rep);
    switch (rep->code) {
        case REP_OK:
            return INTERPRETE_REPONSE_OK;
        case REP_ERREUR_FICHIER:
            printf("Serveur: le fichier n'existe pas\n");
            break;
        case REP_ERREUR_MEMOIRE:
            printf("Serveur: erreur de mémoire, faut augmenter la RAM mon biquet\n");
            break;
        case REP_ERREUR:
            printf("Serveur: erreur\n");
            break;
    }
    return INTERPRETE_REPONSE_PAS_OK;
}

void execute_requete(int clientfd, Requete *req, Reponse *rep, char *filename, time_t req_timestamp) {
    switch (req->code) {
        case OP_GET:
            if (rep->res_len) {
                char *res = (char *) malloc(rep->res_len);
                Rio_readn(clientfd, res, rep->res_len);
                printf("%u bytes transferred in %lu sec\n", rep->res_len, time(NULL) - req_timestamp);
                strcat(pathname, filename);
                printf("File saved as %s\n", pathname);
                int f = open(pathname, O_CREAT | O_WRONLY | O_TRUNC, 700);
                write(f, res, rep->res_len);
                free(res);
                close(f);
                pathname[strlen(pathname) - strlen(filename)] = '\0';
            }
            break;
        case OP_BYE:
            break;
    }
}

size_t prompt(char *buf) {
    printf("ftp> ");
    Fgets(buf, MAXLINE, stdin);
    size_t len = strlen(buf) - 1;
    if (len) buf[len] = '\0';  // Enlever le '\n'
    return len;
}

#define LIRE_COMMANDE_ERR (-1)
int lire_commande(char *buf, Requete *req) {
    size_t len = prompt(buf);

    init_Requete(req);
    int argi = (int) len;

    if (strncmp(buf, "get", 3) == 0) {
        if (len < 5 || buf[3] != ' ') {
            fprintf(stderr, "Il manque un argument\n");
            return LIRE_COMMANDE_ERR;
        }
        req->code = OP_GET;
        req->arg_len = len - 3;
        argi = 4;
    } else if (strcmp(buf, "bye") != 0) {
        fprintf(stderr, "Commande non implémentée\n");
        return LIRE_COMMANDE_ERR;
    } else {
        req->code = OP_BYE;
    }
    return argi;
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    Requete req;
    Reponse rep;
    char *arg;
    int argi;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    host = argv[1];

    if (getcwd(pathname, sizeof(pathname)) == NULL) {                           //  -|
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");  //   |
        exit(EXIT_FAILURE);                                                     //   |> Get the working directory of
    }                                                                           //   |  the client program
    strcat(pathname, "/.client/");                                              //  -|

    clientfd = Open_clientfd(host, PORT);

    // à placer dans une boucle par la suite
        if ((argi = lire_commande(buf, &req)) == LIRE_COMMANDE_ERR) {
            Close(clientfd);
            exit(EXIT_FAILURE);
        }
        arg = buf + argi;

        time_t start = time(NULL);
        envoyer_requete(clientfd, &req, arg);

        if (interprete_reponse(clientfd, &rep) == INTERPRETE_REPONSE_PAS_OK) {
            Close(clientfd);
            exit(EXIT_SUCCESS);
        }
        execute_requete(clientfd, &req, &rep, arg, start);

    Close(clientfd);
    exit(EXIT_SUCCESS);
}
