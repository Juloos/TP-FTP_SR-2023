#ifndef TP_FTP_SR_2023_CLIENT_INTERACTION_H
#define TP_FTP_SR_2023_CLIENT_INTERACTION_H

#include "protocoles.h"

/**
 * @brief Envoie une requête au serveur
 * @param clientfd socket de connection
 * @param req requête à envoyer
 * @param arg argument de la requête si OP_GET
 */
void envoyer_requete(int clientfd, Requete *req, char *arg);

/**
 * @brief Macros pour les codes de retour de la fonction interprete_reponse
 * @def INTERPRETE_REPONSE_OK 0
 * @def INTERPRETE_REPONSE_PAS_OK -1
 * @def INTERPRETE_REPONSE_ERR -2
 */
#define INTERPRETE_REPONSE_OK 0
#define INTERPRETE_REPONSE_PAS_OK (-1)
#define INTERPRETE_REPONSE_ERR (-2)

/**
 * @brief Interprète la réponse du serveur
 * @param clientfd socket de connection
 * @param rep réponse à interpréter
 * @return Code de retour en fonction du résultat (ok, pas ok, erreur)
 */
int interprete_reponse(int clientfd, Reponse *rep);

#endif //TP_FTP_SR_2023_CLIENT_INTERACTION_H
