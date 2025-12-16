// Kevin Johanson
// VPN project encryption
// currently DOES NO ENCRYPTION. grabs packets then sends them through

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PACCKET_MAX_SIZE 2048

//packet struct
typdef struct packet {
    uint8_t data[PACKET_MAX_SIZE];
    int length;
    struct sockaddr_in addr;
} packet_t;

//encryption info
typedef struct cryptInfo {
    uint8_t key[AES_KEY_SIZE]; // shared key so we can encrypt and decrpyt packets
    int initialized;
} cryptoInfo_t;



// initializes encryption info
int InitEncryption(cryptoInfo_t *crypto) {

}

//encrypts a packet
int encryptPacket(cryptoInfo_t *crypto) {

}

//decrypt a packet
int decryptPacket(cryptoInfo_t *crypto) {

}

int makeKey(cryptoInfo_t *crypto) {

}

// cleans pointers and context
void cleanUP(cryptoInfo_t *crypto) {

}