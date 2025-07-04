#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network_manager.h"
#include "debugging.h"


/// TODO
/// 1. Allow user to enable proxying through an external server
///     Prevents need for port forwarding
///     I.e. run a service on `xxor.platinumxy.dev` that allows for server connections
/// 2. Update code to allow for multiple convos at the same time
/// 3. Stretch decouple network manager from the executable
///     Allows multiple xxor instances to run at the same time

#define DEFAULT_SERVER_PORT 12321
#define DEFAULT_BINDING_ADDRESS "127.0.0.1"
#define DEFAULT_TIMEOUT 1000

net_conn_conf_t get_default_net_conf() {
    net_conn_conf_t con;
    con.server_port = DEFAULT_SERVER_PORT;
    con.server_binding_ip = DEFAULT_BINDING_ADDRESS;
    con.timeout = DEFAULT_TIMEOUT;
    return con;
}

bool init_server(net_server_instance_t *server, const net_conn_conf_t *conf) {
    if (!conf || !conf->server_binding_ip) {
        show_err("Invalid server configuration\n");
        return false;
    }
    server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sockfd < 0) {
        show_err("Failed to create server socket\n");
        return false;
    }

    memset(&server->addr, 0, sizeof(server->addr));
    server->addr.sin_family = AF_INET;
    server->addr.sin_port = htons(conf->server_port);

    if (inet_pton(AF_INET, conf->server_binding_ip, &server->addr.sin_addr) <= 0) {
        show_err("Failed to convert provided ip\n");
        close(server->sockfd);
    }

    server->conf.server_port = conf->server_port;
    server->conf.server_binding_ip = strdup(conf->server_binding_ip); // deep copy
    server->conf.timeout = conf->timeout;
    server->status = 0;
    server->conn_established = false;
    server->connfd = 0;

    if (bind(server->sockfd, (struct sockaddr *) &server->addr, sizeof(server->addr)) < 0) {
        show_err("Failed to bind server socket\n");
        close(server->sockfd);
        free((void *) server->conf.server_binding_ip);
        return false;
    }
    return true;
}

network_conn_t *start_server(const net_conn_conf_t *conf) {
    net_server_instance_t *server = calloc(1, sizeof(net_server_instance_t));
    if (server == NULL) {
        show_err("Unable to allocate memory for server instance\n");
        return NULL;
    }

    network_conn_t *conn = calloc(1, sizeof(network_conn_t));
    if (conn == NULL) {
        show_err("Unable to allocate memory for connection instance\n");
        free(server);
        return NULL;
    }
    conn->conn_type = SERVER_CONNECTION;
    conn->instance = (void *) server;
    if (!init_server(server, conf)) {
        free(server);
        free(conn);
        return NULL;
    }
    return conn;
}

bool server_accept_connection(network_conn_t *conn) {
    if (!(conn && conn->instance)) {
        show_err("Invalid connection instance\n");
        return false;
    }
    if (conn->conn_type != SERVER_CONNECTION) {
        show_err("Connection instance is not a server\n");
        return false;
    }

    NET_SRVR_T server = conn->instance;
    if (listen(server->sockfd, 1) != 0) {
        // backlog of only one as we only ever want client
        show_err("Server failed to listen on socket\n");
    }
    socklen_t len = (socklen_t) sizeof(server->clnt);
    server->connfd = accept(server->sockfd, (struct sockaddr *) &server->clnt, &len);

    if (server->connfd < 0) {
        show_err("Server failed to accept connection\n");
        return false;
    }
    server->conn_established = true;
    return true;
}
