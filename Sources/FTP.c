#include <sys/stat.h>
#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"

#define PORT 4242
#define NB_PROC 5
#define MAX_NAME_LEN 256

int volatile sigpipe = 0;

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

void handler_SIGPIPE(int sig) {
    printf("Le client a fermé la connexion\n");
    sigpipe = 1;
}

int creerNfils(int nbFils) {
    for (int i = 0; i < nbFils; i++)
        if ((ptab[i] = Fork()) == 0)
            return 0;
    return 1;
}

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
        return 2;
    }
    Requete_ntoh(&req);

    // Lecture du nom du fichier
    if (req.arg_len) arg = (char *) Malloc(req.arg_len);
    Rio_readn(connfd, arg, req.arg_len);

    if (req.code == OP_GET) {
#ifdef DEBUG
        fprintf(stderr, "Requete: GET %s\n", arg);
#endif

        strcat(filename, arg);

        if ((f = open(filename, O_RDONLY)) == -1) {
            rep.code = REP_ERREUR_FICHIER;
            Reponse_hton(&rep);
            Rio_writen(connfd, &rep, sizeof(Reponse));
            return 1;
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
    }
    return 0;
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    socklen_t clientlen = (socklen_t) sizeof(clientaddr);

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
        Signal(SIGPIPE, handler_SIGPIPE);
        while (1) {
            sigpipe = 0;
            while ((connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen)) == -1);

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

            printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

            while (1) {
                /* Si le client ferme la connexion par un bye ou une erreur */
                if (server_body(connfd) == 2 || sigpipe) break;
            }
        }
    }
    // Parent
    while (1) {
        Pause();
    }
}