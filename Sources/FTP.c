#include <sys/stat.h>
#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"

#define PORT 4242
#define NB_PROC 5

int volatile sigpipe = 0;

pid_t ptab[NB_PROC];

void handler_SIGCHLD(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler_SIGINT(int sig) {
    Signal(SIGCHLD, SIG_DFL);
    for (int i = 0; i < NB_PROC; i++)
        kill(ptab[i], SIGKILL);
    for (int i = 0; i < NB_PROC; i++)
        wait(NULL);
    exit(EXIT_SUCCESS);
}

void handler_SIGPIPE(int sig) {
    printf("Le client a fermé la connexion\n");
    sigpipe = 1;
}

#define CREERNFILS_PARENT 1
#define CREERNFILS_CHILD 0
int creerNfils(int nbFils) {
    for (int i = 0; i < nbFils; i++)
        if ((ptab[i] = Fork()) == 0)
            return CREERNFILS_CHILD;
    return CREERNFILS_PARENT;
}

#define SERVER_BODY_BYE 2
#define SERVER_BODY_ERR 1
#define SERVER_BODY_OK 0
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

            envoie_fichier(rep, connfd, f, taille);

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
    }
    return SERVER_BODY_OK;
}

int main(int argc, char **argv) {
    int listenfd, connfd;

#ifdef DEBUG
    int n = 1;
    fprintf(stderr, "Endianess: %s\n", *(char *) &n == 1 ? "Little Endian" : "Big Endian");
#endif

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    Signal(SIGCHLD, handler_SIGCHLD);
    Signal(SIGINT, handler_SIGINT);

    listenfd = Open_listenfd(PORT);

    if (creerNfils(NB_PROC) == CREERNFILS_CHILD) {
        // Child
        Signal(SIGCHLD, SIG_DFL);
        Signal(SIGINT, SIG_DFL);
        Signal(SIGPIPE, handler_SIGPIPE);
        while (1) {
            sigpipe = 0;
            while ((connfd = Accept(listenfd, NULL, NULL)) == -1);

            while (1) {
                /* Si le client ferme la connexion par un bye ou une erreur */
                if (server_body(connfd) == SERVER_BODY_BYE || sigpipe) break;
            }
        }
    }
    // Parent
    while (1) {
        Pause();
    }
}