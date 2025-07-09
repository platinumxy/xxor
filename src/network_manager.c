#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

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
    con.server_ip = DEFAULT_BINDING_ADDRESS;
    con.timeout = DEFAULT_TIMEOUT;
    return con;
}

int get_available_bytes(const network_conn_t *conn) {
    int sockfd;
    if (conn->conn_type == SERVER) {
        sockfd = ((NET_SRVR_T) conn->instance)->sockfd;
    } else if (conn->conn_type == CLIENT) {
        sockfd = ((NET_SRVR_T) conn->instance)->sockfd;
    } else {
        return -1;
    }

    int cnt;
    if (ioctl(sockfd, FIONREAD, &cnt) == -1) {
        show_err("Unable to read bytes available on the stream");
        return -1;
    }
    return cnt;
}

/// =========== server management ===============

bool init_server(net_server_instance_t *server, const net_conn_conf_t *conf) {
    if (!conf || !conf->server_ip) {
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

    if (inet_pton(AF_INET, conf->server_ip, &server->addr.sin_addr) <= 0) {
        show_err("Failed to convert provided ip\n");
        close(server->sockfd);
    }

    server->conf.server_port = conf->server_port;
    server->conf.server_ip = strdup(conf->server_ip); // deep copy
    server->conf.timeout = conf->timeout;
    server->status = 0;
    server->conn_established = false;
    server->connfd = 0;

    if (bind(server->sockfd, (struct sockaddr *) &server->addr, sizeof(server->addr)) < 0) {
        show_err("Failed to bind server socket\n");
        close(server->sockfd);
        free((void *) server->conf.server_ip);
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
    conn->conn_type = SERVER;
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
    if (conn->conn_type != SERVER) {
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

/// =========== client management ===============

network_conn_t *connect_to_server(const net_conn_conf_t *conf) {
    net_client_instance_t *client = calloc(1, sizeof(net_client_instance_t));
    if (conf->server_ip == NULL) {
        show_err("Unable to allocate memory for client instance\n");
        return NULL;
    }
    client->conf.server_port = conf->server_port;
    client->conf.server_ip = strdup(conf->server_ip);
    client->conf.timeout = conf->timeout;

    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->sockfd < 0) {
        show_err("Failed to create server socket\n");
        free(client);
        return NULL;
    }
    memset(&client->addr, 0, sizeof(client->addr));

    client->addr.sin_family = AF_INET;
    client->addr.sin_port = htons(client->conf.server_port);
    client->addr.sin_addr.s_addr = inet_addr(client->conf.server_ip);


    if (connect(client->sockfd, (struct sockaddr *) &client->addr, sizeof(client->addr)) != 0) {
        show_err("Failed to connect to server\n");
        free(client);
        return NULL;
    }

    network_conn_t *conn = calloc(1, sizeof(network_conn_t));
    if (conn == NULL) {
        show_err("Unable to allocate memory for connection instance\n");
        free(client);
        return NULL;
    }

    conn->conn_type = CLIENT;
    conn->instance = (void *) client;
    return conn;
}

/// =========== Backend threads =================

bool connection_alive() {
    // todo should be managed by a 3rd suporting thread that checks the status of the connection
    return true;
}

bool send_bytes(const network_conn_t *conn, const u8_arr_t *arr) {
    if (conn->conn_type == SERVER) {
        return send(((NET_SRVR_T) conn->instance)->sockfd, arr->data, arr->size, 0) == arr->size;
    }
    if (conn->conn_type == CLIENT) {
        return send(((NET_CLNT_T) conn->instance)->sockfd, arr->data, arr->size, 0) == arr->size;
    }
    show_err("Invalid connection type\n");
    return false;
}

void await_server_connected(const network_conn_t *conn) {
    if (conn->conn_type == SERVER) {
        while (!((NET_SRVR_T) conn->instance)->conn_established) {
            sleep_ms(10);
        }
    }
}

bool recv_bytes(const network_conn_t *conn, const u8_arr_t *arr) {
    if (conn->conn_type == SERVER) {
        return recv(((NET_SRVR_T) conn->instance)->sockfd, arr->data, arr->size, 0) == arr->size;
    }
    if (conn->conn_type == CLIENT) {
        return send(((NET_CLNT_T) conn->instance)->sockfd, arr->data, arr->size, 0) == arr->size;
    }
    show_err("Invalid connection type\n");
    return false;
}

void *send_thread(void *args) {
    network_interface_t *iface = args;
    network_conn_t *conn = iface->conn;
    CQueue(u8_arr_promise) *send_queue = iface->send_queue;


    promise_t(u8_arr) *prms = calloc(1, sizeof(promise_t(u8_arr)));

    await_server_connected(conn);
    while (true) {
        if (send_queue == NULL || !connection_alive()) {
            free(prms);
            return NULL;
        }
        if (send_queue->meta.cnt > 0 && queue_pop(u8_arr_promise, send_queue, &prms)) {
            if (send_bytes(conn, prms->value)) {
                prms->state = PROMISE_FULFILLED;
            }
            prms->state = PROMISE_REJECTED;
        } else {
            sleep_ms(10);
        }
    }
}

void *recv_thread(void *args) {
    network_interface_t *iface = args;
    network_conn_t *conn = iface->conn;
    CQueue(u8_arr_promise) *recv_queue = iface->recv_queue;


    promise_t(u8_arr) *prms = calloc(1, sizeof(promise_t(u8_arr)));

    await_server_connected(conn);
    while (true) {
        if (recv_queue == NULL || !connection_alive()) {
            free(prms);
            return NULL;
        }

        if (recv_queue->meta.cnt > 0 && queue_peek(u8_arr_promise, recv_queue, &prms)) {
            if (get_available_bytes(conn) >= (int) prms->value->size) {
                queue_pop(u8_arr_promise, recv_queue, &prms);
                if (recv_bytes(conn, prms->value)) {
                    prms->state = PROMISE_FULFILLED;
                } else {
                    prms->state = PROMISE_REJECTED;
                }
                continue;
            }
        }
        sleep_ms(10);
    }
}


network_interface_t *init_network_interface(const net_conn_conf_t *conf, bool is_server) {
    network_conn_t *conn;
    if (is_server) {
        conn = start_server(conf);
    } else {
        conn = connect_to_server(conf);
    }
    if (conn == NULL) {
        show_err("Failed to connect to server\n");
        return NULL;
    }
    network_interface_t *iface = calloc(1, sizeof(network_interface_t));
    if (iface == NULL) {
        show_err("Unable to allocate memory for network interface\n");
        free(conn);
        return NULL;
    }
    iface->conn = conn;
    iface->recv_queue = queue_init(u8_arr_promise);
    iface->send_queue = queue_init(u8_arr_promise);
    pthread_t recv_thread_ptr, send_thread_ptr; // Ehhhhh, I dont think we'd ever need to join them
    pthread_create(&recv_thread_ptr, NULL, recv_thread, iface);
    pthread_create(&send_thread_ptr, NULL, send_thread, iface);
    return iface;
}

/// ========== Handshake code ===================
/*
+---------------------------------------------------------------+--------------------------------------------------+
|                          Handshake                            |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
|            Server             |             Client            |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
| 0xFF (Give magic bytes)       |                               |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
|                               | 0x58584F52 (XXOR Magic bytes) |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
| bool byte for accepted        |                               |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
|                               | [random u128]                 | send me sha256 of the key salted with u128      |
+-------------------------------+-------------------------------+--------------------------------------------------+
| sha256 of the pad             |                               | salted with u128                                 |
+-------------------------------+-------------------------------+--------------------------------------------------+
|                               | bool byte for accepted        |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
| [random u128]                 |                               | send me sha256 of the key salted with u128      |
+-------------------------------+-------------------------------+--------------------------------------------------+
|                               | sha256 of the pad             | salted with u128                                 |
+-------------------------------+-------------------------------+--------------------------------------------------+
| bool byte for accepted        |                               |                                                  |
+-------------------------------+-------------------------------+--------------------------------------------------+
*/
