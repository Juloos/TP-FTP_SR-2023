#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

#include <stdint.h>
#include "csapp.h"

// TODO : expliquer notre protocole dans le CR (schéma)

/**
 * @brief Codes des requêtes
 * @details Les codes des requêtes sont définis par les macros suivantes :
 * @var OP_GET Récupération d'un fichier
 * @var OP_BYE Fermeture de la connexion
 * @var OP_TERM Fermeture du serveur esclave
 * @def OP_GET 1
 * @def OP_BYE 2
 */
#define OP_GET 1
#define OP_BYE 2

/**
 * @brief Codes des réponses
 * @details Les codes des réponses sont définis par les macros suivantes :
 * @var REP_OK Réponse OK
 * @var REP_ERREUR Réponse d'erreur
 * @var REP_ERREUR_FICHIER Réponse d'erreur : fichier non trouvé
 * @var REP_ERREUR_MEMOIRE Réponse d'erreur : mémoire insuffisante
 * @var REP_ERREUR_CURSEUR Réponse d'erreur : curseur invalide
 * @def REP_OK 1
 * @def REP_ERREUR 2
 * @def REP_ERREUR_FICHIER 3
 * @def REP_ERREUR_MEMOIRE 4
 * @def REP_ERREUR_CURSEUR 5
 */
#define REP_OK 1
#define REP_ERREUR 2
#define REP_ERREUR_FICHIER 3
#define REP_ERREUR_MEMOIRE 4
#define REP_ERREUR_CURSEUR 5

/**
 * @brief Taille d'un block
 * @details La taille d'un block est définie par la macro suivante :
 * @var TAILLE_BLOCK Taille d'un block
 * @def TAILLE_BLOCK 8192
 */
#define TAILLE_BLOCK 8192

/**
 * \struct Requete
 * @brief Structure représentant une requête
 * @details La structure contient le code de la requête, la taille de l'argument et le curseur (pour la reprise de transfert)
 * @var code Code de la requête
 * @var arg_len Taille de l'argument (taille du nom du fichier pour OP_GET)
 * @var cursor Curseur pour la reprise de transfert (0 si pas de reprise)
 */
typedef struct {
    uint8_t code;
    uint32_t arg_len;
    uint32_t cursor;
} Requete;

/**
 * \struct Reponse
 * @brief Structure représentant une réponse
 * @details La structure contient le code de la réponse (erreur ou non) et la taille du fichier (pour OP_GET)
 * @var code Code de la réponse
 * @var res_len Taille de la réponse (taille du fichier pour OP_GET)
 */
typedef struct {
    uint8_t code;
    uint32_t res_len;
} Reponse;

/**
 * @brief Initialisation d'une requête
 * @details Initialise une requête avec le code OP_BYE et les champs arg_len et cursor à 0
 * @param req Requête à initialiser
 */
void init_Requete(Requete *req);

/**
 * @brief Initialisation d'une réponse
 * @details Initialise une réponse avec le code REP_OK et le champ res_len à 0
 * @param rep Réponse à initialiser
 */
void init_Reponse(Reponse *rep);

/**
 * @brief Modification de l'ordre des bytes d'une requête
 * @details Modifie l'ordre des bytes d'une requête pour qu'elle soit compatible avec le protocole
 * @param req Requête à modifier
 */
void Requete_hton(Requete *req);

/**
 * @brief Modification de l'ordre des bytes d'une réponse
 * @details Modifie l'ordre des bytes d'une réponse pour qu'elle soit compatible avec le protocole
 * @param rep Réponse à modifier
 */
void Requete_ntoh(Requete *req);

/**
 * @brief Modification de l'ordre des bytes d'une réponse
 * @details Modifie l'ordre des bytes d'une réponse pour qu'elle soit compatible avec le protocole
 * @param rep Réponse à modifier
 */
void Reponse_hton(Reponse *rep);

/**
 * @brief Modification de l'ordre des bytes d'une réponse
 * @details Modifie l'ordre des bytes d'une réponse pour qu'elle soit compatible avec le protocole
 * @param rep Réponse à modifier
 */
void Reponse_ntoh(Reponse *rep);

/**
 * @brief Réception d'un fichier
 * @details Réception d'un fichier depuis un client
 * @param clientfd Socket du client
 * @param f Descripteur du fichier à écrire
 * @param taille Taille du fichier
 */
void reception_fichier(int clientfd, int f, unsigned int taille);

/**
 * @brief Envoi d'un fichier
 * @details Envoi d'un fichier vers un client
 * @param rep Réponse à envoyer
 * @param clientfd Socket du client
 * @param f Descripteur du fichier à envoyer
 * @param taille Taille du fichier
 */
void envoie_fichier(Reponse rep, int clientfd, int f, unsigned int taille);

/**
 * @brief Macro pour le nombre d'esclaves
 * @var MAX_SERVERS nombre de serveurs esclaves
 * @def MAX_SERVERS 3
 */
#ifndef MAX_SERVERS
#define MAX_SERVERS 3
#endif

/**
 * \struct
 * @brief Structure Serveur
 * @details Structure permettant l'échange d'information sur le serveur esclave
 * @var port Port associé à l'adress IP d'un serveur
 * @var ip IP d'un serveur
 */
typedef struct {
    int port;
    char ip[16];
} Serveur;

#endif //TP_FTP_SR_2023_PROTOCOLES_H
