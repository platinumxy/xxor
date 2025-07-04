#pragma once

#include <stdbool.h>
#include <netinet/in.h>

typedef struct net_conn_conf {
    int server_port;
    const char *server_binding_ip;
    int timeout;
} net_conn_conf_t;


typedef struct net_server_instance {
    net_conn_conf_t conf;
    int sockfd;
    struct sockaddr_in addr;
    int status;

    bool conn_established;
    int connfd;
    struct sockaddr_in clnt;
} net_server_instance_t;

#define NET_SRVR_T net_server_instance_t*

typedef struct net_client_instance {
    const char *server_ip;
    int server_port;
} net_client_instance_t;

#define NET_CLNT_T net_client_instance_t*

typedef struct network_conn {
    char conn_type;
    void *instance;
} network_conn_t;

#define CLIENT_CONNECTION 0
#define SERVER_CONNECTION 1


net_conn_conf_t get_default_net_conf();

network_conn_t *start_server(const net_conn_conf_t *conf);

bool server_accept_connection(network_conn_t *conn);
