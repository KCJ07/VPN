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
    // what most encryption algorithms like AES use as a key
    uint8_t key[AES_KEY_SIZE]; // shared key so we can encrypt and decrpyt packets
    int init;
} cryptoInfo_t;



// initializes encryption info
int InitEncryption(cryptoInfo_t *crypto, uint8_t *key) {
    
    memcpy(crypto->key, key, AES_KEY_SIZE);
    crypto->init = 1;

    return 0;

}

// encrypts a packet
// does nothing at the moment
int encryptPacket(cryptoInfo_t *crypto, packet_t *packet) {

    return 0;

}

// decrypt a packet
// does nothing at the moment
int decryptPacket(cryptoInfo_t *crypto, packet_t *packet) {
    return 0;
}

//generates pre determined key (TODO: random) 
int makeKey(cryptoInfo_t *crypto) {

    // what most encryption algorithms like AES use as a key
    uint8_t key[32] = {
    1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32 };

    crypto->key = key;

    return 0;

}

// cleans cryptoInfo
void cleanUP(cryptoInfo_t *crypto) {
    memset(crypto->key, 0, AES_KEY_SIZE); 
    crypto->init = 0;
}