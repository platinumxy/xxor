#pragma once

#include <stdbool.h>
#include <netinet/in.h>

#include "circular_queue.h"
#include "promise.h"

typedef struct net_conn_conf {
    int server_port;
    const char *server_ip;
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
    net_conn_conf_t conf;
    int sockfd;
    struct sockaddr_in addr;
    int status;
} net_client_instance_t;

#define NET_CLNT_T net_client_instance_t*

typedef enum network_connection_type {
    SERVER,
    CLIENT
} network_connection_type_t;

typedef struct network_conn {
    network_connection_type_t conn_type;
    void *instance;
} network_conn_t;

net_conn_conf_t get_default_net_conf();

int get_available_bytes(const network_conn_t *conn);

/// =========== server management ===============

network_conn_t *start_server(const net_conn_conf_t *conf);

bool server_accept_connection(network_conn_t *conn);

/// =========== client management ===============

network_conn_t *connect_to_server(const net_conn_conf_t *conf);

/// =========== Backend threads =================

typedef struct network_interface {
    network_conn_t *conn;
    CQueue(u8_arr_promise) *send_queue;
    CQueue(u8_arr_promise) *recv_queue;
} network_interface_t;

network_interface_t *init_network_interface(const net_conn_conf_t *conf, bool is_server);

