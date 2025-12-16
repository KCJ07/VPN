// client file for VPN
// incorporates individual threading per client
// 6 total unique threads:
// Server Reader - Reads packets from server
// Decrypter - decrypts packets
// Encrypter -  encrypts packets
// TUN Reader - reads packets from TUN
// TUN Sender - send packets to TUN
// Server Sender - sends packets to server

// Architecture:
// (outgoing) reads from TUN -> encrypt packets -> forwards to server 
// (incoming) reads from web -> encrypts data -> sends to client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "VPNtun.c"
#include "VPNpacketqueue.c"
#include "VPNencryption.c"

// VPN info
typedef struct VPNinfo {
    int socketfd;
    int tunfd;
    struct sockaddr_in serverAddr;

    //6 threads architure
    pthread_t TUNReaderThread;
    pthread_t encryptThread;
    pthread_t ServerSenderThread;
    pthread_t ServerReaderThread;
    pthread_t decryptThread;
    pthread_t TUNSenderThread;

    // packet queues
    packetQueue_t *tunToEncryptQueue;    // TUN -> Encrypt
    packetQueue_t *encryptToSevercQueue; // Encrypt -> Socket
    packetQueue_t *serverToDecryptQueue; // Socket -> Decrypt
    packetQueue_t *decryptToTunQueue;    // Decrypt -> TUN
    
    // encryption info
    cryptoInfo_t encryptInfo;

    boool running;
    pthread_mutex_t state;

} VPNinfo_t;



//thread 1
void *TUNreader(void *args) {

}

//thread 2
void encrypter(void *args) {

}

//thread 3
void decrypter(void *args) {

}

//thread 4
void TUNwriter(void *args) {

}

//thread 5
void serverSender(void *args) {

}

//thread 6
void serverReader(void *args) {

}

//init client

//start client

//stop client

//cleanup client