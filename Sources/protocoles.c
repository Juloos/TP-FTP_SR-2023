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

unsigned int reception_fichier(int clientfd, int f, unsigned int taille) {
    char progressbar[81];
    int i;
    double pourc = 0.0;
    for (i = 0; i < 80; i++) progressbar[i] = ' ';
    progressbar[80] = '\0';
    printf("[%s]  %5.1f%%\r", progressbar, pourc);
    i = 0;

    char res[TAILLE_BLOCK];
    unsigned int taille_restante = taille;
    while (taille_restante >= TAILLE_BLOCK) {
        if (rio_readn(clientfd, res, TAILLE_BLOCK) == 0) {
            printf("\n");
            Close(clientfd);
            return taille_restante;
        }
        Write(f, res, TAILLE_BLOCK);
        taille_restante -= TAILLE_BLOCK;

        // Mise à jour de la barre de progression
        pourc = 80.0 * (double) (taille - taille_restante) / (double) taille;
        while (i < pourc)
            progressbar[i++] = '=';
        printf("[%s]  %5.1f%%\r", progressbar, pourc * 1.25);
    }
    // Dernier paquet (taille < TAILLE_BLOCK)
    if (rio_readn(clientfd, res, taille_restante) == 0) {
        printf("\n");
        Close(clientfd);
        return taille_restante;
    }
    Write(f, res, taille_restante);
    while (i < 80)
        progressbar[i++] = '=';
    printf("[%s]  %5.1f%%\n", progressbar, 100.0);
    return 0;
}

void envoie_fichier(int clientfd, Reponse *rep, int f, unsigned int taille) {
#ifdef DEBUG
    fprintf(stderr, "Envoi du fichier (taille = %d)\n", taille);
#endif
    char res[TAILLE_BLOCK];
    // Envoie de la structure de réponse (code + taille)
    Reponse_hton(rep);
    rio_writen(clientfd, rep, sizeof(Reponse));
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