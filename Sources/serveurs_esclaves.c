#include "../Headers/serveurs_esclaves.h"
#include <stdio.h>
#include <stdlib.h>

int server_body(int connfd) {
    Requete req;
    char *arg;
    Reponse rep;
    int f;
    char filename[MAXLINE];

    init_Reponse(&rep);

    if (getcwd(filename, sizeof(filename)) == NULL) {
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");
        exit(EXIT_FAILURE);
    }
    strcat(filename, "/.server/");

    if (rio_readn(connfd, &req, sizeof(Requete)) == 0) {
        printf("Un client s'est déconnecté\n");
        Close(connfd);
        return SERVER_BODY_BYE;
    }
    Requete_ntoh(&req);

    // Lecture du nom du fichier
    if (req.arg_len) {
        arg = (char *) Malloc(req.arg_len);
        if (rio_readn(connfd, arg, req.arg_len) == 0) {
            printf("Un client s'est déconnecté\n");
            Close(connfd);
            return SERVER_BODY_BYE;
        }
    }

    switch (req.code) {
        case OP_GET:
#ifdef DEBUG
            fprintf(stderr, "Requete: GET %s\n", arg);
#endif
            strcat(filename, arg);

            if ((f = open(filename, O_RDONLY)) == -1) {
                rep.code = REP_ERREUR_FICHIER;
                Reponse_hton(&rep);
                rio_writen(connfd, &rep, sizeof(Reponse));
                return SERVER_BODY_ERR;
            }
            filename[strlen(filename) - strlen(arg)] = '\0';

            struct stat st;
            fstat(f, &st);

            // Envoie du fichier en plusieurs paquets
            unsigned int taille = st.st_size;

            if (req.cursor > st.st_size) {
                fprintf(stderr, "Erreur: curseur > taille du fichier\n");
                rep.code = REP_ERREUR_CURSEUR;
                Reponse_hton(&rep);
                rio_writen(connfd, &rep, sizeof(Reponse));
                return 1;
            } else if (req.cursor > 0) {
                fprintf(stderr, "Curseur = %d\n", req.cursor);
                Lseek(f, req.cursor, SEEK_SET);
                taille -= req.cursor;
            }
            rep.res_len = taille;

            envoie_fichier(connfd, &rep, f, taille);

            Close(f);
            break;

        case OP_BYE:
#ifdef DEBUG
            fprintf(stderr, "Requete: BYE\n");
#endif
            rep.code = REP_OK;
            Reponse_hton(&rep);
            rio_writen(connfd, &rep, sizeof(Reponse));
            Close(connfd);
            return SERVER_BODY_BYE;

        case OP_INT:
#ifdef DEBUG
            fprintf(stderr, "Requete: INT\n");
#endif
            Kill(getppid(), SIGINT);
            return SERVER_BODY_BYE;
    }
    return SERVER_BODY_OK;
}