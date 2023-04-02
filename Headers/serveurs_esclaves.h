//
// Created by tom on 02/04/23.
//
#ifndef TP_FTP_SR_2023_ESCLAVES_H
#define TP_FTP_SR_2023_ESCLAVES_H

#include "csapp.h"
#include "protocoles.h"

/**
 * @def PORT 4242
 * @brief Port du serveur maître
 */
#define PORT 4242

/**
 * @brief Valeurs de retour de la fonction server_body
 * @def SERVER_BODY_BYE 2
 * @def SERVER_BODY_ERR 1
 * @def SERVER_BODY_OK 0
 */
#define SERVER_BODY_BYE 2
#define SERVER_BODY_ERR 1
#define SERVER_BODY_OK 0

/**
 * @brief Variable contenant l'adresse IP du serveur maître
 */
extern char *master_ip;

/**
 * @brief Code principal exécuté par le serveur
 * @param connfd Socket de connection
 * @return Code de retour en fonction du résultat(ok, erreur, bye)
 */
int server_body(int connfd);

#endif //TP_FTP_SR_2023_ESCLAVES_H
