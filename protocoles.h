#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

// TODO : expliquer notre protocole dans le CR (schéma)

#define MAXLEN 4096

typedef enum {
    OP_LS,
    OP_CD,
    OP_PWD,
    OP_GET,
    OP_PUT,
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
    Erreur code; // 0 = OK, 1 = KO
} Reponse;

#endif //TP_FTP_SR_2023_PROTOCOLES_H
