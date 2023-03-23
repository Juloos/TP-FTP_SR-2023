#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

#define MAXLEN 4096

typedef enum {
    OP_GET,
    OP_BYE
} Op;

typedef enum {
    ERREUR_OK,
    ERREUR_KO
} Erreur;

typedef struct {
    Op code;
    char arg[MAXLEN];
} Requete;

typedef struct {
    Erreur code;
} Reponse;

#endif //TP_FTP_SR_2023_PROTOCOLES_H
