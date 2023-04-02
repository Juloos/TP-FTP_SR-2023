#ifndef TP_FTP_SR_2023_SERVER_BODY_H
#define TP_FTP_SR_2023_SERVER_BODY_H

#define SERVER_BODY_OK 0
#define SERVER_BODY_ERR 1
#define SERVER_BODY_BYE 2
int server_body(int connfd);

#endif //TP_FTP_SR_2023_SERVER_BODY_H
