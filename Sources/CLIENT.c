#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include <string.h>

#define PORT 4242

char pathname[MAXLINE];


float time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

void envoyer_requete(int clientfd, Requete *req, char *arg) {
#ifdef DEBUG
    fprintf(stderr, "Client: envoi de la requête %d, arg_len = %d, cursor = %d\n", req->code, req->arg_len, req->cursor);
#endif
    Requete_hton(req);
    rio_writen(clientfd, req, sizeof(Requete));
    Requete_ntoh(req);
    if (arg != NULL && req->arg_len > 0)
        rio_writen(clientfd, arg, req->arg_len);
}

#define INTERPRETE_REPONSE_OK 0
#define INTERPRETE_REPONSE_PAS_OK (-1)
#define INTERPRETE_REPONSE_ERR (-2)

int interprete_reponse(int clientfd, Reponse *rep) {
    if (rio_readn(clientfd, rep, sizeof(Reponse)) == 0) {
        fprintf(stderr, "Serveur: erreur de lecture\n");
        return INTERPRETE_REPONSE_ERR;
    }
    Reponse_ntoh(rep);
    switch (rep->code) {
        case REP_OK:
            return INTERPRETE_REPONSE_OK;
        case REP_ERREUR_FICHIER:
            printf("Serveur: le fichier n'existe pas\n");
            break;
        case REP_ERREUR_MEMOIRE:
            printf("Serveur: erreur de mémoire, faut augmenter la RAM mon biquet\n");
            break;
        case REP_ERREUR:
            printf("Serveur: erreur\n");
            break;
        case REP_ERREUR_CURSEUR:
            printf("Serveur: erreur de curseur (reprise du transfère)\n");
            break;
    }
    return INTERPRETE_REPONSE_PAS_OK;
}

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
                reception_fichier(clientfd, file, rep->res_len);
                Close(file);

                gettimeofday(&end, NULL);
                printf("%u bytes transferred in %.3f sec\n", rep->res_len, time_diff(&start, &end));

                if (rename(tmpname, pathname) == -1) {
                    perror("rename");
                    exit(EXIT_FAILURE);
                }
                printf("File saved as %s\n", pathname);

                pathname[strlen(pathname) - strlen(filename)] = '\0';
            }
            break;
        case OP_BYE:
            Close(clientfd);
            exit(EXIT_SUCCESS);
    }
}

size_t prompt(char *buf, int couleur) {
    printf("\033[%dm", couleur);
    printf("\033[4mftp>\033[00m");
    printf(" ");
    Fgets(buf, MAXLINE, stdin);
    size_t len = strlen(buf) - 1;
    if (len) buf[len] = '\0';  // Enlever le '\n'
    return len;
}

#define LIRE_COMMANDE_ERR (-1)

int lire_commande(char *buf, Requete *req, int couleur) {
    size_t len = prompt(buf, couleur);

    int argi = (int) len;

    if (strncmp(buf, "get", 3) == 0) {
        if ((buf[3] == ' ' && len == 4) || len == 3) {
            fprintf(stderr, "Il manque un argument\n");
            return LIRE_COMMANDE_ERR;
        } else if (buf[3] != ' ') {
            fprintf(stderr, "Commande inconnue\n");
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
    fprintf(stderr, "Connexion à l'esclave %s:%d\n", serv.ip, serv.port);
    clientfd = Open_clientfd(serv.ip, serv.port);

    /* Code de base de l'etape 2 */
    int couleur = 31;

    while (1) {
        init_Requete(&req);
        if ((argi = lire_commande(buf, &req, couleur)) == -1) {
            if ((couleur = (couleur + 1 - 31) % 18 + 31) == 34) { couleur++; };
            continue;
        } else if (argi == -2) {
            // Bye reçu
            Close(clientfd);
            exit(EXIT_SUCCESS);
        }
        if ((couleur = (couleur + 1 - 31) % 18 + 31) == 34) couleur++;
        arg = buf + argi;

        gettimeofday(&start, NULL);
        envoyer_requete(clientfd, &req, arg);

        /* Erreur avec rio_readn */
        switch (interprete_reponse(clientfd, &rep)) {
            case INTERPRETE_REPONSE_ERR:
                Close(clientfd);
                exit(EXIT_FAILURE);
            case INTERPRETE_REPONSE_PAS_OK:
                continue;
        }
        execute_requete(clientfd, &req, &rep, arg, start);
    }
}
