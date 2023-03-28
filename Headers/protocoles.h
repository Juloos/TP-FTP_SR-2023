#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

#include <stdint.h>

// TODO : expliquer notre protocole dans le CR (schéma)

#define OP_GET 1
#define OP_BYE 2

#define REP_OK 1
#define REP_ERREUR 2
#define REP_ERREUR_FICHIER 3
#define REP_ERREUR_MEMOIRE 4

#define TAILLE_BLOCK 8192

typedef struct {
    char code;
    uint32_t arg_len;
} Requete;

typedef struct {
    char code;
    uint32_t res_len;
} Reponse;


void Requete_hton(Requete *req);

void Requete_ntoh(Requete *req);

void Reponse_hton(Reponse *rep);

void Reponse_ntoh(Reponse *rep);

#endif //TP_FTP_SR_2023_PROTOCOLES_H
