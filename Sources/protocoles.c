#include <arpa/inet.h>
#include "../Headers/protocoles.h"
#include "csapp.h"


void init_Requete(Requete *req) {
    req->code = OP_BYE;
    req->arg_len = 0;
    req->cursor = 0;
}

void init_Reponse(Reponse *rep) {
    rep->code = REP_OK;
    rep->res_len = 0;
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
    char res[TAILLE_BLOCK];
#ifdef DEBUG
    fprintf(stderr, "Réception du fichier (taille = %d)\n", taille);
#endif
    while (taille >= TAILLE_BLOCK) {
#ifdef DEBUG
        fprintf(stderr, "Réception d'un paquet (taille = %d)\n", TAILLE_BLOCK);
#endif
        rio_readn(clientfd, res, TAILLE_BLOCK);
        Write(f, res, TAILLE_BLOCK);
        taille -= TAILLE_BLOCK;
    }
    // Dernier paquet (taille < TAILLE_BLOCK)
#ifdef DEBUG
    fprintf(stderr, "Réception du dernier paquet (taille = %d)\n", taille);
#endif
    rio_readn(clientfd, res, taille);
    Write(f, res, taille);
}

void envoie_fichier(Reponse rep, int clientfd, int f, unsigned int taille) {
#ifdef DEBUG
    fprintf(stderr, "Envoi du fichier (taille = %d)\n", taille);
#endif
    char res[TAILLE_BLOCK];
    // Envoie de la structure de réponse (code + taille)
    Reponse_hton(&rep);
    rio_writen(clientfd, &rep, sizeof(Reponse));
    while (taille > 0) {
#ifdef DEBUG
        fprintf(stderr, "taille = %d\n", taille);
#endif
        if (taille < TAILLE_BLOCK) {
#ifdef DEBUG
            fprintf(stderr, "Envoi du dernier paquet (taille = %d)\n", taille);
#endif
            // Dernier paquet
            read(f, res, taille);
            rio_writen(clientfd, res, taille);
            break;
        }
#ifdef DEBUG
        fprintf(stderr, "Envoi d'un paquet (taille = %d)\n", TAILLE_BLOCK);
#endif
        read(f, res, TAILLE_BLOCK);
        rio_writen(clientfd, res, TAILLE_BLOCK);
        taille -= TAILLE_BLOCK;
    }
}