#include "../Headers/csapp.h"
#include "../Headers/serveurs_esclaves.h"

int volatile sigpipe = 0;

/* TODO : a modifier pour envoyer ce signal à tous les serveurs esclaves */
void handler_SIGINT(int sig) {
    printf("Serveur: fermeture du serveur maître\n");
    exit(EXIT_SUCCESS);
    /* Tuer les serveurs esclaves */
}

void handler_SIGPIPE(int sig) {
    printf("Le client a fermé la connexion\n");
    sigpipe = 1;
}

int main(int argc, char **argv) {
    Signal(SIGPIPE, handler_SIGPIPE);
    int listenfd, connfd;
    int nb_servers = 0;
    int server_courant = 0;

    Serveur serveurs[MAX_SERVERS];

#ifdef DEBUG
    int n = 1;
    fprintf(stderr, "Endianess: %s\n", *(char *) &n == 1 ? "Little Endian" : "Big Endian");
#endif

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    Signal(SIGINT, handler_SIGINT);

    listenfd = Open_listenfd(PORT);

    /* Attend la connexion des serveurs esclaves */
    while (nb_servers < MAX_SERVERS) {
        while ((connfd = Accept(listenfd, NULL, NULL)) == -1);
        Serveur server;
        if (rio_readn(connfd, &server, sizeof(Serveur)) == 0) {
            fprintf(stderr, "Serveur : Un serveur esclave s'est déconnecté avant identification\n");
            continue;
        }
        serveurs[nb_servers].port = server.port;
        strcpy(serveurs[nb_servers].ip, server.ip);
#ifdef DEBUG
        fprintf(stderr, "Serveur : port de l'esclave %d = %d\n", nb_servers, serveurs[nb_servers].port);
        fprintf(stderr, "Serveur : serveur esclave %d connecté\n", numero);
#endif
        Close(connfd);
        nb_servers++;
    }

    /* Attend la connexion d'un client */
    while (1) {
        sigpipe = 0;
        while ((connfd = Accept(listenfd, NULL, NULL)) == -1);

        /* Envoie la structure serveur correspondant pour que le client se connecte à l'esclave */
        rio_writen(connfd, &serveurs[server_courant], sizeof(Serveur));

        /* Ferme la connexion */
        Close(connfd);

        server_courant = (server_courant + 1) % MAX_SERVERS;
    }
}