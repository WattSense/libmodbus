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

static uint8_t *tab_recoil_bits = NULL;
static uint8_t *tab_input_bits = NULL;
static uint16_t *tab_registers = NULL;
static uint16_t *tab_input_registers = NULL;

static int test_read_discre_inputs(modbus_t *ctx)
{

    printf("Reading discrete (R-) input(s) (TOTAL: %i bits):  ", WS_INPUT_BITS_NB);
    if ( modbus_read_input_bits(ctx, WS_INPUT_BITS_ADDRESS, WS_INPUT_BITS_NB, tab_input_bits) > 0) {
        for (int c = 0; c < WS_INPUT_BITS_NB; c++) {
            printf("- %d", tab_input_bits[c] & 0x01 );
        }
        printf("\n");
    }
    else {
        fprintf(stderr, "Unable to read discrete inputs: %s\n", modbus_strerror(errno));
        return -1;
    }

    return 0;
}

static int test_read_coils_inputs(modbus_t *ctx)
{
    printf("Reading (RW) coil(s) (TOTAL: %i bits):  ", WS_BITS_NB);
    if ( modbus_read_bits(ctx, WS_BITS_ADDRESS, WS_BITS_NB, tab_recoil_bits) > 0) {
        for (int c = 0; c < WS_BITS_NB; c++) {
            printf("- %d", tab_recoil_bits[c] & 0x01 );
        }
        printf("\n");
    }
    else {
        fprintf(stderr, "Unable to read coils: %s\n", modbus_strerror(errno));
        return -1;
    }
  return 0;
}

static int test_read_inputs_registers(modbus_t *ctx)
{
    printf("Reading Input (R-) Registers(s) (TOTAL: %i bytes):  ", WS_INPUT_REGISTERS_NB);
    if ( modbus_read_input_registers(ctx, WS_INPUT_REGISTERS_ADDRESS, WS_INPUT_REGISTERS_NB, tab_input_registers) > 0) {
        for (int c = 0; c < WS_INPUT_REGISTERS_NB; c++) {
            printf("- 0x%04x", tab_input_registers[c]);
        }
        printf("\n");
    }
    else {
        fprintf(stderr, "Unable to read input registers: %s\n", modbus_strerror(errno));
        return -1;
    }

    return 0;
}

static int test_read_holding_registers(modbus_t *ctx)
{

    printf("Reading Holding (RW) Registers(s) (TOTAL: %i bytes):  ", WS_REGISTERS_NB);
    if ( modbus_read_registers(ctx, WS_REGISTERS_ADDRESS, WS_REGISTERS_NB, tab_registers) > 0) {
        for (int c = 0; c < WS_REGISTERS_NB; c++) {
            printf("- 0x%04x", tab_registers[c]);
        }
        printf("\n");
    }
    else {
        fprintf(stderr, "Unable to read input registers: %s\n", modbus_strerror(errno));
        return -1;
    }

    return 0;
}

static  int test_write_on_all_coils(modbus_t *ctx)
{
    printf("Turning ON all the coils... ");

    int retval = modbus_write_bits(ctx, WS_BITS_ADDRESS, WS_BITS_NB, wtest_bit);
    if (retval > 0)
    {
        printf("%d/%d bits were settled. Reading back: ", (int) retval, WS_BITS_NB);
        if (modbus_read_bits(ctx, WS_BITS_ADDRESS, WS_BITS_NB, tab_recoil_bits) > 0) {
            for (int c = 0; c < WS_BITS_NB; c++) {
                printf("- %d", tab_recoil_bits[c] & 0x01 );
            }
        }
        printf("\n");
    } else {
        fprintf(stderr, "Unable to write coils: %s\n", modbus_strerror(errno));
        return -1;
    }
    return 0;
}

static  int test_write_off_individually_coils(modbus_t *ctx)
{
    printf("Turning OFF the coils one by one... ");
    for (int c = 0; c < WS_BITS_NB; c++) {
        if (modbus_write_bit(ctx, WS_BITS_ADDRESS + c, 0x0) < 0) {
            fprintf(stderr, "Unable to write coils: %s\n", modbus_strerror(errno));
            return -1;
        }
    }
    printf("Reading back their values: ");
    if (modbus_read_bits(ctx, WS_BITS_ADDRESS, WS_BITS_NB, tab_recoil_bits) > 0) {
        for (int c = 0; c < WS_BITS_NB; c++) {
            printf("- %d", tab_recoil_bits[c] /*& 0x01*/ );
        }
    }
    printf("\n");

    return 0;
}

static  int test_write_on_individually_coils(modbus_t *ctx)
{
    printf("Turning ON the coils one by one... ");
    for (int c = 0; c < WS_BITS_NB; c++) {
        if (modbus_write_bit(ctx, WS_BITS_ADDRESS + c, 0x1) < 0) {
            fprintf(stderr, "Unable to write coils: %s\n", modbus_strerror(errno));
            return -1;
        }
    }
    printf("Reading back their values: ");
    if (modbus_read_bits(ctx, WS_BITS_ADDRESS, WS_BITS_NB, tab_recoil_bits) > 0) {
        for (int c = 0; c < WS_BITS_NB; c++) {
            printf("- %d", tab_recoil_bits[c] /*& 0x01*/ );
        }
    }
    printf("\n");
    return 0;
}

static  int test_write_individually_registers(modbus_t *ctx)
{
    uint16_t counter = 0x000F;
    printf("Writting register by register... ");
    for (int c = 0; c < WS_REGISTERS_NB; c++) {
        if (modbus_write_register(ctx, WS_REGISTERS_ADDRESS + c, counter++) < 0) {
            fprintf(stderr, "Unable to write register: %s\n", modbus_strerror(errno));
            return -1;
        }
    }
    printf("Reading back the Register(s):  ");
    if ( modbus_read_registers(ctx, WS_REGISTERS_ADDRESS, WS_REGISTERS_NB, tab_registers) > 0) {
        for (int c = 0; c < WS_REGISTERS_NB; c++) {
            printf("- 0x%04x", tab_registers[c]);
        }
        printf("\n");
    }

    return 0;
}

