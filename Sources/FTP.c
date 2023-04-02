#include <sys/stat.h>
#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"

#define PORT 4242

#ifndef NB_PROC
#define NB_PROC 5
#endif

pid_t ptab[NB_PROC];

void handler_SIGCHLD(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler_SIGINT(int sig) {
    Signal(SIGCHLD, SIG_DFL);
    for (int i = 0; i < NB_PROC; i++)
        Kill(ptab[i], SIGKILL);
    for (int i = 0; i < NB_PROC; i++)
        Wait(NULL);
    exit(EXIT_SUCCESS);
}

int creerNfils(int nbFils) {
    for (int i = 0; i < nbFils; i++)
        if ((ptab[i] = Fork()) == 0)
            return 0;
    return 1;
}

void server_body(int connfd) {
    Requete req;
    char *arg;
    Reponse rep = {REP_OK, 0};
    int f;
    char filename[MAXLINE];

    if (getcwd(filename, sizeof(filename)) == NULL) {
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");
        exit(EXIT_FAILURE);
    }
    strcat(filename, "/.server/");

    Rio_readn(connfd, &req, sizeof(Requete));
    Requete_ntoh(&req);
    if (req.arg_len) arg = (char *) Malloc(req.arg_len);
    Rio_readn(connfd, arg, req.arg_len);

    switch (req.code) {
        case (OP_GET):
            printf("Requete: GET %s\n", arg);

            strcat(filename, arg);
            if ((f = open(filename, O_RDONLY)) == -1) {
                rep.code = REP_ERREUR_FICHIER;
                Reponse_hton(&rep);
                Rio_writen(connfd, &rep, sizeof(Reponse));
                return;
            }
            filename[strlen(filename) - strlen(arg)] = '\0';

            struct stat st;
            fstat(f, &st);

            char *buf;
            if ((buf = (char *) malloc(st.st_size)) == NULL) {
                rep.code = REP_ERREUR_MEMOIRE;
                Reponse_hton(&rep);
                Rio_writen(connfd, &rep, sizeof(Reponse));
                return;
            }
            read(f, buf, st.st_size);
            close(f);

            rep.res_len = st.st_size;
            Reponse_hton(&rep);
            Rio_writen(connfd, &rep, sizeof(Reponse));
            if (st.st_size > 0)
                Rio_writen(connfd, buf, st.st_size);
            free(buf);
            break;
        case (OP_BYE):
            printf("Requete: BYE\n");
            Reponse_hton(&rep);
            Rio_writen(connfd, &rep, sizeof(Reponse));
            break;
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    Signal(SIGCHLD, handler_SIGCHLD);
    Signal(SIGINT, handler_SIGINT);

    listenfd = Open_listenfd(PORT);

    if (creerNfils(NB_PROC) == 0) {
        // Child
        Signal(SIGCHLD, SIG_DFL);
        Signal(SIGINT, SIG_DFL);
        while (1) {
            while ((connfd = Accept(listenfd, NULL, NULL)) == -1);

            server_body(connfd);

            Close(connfd);
        }
    }

    // Parent
    while (1) {
        Pause();
    }
}