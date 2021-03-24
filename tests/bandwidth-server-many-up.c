/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <math.h>

#include <modbus.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define NB_CONNECTION    5

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;

static int server_socket = -1;

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

int main(void)
{
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;

    ctx = modbus_new_tcp("0.0.0.0", 1502);

    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    int16_t i16 = 0;
    int32_t i32 = -5555555;
    int64_t i64 = 0x8FAB3A9DFE0C88;
    uint32_t u32 = 0xFFAB3A9D;
    uint64_t u64 = 0x8FAB3A9DFE0C88;

    /* All the values for signed int (8, 16, 32, 64) */
    i16 = -120;
    mb_mapping->tab_registers[0x000] = (uint16_t)i16;
    i16 = -500;
    mb_mapping->tab_registers[0x010] = (uint16_t)i16;
    MODBUS_SET_INT32_TO_INT16(mb_mapping->tab_registers, 0x020, i32);
    MODBUS_SET_INT64_TO_INT16(mb_mapping->tab_registers, 0x030, i64);


    /* All the values for unsigned int (8, 16, 32, 64) */
    mb_mapping->tab_registers[0x040] = 136;
    mb_mapping->tab_registers[0x050] = 0xFE0C;
    MODBUS_SET_INT32_TO_INT16(mb_mapping->tab_registers, 0x060, u32);
    MODBUS_SET_INT64_TO_INT16(mb_mapping->tab_registers, 0x070, u64);

    /* TODO: All the values for float (64-bits) */
    // modbus_set_float_abcd(M_PI, (mb_mapping->tab_registers + 0x080));

    /* All the values for float (32-bits) */
    modbus_set_float_abcd(M_PI, (mb_mapping->tab_registers + 0x090));

    /* Raw bytes 0x0102030405060708090A*/
    mb_mapping->tab_registers[0x0A0] = 0x01;
    mb_mapping->tab_registers[0x0A1] = 0x02;
    mb_mapping->tab_registers[0x0A2] = 0x03;
    mb_mapping->tab_registers[0x0A3] = 0x04;
    mb_mapping->tab_registers[0x0A4] = 0x05;
    mb_mapping->tab_registers[0x0A5] = 0x06;
    mb_mapping->tab_registers[0x0A6] = 0x07;
    mb_mapping->tab_registers[0x0A7] = 0x08;
    mb_mapping->tab_registers[0x0A8] = 0x09;
    mb_mapping->tab_registers[0x0A9] = 0x0A;

    /* UTF-8 string: "This is a UTF-8 string!" */
    mb_mapping->tab_registers[0x0B0] = 0x54;
    mb_mapping->tab_registers[0x0B1] = 0x68;
    mb_mapping->tab_registers[0x0B2] = 0x69;
    mb_mapping->tab_registers[0x0B3] = 0x73;
    mb_mapping->tab_registers[0x0B4] = 0x20;
    mb_mapping->tab_registers[0x0B5] = 0x69;
    mb_mapping->tab_registers[0x0B6] = 0x73;
    mb_mapping->tab_registers[0x0B7] = 0x20;
    mb_mapping->tab_registers[0x0B8] = 0x61;
    mb_mapping->tab_registers[0x0B9] = 0x20;
    mb_mapping->tab_registers[0x0BA] = 0x55;
    mb_mapping->tab_registers[0x0BB] = 0x54;
    mb_mapping->tab_registers[0x0BC] = 0x46;
    mb_mapping->tab_registers[0x0BD] = 0x2d;
    mb_mapping->tab_registers[0x0BE] = 0x38;
    mb_mapping->tab_registers[0x0BF] = 0x20;
    mb_mapping->tab_registers[0x0C0] = 0x73;
    mb_mapping->tab_registers[0x0C1] = 0x74;
    mb_mapping->tab_registers[0x0C2] = 0x72;
    mb_mapping->tab_registers[0x0C3] = 0x69;
    mb_mapping->tab_registers[0x0C4] = 0x6e;
    mb_mapping->tab_registers[0x0C5] = 0x67;
    mb_mapping->tab_registers[0x0C6] = 0x21;

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return -1;
    }

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for (;;) {
        rdset = refset;
        if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                    modbus_reply(ctx, query, rc, mb_mapping);
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    return 0;
}
