/*
 * Copyright © 2018 Raul A. Fuentes Samaniego <raul.fuentes@wattsense.com>
 *
 * ????
 */

 #include <stdio.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include <getopt.h>
 #include <stdbool.h>
 #include <errno.h>

 #ifdef _WIN32
 # include <winsock2.h>
 #else
 # include <sys/socket.h>
 #endif

 #include "modbus.h"
 #include "wsbox-tests.h"

/** Read all the variables storaged by our Server/slave mdbus node */
static void _reading_variables(modbus_mapping_t *mb_mapping)
{
    int c;

    /* NOTE: Libmous uses a full byte per bit */
    printf("Reading discrete (R-) input(s) (TOTAL: %i bits):  ", WS_INPUT_BITS_NB);
    for (c = 0; c < WS_INPUT_BITS_NB; c++) {
        printf("- %d", mb_mapping->tab_input_bits[c] & 0x01 );
    }
    printf("\n");

    printf("Reading (RW) Coil(s) (TOTAL: %i bits):  ", WS_BITS_NB);
    for (c = 0; c < WS_BITS_NB; c++) {
        printf(" - %d",  mb_mapping->tab_bits[c] & 0x01 );
    }
    printf(" -\n");

    printf("Reading Input (R-) Registers(s) (TOTAL: %i  words):\t\t", WS_INPUT_REGISTERS_NB);
    for (c = 0; c < WS_INPUT_REGISTERS_NB; c++) {
        printf("- 0x%04X", mb_mapping->tab_input_registers[c]);
    }
    printf(" -\n");

    printf("Reading Holding (RW) Registers(s) (TOTAL: %i words):\t\t", WS_REGISTERS_NB);
    for (c = 0; c < WS_REGISTERS_NB; c++) {
        printf("- 0x%04x", mb_mapping->tab_registers[c]);
    }
    printf("\n-----------------\n");

    return;
}

/** Application variables are "updated" (arbirtraly) here */
static void _updating_app_vars(modbus_mapping_t *mb_mapping)
{
    int c;

    for (c = 0; c < WS_INPUT_BITS_NB; c++) {
        if (rand() % 20 < 5) {
            mb_mapping->tab_input_bits[c] = ~mb_mapping->tab_input_bits[c] & 0x01;
        }
    }

    for (c = 0; c < WS_INPUT_REGISTERS_NB; c++) {
        mb_mapping->tab_input_registers[c] += 1;
    }

}

static void usage(void)
{
    printf(
        "Usage : modbus_tcppi_server [-v] [-h] <-p <TCP Port | Service name>\n");
    printf("-p\tSpecifies the TCP Port (Default 502). It can be also a service name.\n");
    printf("-v\tEnable verbosity (libmodus debug).\n");
    printf("-h\tDisplay this text. \n");
}

int main(int argc, char *argv[])
{

    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc = 0;
    int i = 0;
    int opt;
    uint8_t *query;
    char port[32];
    bool verbose = false;

    while ((opt = getopt(argc, argv, "vhp:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                strncpy(port, optarg, sizeof(port));
                i = 1;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                usage();
                return 0;
            default:
                break;
        }
    }

    if ((i == 0))  {
         strncpy(port, DEFAULT_MODBUS_PORT_STR, sizeof(DEFAULT_MODBUS_PORT_STR));
    }

    srand(time(NULL));

    /* NOTE: "::0" also accepts IPv4 incoming requests!  */
    ctx = modbus_new_tcp_pi("::0", port);
    query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);

    if (!ctx) {
        printf("ERROR: Unable to create modbus context!\n");
        return -1;
    }

    if (verbose) {
        modbus_set_debug(ctx, TRUE);
    }

    mb_mapping = modbus_mapping_new_start_address(
        WS_BITS_ADDRESS, WS_BITS_NB,
        WS_INPUT_BITS_ADDRESS, WS_INPUT_BITS_NB,
        WS_REGISTERS_ADDRESS, WS_REGISTERS_NB,
        WS_INPUT_REGISTERS_ADDRESS, WS_INPUT_REGISTERS_NB);

    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* TODO: Improve to use user input? */
    /* Populating the discret input variables (1-bit)  */
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, WS_INPUT_BITS_NB,
                               WS_INPUT_BITS_TAB);
    for (i = 0; i < WS_INPUT_REGISTERS_NB; i++) {
        mb_mapping->tab_input_registers[i] = WS_INPUT_REGISTERS_TAB[i];
    }
    for (i = 0; i < WS_REGISTERS_NB; i++) {
        mb_mapping->tab_registers[i] = WS_REGISTERS_TAB[i];
    }

    _reading_variables(mb_mapping);

    /* Settling the socket and listening mode for our server/slave */
    s = modbus_tcp_pi_listen(ctx, 1);
    modbus_tcp_pi_accept(ctx, &s);


    for (;;) {
        do {
            rc = modbus_receive(ctx, query);
            /* Filtered queries return 0 */
        } while (rc == 0);

        /* The connection is not closed on errors which require on reply such as
           bad CRC in RTU. */
        if (rc == -1 && errno != EMBBADCRC) {
            /* Quit */
            printf("Leaving...\n");
            break;
        }

        rc = modbus_reply(ctx, query, rc, mb_mapping);
        if (rc == -1) {
            break;
        }
        else if (rc > 0) {
            _updating_app_vars(mb_mapping);
        }
    }

    printf("Final state: \n");
    _reading_variables(mb_mapping);

    modbus_mapping_free(mb_mapping);
    free(query);
    modbus_free(ctx);

    return 0;

}
