#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <stdbool.h>

#include <poll.h>

#include "main.h"

#include <strings.h>
#include <unistd.h>
#include <sys/ioctl.h>


#include "debugging.h"
#include "key_management.h"
#include "network_manager.h"


bool dev_rand_fill(const size_t len, uint8_t *buffer) {
    getrandom(buffer, len, 0);
    return true;
}


void test_keys() {
    const char *file_name = "../../testing/example.key\x00";
    size_t len = 0xAAAA;
    uint8_t *otp_buffer = calloc(len, sizeof(uint8_t));
    xxor_meta_t *metadata = gen_new_xxor_key(len, otp_buffer, dev_rand_fill);
    if (save_new_xxor_key(file_name, otp_buffer, metadata)) {
        printf("Saved the key\n");
    } else {
        printf("Failed to save the key\n");
    }
    free(metadata);
    free(otp_buffer);

    xxor_instance_t *instance = load_file(file_name);
    printf("Loaded the instance\n");
    print_xxor_key(instance);

    for (int j = 0; j < 10; j++) {
        uint8_t err;
        uint8_t *buff = read_key_bytes(instance, 10, &err);
        if (buff == NULL) {
            printf("Failed to read key\n");
            dbg_print_err_read_key_bytes(err);
        } else {
            for (int i = 0; i < 10; i++) {
                printf("%02x ", buff[i]);
            }
            printf("\n");
        }
        free(buff);
    }
    print_xxor_key(instance);
    free_xxor_key(instance);
}

void test_net_server() {
    const net_conn_conf_t config = get_default_net_conf();
    network_conn_t *conn = start_server(&config);

    if (conn == NULL) {
        printf("Failed to start server\n");
        return;
    }
    printf("Server started, waiting for connection\n");

    if (server_accept_connection(conn)) {
        printf("Accepted connection\n");
    } else {
        printf("Failed to accept connection\n");
    }

    printf("Sending 0x41 0x64 times\n");
    char buff = 0x41;
    for (int i = 0; i < 100; i++) {
        write(((NET_SRVR_T) conn->instance)->connfd, &buff, 1);
    }

    printf("Sent all receiving 0x64 bytes\n");


    for (int i = 0; i < 100; i++) {
        recv(((NET_SRVR_T) conn->instance)->connfd, &buff, 1, 0);
        printf("%02x ", buff);
    }
    printf("\n");

    printf("Connection closed\n");
}

void test_net_client() {
    const net_conn_conf_t config = get_default_net_conf();
    network_conn_t *conn = connect_to_server(&config, "127.0.0.1", 12321);
    printf("connected to server, recv 0x64 bytes\n");

    char buff;
    for (int i = 0; i < 100; i++) {
        recv(((NET_CLNT_T) conn->instance)->sockfd, &buff, 1, 0);
        printf("%02x ", buff);
    }
    printf("\n");
    printf("Received all now sending 0x43 0x64 times\n");
    buff = 0x43;
    for (int i = 0; i < 100; i++) {
        write(((NET_CLNT_T) conn->instance)->sockfd, &buff, 1);
    }
    printf("now done\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        test_net_server();
    } else {
        test_net_client();
    }
}
