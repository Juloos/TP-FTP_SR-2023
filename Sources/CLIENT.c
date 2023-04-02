#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include "../Headers/client_interaction.h"
#include <string.h>

#define PORT 4242 ///< Port du serveur maître

/**
 * @brief Chemin du dossier du client
 */
char pathname[MAXLINE];

/**
 * @brief Couleur du prompt
 */
int couleur = 31;

/**
 * @brief Fonction qui calcule la différence de temps entre deux timeval
 * @param start structure initialisée avec gettimeofday juste avant le début du téléchargement
 * @param end structure initialisée avec gettimeofday juste après la fin du téléchargement
 * @return le temps écoulé entre start et end
 */
float time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

/**
 * @brief Fonction qui affiche le prompt
 * @param buf Buffer dans lequel on stocke la commande
 * @return La taille de la commande
 */
size_t prompt(char *buf) {
    printf("\033[%dm", couleur);
    printf("\033[4mftp>\033[00m ");
    if ((couleur = (couleur + 1 - 31) % 18 + 31) == 34)
        couleur++;
    Fgets(buf, MAXLINE, stdin);
    size_t len = strlen(buf) - 1;
    if (len) buf[len] = '\0';  // Enlever le '\n'
    return len;
}

#define LIRE_COMMANDE_ERR (-1) ///< Macro qui permet de vérifier si la commande est valide

/**
 * @brief Fonction qui lit la commande entrée par l'utilisateur
 * @param buf Buffer dans lequel on stocke la commande
 * @param req Requete à remplir
 * @return L'indice de l'argument de la commande dans le buffer ou LIRE_COMMANDE_ERR si la commande est invalide
 */
int lire_commande(char *buf, Requete *req) {
    size_t len = prompt(buf);

    int argi = (int) len;

    if (strncmp(buf, "get", 3) == 0) {
        if (len < 5 || buf[3] != ' ') {
            fprintf(stderr, "Il manque un argument\n");
            return LIRE_COMMANDE_ERR;
        }
        req->code = OP_GET;
        req->arg_len = len - 3;
        argi = 4;

        // Nom du fichier temporaire tant qu'il n'est pas complet
        strcat(pathname, ".");
        strcat(pathname, buf + argi);

        if (access(pathname, F_OK) != -1) {
            // Le fichier temporaire existe déjà
            // On récupère la taille du fichier
            struct stat st;
            stat(pathname, &st);
            req->cursor = st.st_size;
        }
        pathname[strlen(pathname) - strlen(buf + argi) - 1] = '\0';

    } else if (strcmp(buf, "bye") == 0) {
        req->code = OP_BYE;
    } else {
        fprintf(stderr, "Commande inconnue\n");
        return LIRE_COMMANDE_ERR;
    }
    return argi;
}

/**
 * @brief Fonction qui execute la requête
 * @param clientfd socket de communication avec le serveur
 * @param req Structure de la requête à exécuter
 * @param rep Structure de la réponse du serveur
 * @param filename Nom du fichier à télécharger si le code de la requête est OP_GET
 * @param start Structure initialisée avec gettimeofday juste avant le début du téléchargement
 */
void execute_requete(int clientfd, Requete *req, Reponse *rep, char *filename, struct timeval start) {
#ifdef DEBUG
    fprintf(stderr, "Client: exécution de la requête %d, arg_len = %d, cursor = %d\n", req->code, req->arg_len, req->cursor);
#endif
    struct timeval end;
    int file = -1;

    switch (req->code) {
        case OP_GET:
            if (rep->res_len) {
                // Nom du fichier temporaire tant qu'il n'est pas complet
                char tmpname[MAXLINE];
                strcpy(tmpname, pathname);
                strcat(tmpname, ".");
                strcat(tmpname, filename);
                // Nom du fichier final
                strcat(pathname, filename);
#ifdef DEBUG
                fprintf(stderr, "filename = %s\n", pathname);
#endif
                // req->cursor modifié dans lire_commande
                if (req->cursor > 0) {
                    // On reprend la transmission
                    file = open(tmpname, O_WRONLY | O_APPEND, 0700);
                } else {
                    // On commence une nouvelle transmission
                    // S'il existe déjà, on le remplace
                    file = open(tmpname, O_CREAT | O_WRONLY | O_TRUNC, 0700);
                }
                unsigned int taille_restante = reception_fichier(clientfd, file, rep->res_len);
                gettimeofday(&end, NULL);
                Close(file);

                if (taille_restante != 0) {
                    printf("File transfer interrupted, partial file saved as %s\n", tmpname);
                    exit(EXIT_SUCCESS);
                }

                if (rename(tmpname, pathname) == -1) {
                    perror("rename");
                    exit(EXIT_FAILURE);
                }
                printf("File saved as %s\n", pathname);
                printf("%u bytes transferred in %.3f sec\n", rep->res_len - taille_restante, time_diff(&start, &end));

                pathname[strlen(pathname) - strlen(filename)] = '\0';
            }
            break;
        case OP_BYE:
            Close(clientfd);
            exit(EXIT_SUCCESS);
    }
}

/**
 * @brief Handler qui gère le signal SIGPIPE
 * @param sig Signal reçu
 */
void handler_SIGPIPE(int sig) {
    printf("Le serveur a fermé la connexion\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    Requete req;
    Reponse rep;
    char *arg;
    int argi;
    struct timeval start;

#ifdef DEBUG
    int n = 1;
    fprintf(stderr, "Endianess: %s\n", *(char *) &n == 1 ? "Little Endian" : "Big Endian");
#endif

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    host = argv[1];

    /* Get the working directory of the program */
    if (getcwd(pathname, sizeof(pathname)) == NULL) {
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");
        exit(EXIT_FAILURE);
    }
    strcat(pathname, "/.client/");

    Signal(SIGPIPE, handler_SIGPIPE);
    /* Connexion au serveur maître pour récupérer le numero de l'esclave */
    clientfd = Open_clientfd(host, PORT);

    /* Lecture de l'ip et le port du serveur esclave */
    Serveur serv;
    if (rio_readn(clientfd, &serv, sizeof(Serveur)) == 0) {
        fprintf(stderr, "Le serveur maître a fermé la connexion\n");
        exit(EXIT_FAILURE);
    }
    Close(clientfd);

    /* Connexion à l'esclave */
    serv.port = ntohs(serv.port);
    fprintf(stderr, "Connexion à l'esclave %s:%d\n", serv.ip, serv.port);
    clientfd = Open_clientfd(serv.ip, serv.port);

    while (1) {
        init_Requete(&req);
        if ((argi = lire_commande(buf, &req)) == -1)
            continue;
        else if (argi == -2)  // Bye reçu
            exit(EXIT_SUCCESS);
        arg = buf + argi;

        gettimeofday(&start, NULL);
        envoyer_requete(clientfd, &req, arg);

        /* Erreur avec rio_readn */
        switch (interprete_reponse(clientfd, &rep)) {
            case INTERPRETE_REPONSE_ERR:
                exit(EXIT_SUCCESS);
            case INTERPRETE_REPONSE_PAS_OK:
                continue;
            case INTERPRETE_REPONSE_OK:
                execute_requete(clientfd, &req, &rep, arg, start);
        }
    }
}
