#ifndef TP_FTP_SR_2023_PROTOCOLES_H
#define TP_FTP_SR_2023_PROTOCOLES_H

#include <stdint.h>
#include "csapp.h"

// TODO : expliquer notre protocole dans le CR (schéma)

#define OP_GET 1 ///< Code pour la requête GET
#define OP_BYE 2 ///< Code pour la requête BYE
#define OP_INT 3 ///< Code pour l'interruption du serveur maître (etape 3)

#define REP_OK 1              ///< Réponse OK
#define REP_ERREUR 2          ///< Réponse d'erreur
#define REP_ERREUR_FICHIER 3  ///< Réponse d'erreur : fichier non trouvé
#define REP_ERREUR_MEMOIRE 4  ///< Réponse d'erreur : mémoire insuffisante
#define REP_ERREUR_CURSEUR 5  ///< Réponse d'erreur : curseur invalide

#define TAILLE_BLOCK 8192 ///< Taille d'un block de données (etape 2)

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
 * \struct
 * @brief Structure Serveur
 * @details Structure permettant l'échange d'information sur le serveur esclave
 * @var port Port associé à l'adress IP d'un serveur
 * @var ip IP d'un serveur
 */
typedef struct {
    uint16_t port;
    char ip[16];
} Serveur;


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
 * @return Nombre d'octets restants (en cas d'interruption)
 */
unsigned int reception_fichier(int clientfd, int f, unsigned int taille);

/**
 * @brief Envoi d'un fichier
 * @details Envoi d'un fichier vers un client
 * @param clientfd Socket du client
 * @param rep Réponse à envoyer
 * @param f Descripteur du fichier à envoyer
 * @param taille Taille du fichier
 */
void envoie_fichier(int clientfd, Reponse *rep, int f, unsigned int taille);

#ifndef MAX_SERVERS
#define MAX_SERVERS 3 ///< Nombre de serveurs esclaves
#endif

#endif //TP_FTP_SR_2023_PROTOCOLES_H
