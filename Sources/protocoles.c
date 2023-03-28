#include <arpa/inet.h>
#include "../Headers/protocoles.h"


void Requete_hton(Requete *req) {
    req->arg_len = htonl(req->arg_len);
}

void Requete_ntoh(Requete *req) {
    req->arg_len = ntohl(req->arg_len);
}

void Reponse_hton(Reponse *rep) {
    rep->res_len = htonl(rep->res_len);
}

void Reponse_ntoh(Reponse *rep) {
    rep->res_len = ntohl(rep->res_len);
}
