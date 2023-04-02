//
// Created by tom on 02/04/23.
//

#include "../Headers/serveurs_esclaves.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME_LEN 256

int volatile sigpipe0 = 0;

char *master_ip = "127.0.0.1";

void handler_SIGPIPE1(int sig) {
    printf("Le client a fermé la connexion\n");
    sigpipe0 = 1;
}

int main(int argc, char **argv) {
    Signal(SIGPIPE, handler_SIGPIPE1);
    int clientfd;
    char *master_host = master_ip;
    int port;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /* Se connecte au serveur maître */
    clientfd = Open_clientfd(master_host, PORT);

    /* Lui envoie son IP et son port pour dire qu'il existe */
    port = atoi(argv[1]);

    Serveur serv;
    serv.port = port;

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getsockname(clientfd, (struct sockaddr *) &addr, &len);
    printf("Adresse IP de l'esclave : %s\n", inet_ntoa(addr.sin_addr));

    strcpy(serv.ip, inet_ntoa(addr.sin_addr));

    /* Envoie ses données au serveur maître */
    rio_writen(clientfd, &serv, sizeof(serv));

    /* Ferme la connexion */
    Close(clientfd);

    /* Attend la connexion d'un client */
    int listenfd, connfd;

    listenfd = Open_listenfd(port);

    while (1) {
        sigpipe0 = 0;
        while ((connfd = Accept(listenfd, NULL, NULL)) == -1);

        while (1) {
            /* Si le client ferme la connexion par un bye ou une erreur */
            if (server_body(connfd) == SERVER_BODY_BYE || sigpipe0) break;
        }
    }
    return 0;
}