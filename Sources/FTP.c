//
// Created by tom on 23/03/23.
//
/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NB_PROC 5

pid_t tab[NB_PROC];

void echo(int connfd);

void HandlerChild(int sig) {
    int status;
    pid_t kpid;
    while ((kpid = Waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        printf("Processus %d terminated\n", kpid);
    }
}

void HandlerKill(int sig) {
    Signal(SIGCHLD, SIG_DFL);
    for (int i = 0; i < NB_PROC; i++) {
        if (tab[i] > 0) {
            fprintf(stderr, "Proccessus %d killed\n", tab[i]);
            Kill(tab[i], SIGINT);
            if (Waitpid(tab[i], NULL, 0) > 0) {
                printf("Processus %d terminated v2\n", tab[i]);
            }
        }
    }
    exit(0);
}

/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv) {

    Signal(SIGCHLD, HandlerChild);
    Signal(SIGINT, HandlerKill);

    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    clientlen = (socklen_t) sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    printf("N° de port : %d\n", port);

    //pid_t *tab = (pid_t *) malloc(sizeof(pid_t) * NB_PROC);

    int nbFils = 0;
    int nbEssai = 10;
    int essai;
    int i = 0;
    pid_t pid;

    do {
        essai = 0;
        do {
            pid = Fork();
            essai++;
        } while (pid == -1 && essai < nbEssai);
        nbFils++;
        if (pid > 0) { /* father */
            tab[i] = pid;
            i++;
        } else { /* child */
            Signal(SIGCHLD, SIG_DFL);
            Signal(SIGINT, SIG_DFL);
            while (1) {
                while ((connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen)) == -1);

                /* determine the name of the client */
                Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

                printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

                echo(connfd);
                Close(connfd);
            }
        }
    } while (pid != 0 && nbFils < NB_PROC);

    while (1) {
        pause();
    }
}