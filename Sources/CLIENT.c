#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include <string.h>
#include <time.h>

#define PORT 4242

void handler_SIGPIPE(int sig) {
    printf("Le serveur a fermé la connexion\n");
    exit(EXIT_SUCCESS);
}

float time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    size_t len;
    char filename[MAXLINE];
    int f;

    struct timeval start;
    struct timeval end;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    host = argv[1];

    // Get the working directory of the client
    if (getcwd(filename, sizeof(filename)) == NULL) {
        fprintf(stderr, "Erreur lors de l'obtention du répertoire courant\n");
        exit(EXIT_FAILURE);
    }
    strcat(filename, "/.client/");

    Signal(SIGPIPE, handler_SIGPIPE);
    clientfd = Open_clientfd(host, PORT);

    // à placer dans une boucle par la suite
    while (1) {
        printf("ftp> ");
        Fgets(buf, MAXLINE, stdin);
        len = strlen(buf) - 1;
        if (len) buf[len] = '\0';  // Enlever le '\n'

        Requete req = {OP_BYE, 0, 0};
        char *arg = NULL;


        if (strncmp(buf, "get ", 4) == 0) {
            req.code = OP_GET;
            if (len < 5) {
                fprintf(stderr, "Il manque un argument\n");
                continue;
            }
            req.arg_len = len - 4;
            arg = buf + 4;
        } else if (strncmp(buf, "bye ", 4) == 0 || strncmp(buf, "bye", 3) == 0) {
            req.code = OP_BYE;
            fprintf(stderr, "Commande bye reçue\n");
            Requete_hton(&req);
            Rio_writen(clientfd, &req, sizeof(Requete));
            Close(clientfd);
            exit(EXIT_SUCCESS);
        } else if (strncmp(buf, "get", 3) == 0 && len == 3) {
            fprintf(stderr, "Il manque un argument\n");
            continue;
        } else {
            fprintf(stderr, "Commande inconnue\n");
            continue;
        }

        //time_t start = time(NULL);
        gettimeofday(&start, NULL);
        len = req.arg_len;  // Avoid endianess issues with self

        strcat(filename, arg);

        // Si le fichier existe déjà, on se place à la fin
        if (access(filename, F_OK) != -1) {
            fprintf(stderr, "Le fichier existe déjà\n"); // Debug
            f = Open(filename, O_APPEND, 0700);
            // On récupère la taille du fichier pour deplacer le curseur du serveur
            req.cursor = Lseek(f, 0, SEEK_END);
        } else {
            f = Open(filename, O_CREAT | O_WRONLY, 0700);
        }
        filename[strlen(filename) - len] = '\0';
        Requete_hton(&req);
        // Envoie de la structure requête
        Rio_writen(clientfd, &req, sizeof(Requete));
        // Envoie du nom du fichier
        Rio_writen(clientfd, arg, len);

        Reponse rep;
        Rio_readn(clientfd, &rep, sizeof(Reponse));
        Reponse_ntoh(&rep);
        switch (rep.code) {
            case REP_OK:
                switch (req.code) {
                    case OP_GET:
                        if (rep.res_len) {
                            unsigned int taille = rep.res_len - req.cursor;
                            reception_fichier(clientfd, f, taille);
                            gettimeofday(&end, NULL);
                            printf("%u bytes transferred in %f sec\n", rep.res_len - req.cursor, time_diff(&start, &end));
                            Close(f);
                        }
                        break;
                    case OP_BYE:
                        Close(clientfd);
                        exit(EXIT_SUCCESS);
                }
                break;
            case REP_ERREUR_FICHIER:
                printf("Serveur: le fichier n'existe pas\n");
                break;
            case REP_ERREUR_MEMOIRE:
                printf("Serveur: erreur de mémoire\n");
                break;
            case REP_ERREUR:
                printf("Serveur: erreur\n");
                break;
            case REP_FICHIER_EXISTE:
                printf("Serveur: le fichier existe déjà en entier\n");
                break;
            case REP_ERREUR_CURSEUR:
                printf("Serveur: erreur de curseur\n");
                break;
        }
    }
}
