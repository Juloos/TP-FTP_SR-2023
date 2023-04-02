#include <sys/stat.h>
#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include "../Headers/server_body.h"

int server_get(int connfd, Requete req, char *arg, Reponse *rep) {
#ifdef DEBUG
    fprintf(stderr, "Requete: GET %s\n", arg);
#endif
    int f;
    char filename[MAXLINE];

    if (getcwd(filename, sizeof(filename)) == NULL) {
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");
        exit(EXIT_FAILURE);
    }
    strcat(filename, "/.server/");
    strcat(filename, arg);

    if ((f = open(filename, O_RDONLY)) == -1) {
        rep->code = REP_ERREUR_FICHIER;
        Reponse_hton(rep);
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
        rep->code = REP_ERREUR_CURSEUR;
        Reponse_hton(rep);
        rio_writen(connfd, rep, sizeof(Reponse));
        return SERVER_BODY_ERR;
    } else if (req.cursor > 0) {
        fprintf(stderr, "Curseur = %d\n", req.cursor);
        Lseek(f, req.cursor, SEEK_SET);
        taille -= req.cursor;
    }
    rep->res_len = taille;

    envoie_fichier(connfd, rep, f, taille);

    Close(f);
    return SERVER_BODY_OK;
}

int server_bye(int connfd, Reponse *rep) {
#ifdef DEBUG
    fprintf(stderr, "Requete: BYE\n");
#endif
    rep->code = REP_OK;
    Reponse_hton(rep);
    rio_writen(connfd, rep, sizeof(Reponse));
    Close(connfd);
    return SERVER_BODY_BYE;
}

int server_body(int connfd) {
    Requete req;
    char *arg;
    Reponse rep;

    init_Reponse(&rep);

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
            return server_get(connfd, req, arg, &rep);
        case OP_BYE:
            return server_bye(connfd, &rep);
    }

    return SERVER_BODY_ERR;  // Should never happen
}
