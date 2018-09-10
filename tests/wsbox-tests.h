#ifndef _WSBOX_TEST_H_
#define _WSBOX_TEST_H_

#define DEFAULT_MODBUS_PORT_STR "502"
#define DEFAULT_TEST_ADDRESS_STR "localhost"

/* Setting coils, inputs and registers sizes and starting address  */
const uint16_t WS_BITS_ADDRESS = 0x130;
const uint16_t WS_BITS_NB = 0x05; /* Default 0x25 */

const uint16_t WS_INPUT_BITS_ADDRESS = 0x01C4;
const uint16_t WS_INPUT_BITS_NB = 0x06;

const uint16_t WS_REGISTERS_ADDRESS = 0x160;
const uint16_t WS_REGISTERS_NB = 0x3;

const uint16_t WS_INPUT_REGISTERS_ADDRESS = 0x108;
const uint16_t WS_INPUT_REGISTERS_NB = 0x1;

/* Starting values for the inputs and registers (Server side) */
const uint8_t WS_BITS_TAB[] = { 0xCD };
const uint8_t WS_INPUT_BITS_TAB[] = { 0xFF}; /* NOTE: Bits not bytes */
const uint16_t WS_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };
const uint16_t WS_INPUT_REGISTERS_TAB[] = { 0x0000 };


/* Writting values (Client side) */
const uint8_t wtest_bit[] = {1, 1, 1, 1, 1}; /* test_write_on_all_coils */
const uint16_t counters[] = { 0xF22B, 0xF001, 0xF064 };
#endif /* _WSBOX_TEST_H_ */
