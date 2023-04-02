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

#define INTERPRETE_REPONSE_OK 0         ///< La réponse est ok
#define INTERPRETE_REPONSE_PAS_OK (-1)  ///< La réponse est pas ok
#define INTERPRETE_REPONSE_ERR (-2)     ///< Erreur de communication

/**
 * @brief Interprète la réponse du serveur
 * @param clientfd socket de connection
 * @param rep réponse à interpréter
 * @return Code de retour en fonction du résultat (ok, pas ok, erreur)
 */
int interprete_reponse(int clientfd, Reponse *rep);

#endif //TP_FTP_SR_2023_CLIENT_INTERACTION_H
