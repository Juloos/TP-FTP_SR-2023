#include <sys/stat.h>
#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"

#define PORT 4242
#define NB_PROC 5
#define MAX_NAME_LEN 256

pid_t ptab[NB_PROC];

void handler_SIGCHLD(int sig) {
    while (Waitpid(-1, NULL, WNOHANG) > 0);
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
    rio_t rio;
    Requete req;
    char *arg;
    Reponse rep = {OK, 0};
    FILE *f;
    char serveur_wd[256];

    if (getcwd(serveur_wd, sizeof(serveur_wd)) == NULL) {
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");
        exit(EXIT_FAILURE);
    }
    // Remove BIN/ from the path and replace it with SERVER/
    serveur_wd[strlen(serveur_wd) - 3] = '\0';
    strcat(serveur_wd, "SERVER/");

    Rio_readinitb(&rio, connfd);

    Rio_readnb(&rio, &req, sizeof(Requete));
    Requete_ntoh(&req);
    if (req.arg_len) arg = (char *) Malloc(req.arg_len);
    Rio_readnb(&rio, arg, req.arg_len);

    if (req.code == OP_GET) {
        printf("Requete: GET %s\n", arg);

        char *filename = (char *) Malloc(strlen(serveur_wd) + strlen(arg) + 1);
        strcpy(filename, serveur_wd);
        strcat(filename, arg);

        if ((f = fopen(filename, "r")) == NULL) {
            rep.code = ERREUR_FICHIER;
            Reponse_hton(&rep);
            Rio_writen(connfd, &rep, sizeof(Reponse));
        }

        struct stat st;
        stat(arg, &st);

        char *buf;
        if ((buf = (char *) Malloc(st.st_size)) == NULL) {
            rep.code = ERREUR_MEMOIRE;
            Reponse_hton(&rep);
            Rio_writen(connfd, &rep, sizeof(Reponse));
        }
        fread(buf, 1, st.st_size, f);

        rep.res_len = st.st_size;
        Reponse_hton(&rep);
        Rio_writen(connfd, &rep, sizeof(Reponse));
        Rio_writen(connfd, buf, st.st_size);
    }
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
        while (1) {
            while ((connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen)) == -1);

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

            printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

            server_body(connfd);

            Close(connfd);
        }
    }

    // Parent
    while (1) {
        Pause();
    }
}