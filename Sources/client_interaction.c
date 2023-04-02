#include "../Headers/csapp.h"
#include "../Headers/protocoles.h"
#include "../Headers/client_interaction.h"

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

int interprete_reponse(int clientfd, Reponse *rep) {
    if (rio_readn(clientfd, rep, sizeof(Reponse)) == 0) {
        printf("Le serveur a fermé la connexion\n");
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
            printf("Serveur: erreur inconnue au bataillon\n");
            break;
        case REP_ERREUR_CURSEUR:
            printf("Serveur: erreur de curseur (reprise du transfère)\n");
            break;
    }
    return INTERPRETE_REPONSE_PAS_OK;
}
