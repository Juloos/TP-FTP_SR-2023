#include "../Headers/csapp.h"
#include "../Headers/serveurs_esclaves.h"

/**
 * @brief Variable globale pour savoir si le client a fermé la connexion
 * @param sigpipe 0 si le client n'a pas fermé la connexion, 1 sinon
 */
int volatile sigpipe = 0;

/**
 * @brief Tableau de serveurs esclaves
 * @param serveurs Tableau de serveurs esclaves
 */
Serveur serveurs[MAX_SERVERS];

/**
 * @brief Handler pour le signal SIGINT
 * @param sig Signal SIGINT
 */
void handler_SIGINT(int sig) {
    Requete req;
    int clientfd;

    init_Requete(&req);
    req.code = OP_INT;
    Requete_hton(&req);

    /* Tuer les serveurs esclaves */
    for (int i = 0; i < MAX_SERVERS; i++) {
        if ((clientfd = open_clientfd(serveurs[i].ip, serveurs[i].port)) != -1) {
            rio_writen(clientfd, &req, sizeof(Requete));
            Close(clientfd);
        }
    }

    printf("Serveur: fermeture du serveur maître\n");
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

int main(int argc, char **argv) {
    Signal(SIGPIPE, handler_SIGPIPE);
    int listenfd, connfd;
    int nb_servers = 0;
    int server_courant = 0;

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

#ifdef DEBUG
    fprintf(stderr, "Serveur : Serveur maître en attente de connexion des %d esclaves\n", MAX_SERVERS);
#endif

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
        fprintf(stderr, "Serveur : serveur esclave %d connecté\n", nb_servers);
#endif
        Close(connfd);
        nb_servers++;
    }

#ifdef DEBUG
    fprintf(stderr, "Serveur : Tous les serveurs esclaves sont connectés\n");
#endif

    /* Attend la connexion d'un client */
    while (1) {
        sigpipe = 0;
        while ((connfd = Accept(listenfd, NULL, NULL)) == -1);

        /* Envoie la structure serveur correspondant pour que le client se connecte à l'esclave */
        serveurs[server_courant].port = htons(serveurs[server_courant].port);
        rio_writen(connfd, &serveurs[server_courant], sizeof(Serveur));
        serveurs[server_courant].port = ntohs(serveurs[server_courant].port);

        /* Ferme la connexion */
        Close(connfd);

        // On change d'esclave courant
        server_courant = (server_courant + 1) % MAX_SERVERS;
    }
}