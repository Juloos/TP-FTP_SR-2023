#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

#include <stdint.h>

// TODO : expliquer notre protocole dans le CR (schéma)

typedef enum {
    OP_GET,
    OP_BYE
} Op;

typedef enum {
    OK,
    ERREUR,
    ERREUR_FICHIER,
    ERREUR_MEMOIRE
} Rep;

typedef struct {
    Op code;
    uint16_t arg_len;
} Requete;

typedef struct {
    Rep code;
    uint16_t res_len;
} Reponse;


void Requete_hton(Requete *req);

void Requete_ntoh(Requete *req);

void Reponse_hton(Reponse *rep);

void Reponse_ntoh(Reponse *rep);

#endif //TP_FTP_SR_2023_PROTOCOLES_H
