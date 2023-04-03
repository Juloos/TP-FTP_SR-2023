#ifndef TP_FTP_SR_2023_CLIENT_INTERACTION_H
#define TP_FTP_SR_2023_CLIENT_INTERACTION_H

#include "protocoles.h"


void envoyer_requete(int clientfd, Requete *req, char *arg);

#define INTERPRETE_REPONSE_OK 0
#define INTERPRETE_REPONSE_PAS_OK (-1)
#define INTERPRETE_REPONSE_ERR (-2)
int interprete_reponse(int clientfd, Reponse *rep);

#endif //TP_FTP_SR_2023_CLIENT_INTERACTION_H