static  int test_write_multiples_registers(modbus_t *ctx)
{
    printf("Writting multiples registers... ");
    int retval = modbus_write_registers(ctx, WS_REGISTERS_ADDRESS, WS_REGISTERS_NB, counters);
    if (retval > 0) {
        printf("%d/%d registers were settled. Reading back: ", (int) retval, WS_REGISTERS_NB);
        if ( modbus_read_registers(ctx, WS_REGISTERS_ADDRESS, WS_REGISTERS_NB, tab_registers) > 0) {
            for (int c = 0; c < WS_REGISTERS_NB; c++) {
                printf("- 0x%04x", tab_registers[c]);
            }
            printf("\n");
        }
    }
    else {
        return -1;
    }

    return 0;
}

static void usage(void)
{
    printf(
        "Usage : modbus_tcppi_client [-v] [-l] [-h] [-a Hostname | IP address ]" \
        " [-p <TCP Port | Service name]\n");
    printf("-a\tSpecifies the IP address. It can be hostname (Default localhost)\n");
    printf("-p\tSpecifies the TCP Port (Default 502). It can be also a service name.\n");
    printf("-v\tEnable verbosity (libmodus debug).\n");
    printf("-l\tA permannt loop of reading and writting tests (for testing reconnection).\n");
    printf("-h\tDisplay this text. \n");
}


int main(int argc, char *argv[])
{
    int opt;
    char port[32];
    char address[128];
    bool _verbose = false;
    bool _loop = false;
    bool _port = false;
    bool _address = false;

    while ((opt = getopt(argc, argv, "vlha:p:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                strncpy(port, optarg, sizeof(port));
                _port = true;
                break;
            case 'a':
                strncpy(address, optarg, sizeof(address));
                _port = true;
                break;
            case 'v':
                _verbose = true;
                break;
            case 'l':
                _loop = true;
                break;
            case 'h':
                usage();
                return 0;
            default:
                printf("unknown parameter!\n");
                usage();
                return -1;
                break;
        }
    }

    if (!_port) {
      strncpy(port, DEFAULT_MODBUS_PORT_STR, sizeof(DEFAULT_MODBUS_PORT_STR));
    }

    if (!_address) {
      strncpy(address, DEFAULT_TEST_ADDRESS_STR, sizeof(DEFAULT_TEST_ADDRESS_STR));
    }

    modbus_t *ctx = NULL;
    ssize_t retval;

    ctx = modbus_new_tcp_pi(address, port);

    if (ctx == NULL) {
        fprintf(stderr, "\nTESTS FAIL!\n Unable to allocate libmodbus context\n");
        return -1;
    }

    /* TODO Under user control */
    modbus_set_debug(ctx, _verbose);

    if (_loop) {
      retval = modbus_set_error_recovery(ctx,
                                MODBUS_ERROR_RECOVERY_LINK |
                                MODBUS_ERROR_RECOVERY_PROTOCOL);
      if (retval < 0) {
            printf("Unable to set error recovery! (%s)\n", modbus_strerror(errno));
      }
    }

    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;
    retval = modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);

    if (retval > 0) {
        printf("Default timeout %d [S] . %d [uS]\n", old_response_to_sec, old_response_to_usec);
    }

    /* Allocate and initialize the memory to store the bits */
    tab_recoil_bits = (uint8_t *) malloc(WS_BITS_NB * sizeof(uint8_t));
    tab_input_bits = (uint8_t *) malloc(WS_INPUT_BITS_NB * sizeof(uint8_t));
    tab_registers = (uint16_t *) malloc(WS_REGISTERS_NB * sizeof(uint16_t));
    tab_input_registers = (uint16_t *) malloc(WS_INPUT_REGISTERS_NB * sizeof(uint16_t));

    memset(tab_recoil_bits, 0, WS_BITS_NB * sizeof(uint8_t));
    memset(tab_input_bits, 0, WS_INPUT_BITS_NB * sizeof(uint8_t));
    memset(tab_registers, 0, WS_REGISTERS_NB * sizeof(uint16_t));
    memset(tab_input_registers, 0, WS_INPUT_REGISTERS_NB * sizeof(uint16_t));

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "\nTESTS FAIL!\n Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    retval = 0;
    do {
        printf("\n**** READING TESTING ****\n");
        retval += test_read_discre_inputs(ctx);
        usleep(100);
        retval += test_read_coils_inputs(ctx);
        usleep(100);
        retval += test_read_inputs_registers(ctx);
        usleep(100);
        retval += test_read_holding_registers(ctx);

        usleep(500);

        printf("\n**** WRITTING TESTING ****\n");
        retval += test_write_on_all_coils(ctx);
        usleep(100);
        retval += test_write_off_individually_coils(ctx);
        usleep(100);
        retval += test_write_on_individually_coils(ctx);
        usleep(100);
        retval += test_write_individually_registers(ctx);
        usleep(100);
        retval += test_write_multiples_registers(ctx);

        if (_loop)
            usleep(5000000);

    } while(_loop);

    if (retval >= 0) {
        printf("\nTESTS SUCCESSFUL\n");
    }
    else {
        printf("\nTESTS FAIL!\n one or more tests failed.\n");
    }

    modbus_free(ctx);
    free(tab_recoil_bits);
    free(tab_input_bits);
    free(tab_registers);
    free(tab_input_registers);

}
