#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

#include <stdint.h>
#include "csapp.h"

// TODO : expliquer notre protocole dans le CR (schéma)

#define OP_GET 1
#define OP_BYE 2

#define REP_OK 1
#define REP_ERREUR 2
#define REP_ERREUR_FICHIER 3
#define REP_ERREUR_MEMOIRE 4
#define REP_ERREUR_CURSEUR 5

#define TAILLE_BLOCK 8192

typedef struct {
    uint8_t code;
    uint32_t arg_len;
    uint32_t cursor;
} Requete;

typedef struct {
    uint8_t code;
    uint32_t res_len;
} Reponse;

// Remplie la structure avec des valeurs par défaut
void init_Requete(Requete *req);

void init_Reponse(Reponse *rep);

void Requete_hton(Requete *req);

void Requete_ntoh(Requete *req);

void Reponse_hton(Reponse *rep);

void Reponse_ntoh(Reponse *rep);

void reception_fichier(int clientfd, int f, unsigned int taille);

void envoie_fichier(Reponse rep, int clientfd, int f, unsigned int taille);

#define MAX_SERVERS 3

typedef struct {
    int port;
    char ip[MAXLINE];
} Serveur;

#endif //TP_FTP_SR_2023_PROTOCOLES_H
