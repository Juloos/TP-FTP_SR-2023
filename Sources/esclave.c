#include "../Headers/serveurs_esclaves.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Variable globale pour savoir si le client a fermé la connexion
 * @param sigpipe 0 si le client n'a pas fermé la connexion, 1 sinon
 */
int volatile sigpipe = 0;

/**
 * @brief Tableau de processus fils
 * @param ptab Tableau de processus fils
 */
pid_t ptab[NB_PROC];

/**
 * @brief Handler pour le signal SIGCHLD
 * @param sig Signal SIGCHLD
 */
void handler_SIGCHLD(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * @brief Handler pour le signal SIGINT
 * @param sig Signal SIGINT
 */
void handler_SIGINT(int sig) {
    Signal(SIGCHLD, SIG_DFL);
    for (int i = 0; i < NB_PROC; i++)
        kill(ptab[i], SIGKILL);
    for (int i = 0; i < NB_PROC; i++)
        wait(NULL);
    exit(EXIT_SUCCESS);
}

/**
 * @brief Handler pour le signal SIGPIPE
 * @param sig Signal SIGPIPE
 */
void handler_SIGPIPE(int sig) {
    printf("Le client a fermé la connexion\n");
    sigpipe = 1;
}

/**
 * @brief Créer un nombre de fils
 * @param nbFils Nombre de fils à créer
 * @return CREERNFILS_CHILD si le processus est un fils, CREERNFILS_PARENT sinon
 */
int creerNfils(int nbFils) {
    for (int i = 0; i < nbFils; i++)
        if ((ptab[i] = Fork()) == 0)
            return CREERNFILS_CHILD;
    return CREERNFILS_PARENT;
}

int main(int argc, char **argv) {
    Signal(SIGPIPE, handler_SIGPIPE);
    int clientfd;
    char *master_host;
    uint16_t port;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /* Récupération de l'@ IP du serveur */
    master_host = argv[1];

    /* Se connecte au serveur maître */
    clientfd = Open_clientfd(master_host, PORT);

    /* Lui envoie son IP et son port pour dire qu'il existe */
    port = atoi(argv[2]);

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

    Signal(SIGCHLD, handler_SIGCHLD);
    Signal(SIGINT, handler_SIGINT);

    /* Création de 5 fils */
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