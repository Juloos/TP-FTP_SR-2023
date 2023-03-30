#include <arpa/inet.h>
#include "../Headers/protocoles.h"
#include "csapp.h"


void init_Requete(Requete *req) {
    req->code = OP_BYE;
    req->arg_len = 0;
    req->cursor = 0;
}

void Requete_hton(Requete *req) {
    req->arg_len = htonl(req->arg_len);
    req->cursor = htonl(req->cursor);
}

void Requete_ntoh(Requete *req) {
    req->arg_len = ntohl(req->arg_len);
    req->cursor = ntohl(req->cursor);
}

void Reponse_hton(Reponse *rep) {
    rep->res_len = htonl(rep->res_len);
}

void Reponse_ntoh(Reponse *rep) {
    rep->res_len = ntohl(rep->res_len);
}

void reception_fichier(int clientfd, int f, unsigned int taille) {
    char *res = (char *) malloc(TAILLE_BLOCK);
    fprintf(stderr, "Réception du fichier (taille = %d)\n", taille);
    while (taille >= TAILLE_BLOCK) {
        fprintf(stderr, "Réception d'un paquet (taille = %d)\n", TAILLE_BLOCK);
        rio_readn(clientfd, res, TAILLE_BLOCK);
        Write(f, res, TAILLE_BLOCK);
        taille -= TAILLE_BLOCK;
    }
    // Dernier paquet (taille < TAILLE_BLOCK)
    fprintf(stderr, "Réception du dernier paquet (taille = %d)\n", taille);
    rio_readn(clientfd, res, taille);
    Write(f, res, taille);
    free(res);
}

void envoie_fichier(Reponse rep, int clientfd, int f, unsigned int taille) {
    fprintf(stderr, "Envoi du fichier (taille = %d)\n", taille);
    char *res = (char *) malloc(TAILLE_BLOCK);
    if (res == NULL) {
        rep.code = REP_ERREUR_MEMOIRE;
        // Envoie de la structure de réponse (code + taille)
        Reponse_hton(&rep);
        rio_writen(clientfd, &rep, sizeof(Reponse));
        return;
    } else {
        // Envoie de la structure de réponse (code + taille)
        Reponse_hton(&rep);
        rio_writen(clientfd, &rep, sizeof(Reponse));
        while (taille > 0) {
            fprintf(stderr, "taille = %d\n", taille);
            if (taille < TAILLE_BLOCK) {
                fprintf(stderr, "Envoi du dernier paquet (taille = %d)\n", taille);
                // Dernier paquet
                read(f, res, taille);
                rio_writen(clientfd, res, taille);
                break;
            }
            fprintf(stderr, "Envoi d'un paquet (taille = %d)\n", TAILLE_BLOCK);
            read(f, res, TAILLE_BLOCK);
            rio_writen(clientfd, res, TAILLE_BLOCK);
            taille -= TAILLE_BLOCK;
        }
    }
    free(res);
}