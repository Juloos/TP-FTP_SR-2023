#include "../Headers/csapp.h"
#include "../Headers/serveurs_esclaves.h"

int volatile sigpipe = 0;

/* Même machine pour l'instant */
char *ip_servers[MAX_SERVERS] = {"127.0.0.1", "127.0.0.1", "127.0.0.1"};

int numero_port(int numero_esclave) {
    switch(numero_esclave) {
        case 0:
            return PORT1;
        case 1:
            return PORT2;
        case 2:
            return PORT3;
        default:
            return -1;
    }
}

char *hostname(int numero_esclave) {
    switch(numero_esclave) {
        case 0:
            return ip_servers[0];
        case 1:
            return ip_servers[1];
        case 2:
            return ip_servers[2];
        default:
            return NULL;
    }
}


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

    /* Affichage de l'ip */
    char hostName[256];
    gethostname(hostName, sizeof(hostName));
    struct hostent *host1 = gethostbyname(hostName);
    printf("Serveur : ip = %s\n", inet_ntoa(*(struct in_addr *) host1->h_addr_list[0]));

    listenfd = Open_listenfd(PORT);

    for(int i=0; i<MAX_SERVERS; i++) {
        fprintf(stderr, "port de l'esclave %d = %d\n", i+1, numero_port(i));
    }

    /* Attend la connexion des serveurs esclaves */
    while (nb_servers < MAX_SERVERS) {
        while ((connfd = Accept(listenfd, NULL, NULL)) == -1);
        uint8_t numero;
        if (rio_readn(connfd, &numero, sizeof(numero)) == 0) {
            fprintf(stderr, "Serveur : Un serveur esclave s'est déconnecté avant identification\n");
            continue;
        }
        serveurs[numero].port = numero_port(numero);
        fprintf(stderr, "Serveur : port de l'esclave %d = %d\n", numero, serveurs[numero].port);
        strcpy(serveurs[numero].ip, hostname(numero));
        printf("Serveur : serveur esclave %d connecté\n", numero);
        Close(connfd);
        nb_servers++;
    }

    /* Attend la connexion d'un client */
    while (1) {
        sigpipe = 0;
        while ((connfd = Accept(listenfd, NULL, NULL)) == -1);

        while (1) {
            /* Envoie la structure serveur correspondant pour que le client se connecte à l'esclave */
            rio_writen(connfd, &serveurs[server_courant], sizeof(Serveur));

            /* Ferme la connexion */
            Close(connfd);

            server_courant = (server_courant + 1) % MAX_SERVERS;
        }
    }
}