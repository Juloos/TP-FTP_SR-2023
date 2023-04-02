//
// Created by tom on 02/04/23.
//
#ifndef TP_FTP_SR_2023_ESCLAVES_H
#define TP_FTP_SR_2023_ESCLAVES_H

#include "csapp.h"
#include "protocoles.h"

#define PORT 4242 ///< Port du serveur maître

#define SERVER_BODY_OK 0    ///< Code de retour OK
#define SERVER_BODY_ERR 1   ///< Code de retour erreur
#define SERVER_BODY_BYE 2   ///< Code de retour bye

#define CREERNFILS_PARENT 1   ///< Code de retour du père
#define CREERNFILS_CHILD 0    ///< Code de retour du fils

#ifndef NB_PROC
#define NB_PROC 5    ///< Nombre de processus fils
#endif

/**
 * @brief Code principal exécuté par le serveur
 * @param connfd Socket de connection
 * @return Code de retour en fonction du résultat(ok, erreur, bye)
 */
int server_body(int connfd);

#endif //TP_FTP_SR_2023_ESCLAVES_H
