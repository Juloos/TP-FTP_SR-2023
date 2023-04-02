//
// Created by tom on 02/04/23.
//
#ifndef TP_FTP_SR_2023_ESCLAVES_H
#define TP_FTP_SR_2023_ESCLAVES_H

#include "csapp.h"
#include "protocoles.h"

#define PORT 4242
#define PORT1 5000
#define PORT2 5001
#define PORT3 5002

#define SERVER_BODY_BYE 2
#define SERVER_BODY_ERR 1
#define SERVER_BODY_OK 0

extern char *master_ip;

int server_body(int connfd);

int numero_port(int numero_esclave);

char *hostname(int numero_esclave);

#endif //TP_FTP_SR_2023_ESCLAVES_H
