#include <sys/stat.h>
#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include "../Headers/server_body.h"

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
                if (server_body(connfd) == SERVER_BODY_BYE || sigpipe)
                    break;
            }
        }
    }
    // Parent
    while (1) {
        Pause();
    }
}