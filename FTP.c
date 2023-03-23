#include "Headers/csapp.h"

#define PORT 4242
#define NB_PROC 5
#define MAX_NAME_LEN 256

pid_t ptab[NB_PROC];

void HandlerChild(int sig) {
    while (Waitpid(-1, NULL, WNOHANG) > 0);
}

void HandlerKill(int sig) {
    Signal(SIGCHLD, SIG_DFL);
    for (int i = 0; i < NB_PROC; i++) {
        Kill(ptab[i], SIGKILL);
        Waitpid(ptab[i], NULL, 0);
    }
    exit(EXIT_SUCCESS);
}

int creerNfils(int nbFils) {
    for (int i = 0; i < nbFils; i++)
        if ((ptab[i] = Fork()) == 0)
            return 0;
    return 1;
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

    Signal(SIGCHLD, HandlerChild);
    Signal(SIGINT, HandlerKill);

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

            Close(connfd);
            exit(EXIT_SUCCESS);
        }
    }

    // Parent
    while (1) {
        Pause();
    }
}