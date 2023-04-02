//
// Created by tom on 02/04/23.
//

#include "../Headers/serveurs_esclaves.h"
#include <stdio.h>
#include <stdlib.h>

#define NUMERO_SERVEUR 0

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

    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /* Se connecte au serveur maître */
    clientfd = Open_clientfd(master_host, PORT);

    /* Lui envoie un message pour dire qu'il existe */
    uint8_t numero = NUMERO_SERVEUR;
    rio_writen(clientfd, &numero, sizeof(numero));

    /* Ferme la connexion */
    Close(clientfd);

    /* Attend la connexion d'un client */
    int listenfd, connfd;

    listenfd = Open_listenfd(PORT1);

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