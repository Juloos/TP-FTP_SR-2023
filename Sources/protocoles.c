#include <arpa/inet.h>
#include "../Headers/protocoles.h"


void Requete_hton(Requete *req) {
    req->code = htons(req->code);
    req->arg_len = htons(req->arg_len);
}

void Requete_ntoh(Requete *req) {
    req->code = ntohs(req->code);
    req->arg_len = ntohs(req->arg_len);
}

void Reponse_hton(Reponse *rep) {
    rep->code = htons(rep->code);
    rep->res_len = htons(rep->res_len);
}

void Reponse_ntoh(Reponse *rep) {
    rep->code = ntohs(rep->code);
    rep->res_len = ntohs(rep->res_len);
}
