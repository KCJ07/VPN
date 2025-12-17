// Kevin Johanson
// VPN client/server - doesn't hide IP
// 4-thread architecture:
// Thread 1: TUN Reader - reads from TUN interface
// Thread 2: Internet Sender - sends to internet
// Thread 3: Internet Reader - reads from internet  
// Thread 4: TUN Sender - writes to TUN interface
//
//  TUN -> Queue -> Internet -> Queue -> TUN


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <netinet/ip.h>

#include "VPNencryption.c"
#include "VPNpacketqueue.c"
#include "VPNtun.c"



#define TUN_DEVICE "tun0"
#define TUN_IP "10.8.0.1" // 10.8.0.x: Private VPN network range
#define TUN_NETMASK "255.255.255.0"
#define TUN_MTU 1500 // how big packets can be / maximumum transition unit
#define QUEUE_CAPACITY 100

// VPN info
typedef struct VPNInfo {
    int socketfd;
    int tunfd;

    //4 threads architure
    pthread_t tunReaderThread;
    pthread_t webSenderThread;
    pthread_t webReaderThread;
    pthread_t tunSenderThread;

    // packet queues
    packetQueue_t *tunToInternetQueue;    // TUN -> Internet
    packetQueue_t *internetToTunQueue;    // Internet -> TUN
    
    // encryption info (not needed/placeholder)
    cryptoInfo_t encryptInfo;

    bool running;

} VPNInfo_t;


//thread 1
//Read packets from TUN device
void* tunReader(void *arg) {
    VPNInfo_t* vpn = (VPNInfo_t *)arg;


    // main loop for worker
    while (vpn->running) {
        packet_t* packet = malloc(sizeof(packet_t));

        //start the actuall reading from TUN
        int len = readTUN(vpn->tunfd, packet->data, PACKET_MAX_SIZE);
        if (len > 0) {
            packet->length = len;

            //what do I do with the packet IP?

            //push packet to queue
            push(vpn->tunToInternetQueue, &packet);
        } else {
            free(packet);
        }

    }

    return NULL;
}


//thread 2
//sends/writes packets to the TUN
void* tunSender(void *arg) {
    VPNInfo_t* vpn = (VPNInfo_t *)arg;

    //main worker loop
    while (vpn->running) {
        packet_t* packet = NULL;
        
        if (pop(vpn->internetToTunQueue, &packet) == 0) {

            //write the popped packet to TUN 
            writeTUN(vpn->tunfd, packet->data, packet->length);
            free(packet);
        }
    }
    return NULL;
}

//thread 3
//sends packets from the web
void* webSender(void *arg) {
    VPNInfo_t* vpn = (VPNInfo_t *)arg;

    while (vpn->running) {
        packet_t* packet = NULL;

        if (pop(vpn->tunToInternetQueue, &packet) == 0 && packet) {

            //get ip 
            struct iphdr *iph = (struct iphdr *)packet->data; // cast ipheader struct to our packet data

            // set up destination config in socketaddress??
            struct sockaddr_in destAddr;
            memset(&destAddr, 0, sizeof(destAddr));
            destAddr.sin_family = AF_INET;
            destAddr.sin_addr.s_addr = iph->daddr;

            char destIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(destAddr.sin_addr), destIP, INET_ADDRSTRLEN);
           
            //use send and fd to send it to the interweb
            sendto(vpn->socketfd, packet->data, packet->length, 0, (struct sockaddr*) &destAddr, sizeof(destAddr));
            free(packet);
        }

    }
    return NULL;
}

//thread 4
void* webReader(void *arg) {
    VPNInfo_t* vpn = (VPNInfo_t *)arg;

    while (vpn->running) {
        packet_t* packet = malloc(sizeof(packet_t));

        // recieve from raw socket
        struct sockaddr_in srcAddr;
        socklen_t addrLen = sizeof(srcAddr);

        int len = recvfrom(vpn->socketfd, packet->data, PACKET_MAX_SIZE, 0, (struct sockaddr *)&srcAddr, &addrLen);
        
        if (len > 0) {
            packet->length = len;
  
            // push the packet recieved to the internet to tun queue
            push(vpn->internetToTunQueue, &packet);

        } else {
            free(packet);
        }

        
    }
    
    return NULL;
}

//init
int vpnInit(VPNInfo_t *vpn) {
    
    memset(vpn, 0, sizeof(VPNInfo_t));
    vpn->running = false;

    //creates TUN interface
    char tunName[16] = TUN_DEVICE;
    vpn->tunfd = createTUN(tunName);
    if (vpn->tunfd < 0) {
        perror("TUN interface creation error");
        return -1;
    }

    //TUN configuration
    if (TUNconfig(tunName, TUN_IP, TUN_NETMASK, TUN_MTU) < 0) {
        perror("Failed to configure TUN");;
        return -1;
    }

    //create "raw" socket for web access using complete ip packets "IPPROTO_RAW"
    vpn->socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (vpn->socketfd < 0 ) {
        perror("could not create socket");
        return -1;
    }

    // Enable IP_HDRINCL so we can send raw IP packets ???
    int one = 1;
    setsockopt(vpn->socketfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    //encryptiion initialization
    uint8_t key[AES_KEY_SIZE];
    makeKey(key, AES_KEY_SIZE);
    if (InitEncryption(&vpn->encryptInfo, key) < 0) {
        perror("initializing encryption Error");
        return -1;
    }

    // Initialize queues
    queueInit(&vpn->tunToInternetQueue, QUEUE_CAPACITY, 1);
    queueInit(&vpn->internetToTunQueue, QUEUE_CAPACITY, 2);
    // TODO: Check failure

    printf("Client VPN initialized");
    return 0;
}

//start
int vpnStart(VPNInfo_t *vpn) {
    
    vpn->running = true;

    // start 4 worker threads 
    pthread_create(&vpn->tunReaderThread, NULL, tunReader, vpn);
    pthread_create(&vpn->tunSenderThread, NULL, tunSender, vpn);
    pthread_create(&vpn->webSenderThread, NULL, webSender, vpn);
    pthread_create(&vpn->webReaderThread, NULL, webReader, vpn);
    //TODO: add error checking

    printf("All threads started");
    return 0;

}

//stop all threads by joining them
int vpnStop(VPNInfo_t *vpn) {
    
    vpn->running = false;

    // join all 4 worker threads 
    pthread_join(vpn->tunReaderThread, NULL);
    pthread_join(vpn->tunSenderThread, NULL);
    pthread_join(vpn->webSenderThread, NULL);
    pthread_join(vpn->webReaderThread, NULL);
    //TODO: add error checking

    printf("All threads stopped");
    return 0;

}

//cleans up sockets, threads, and encryption info
int vpnCleanup(VPNInfo_t *vpn) {

    // clean queues
    if (vpn->tunToInternetQueue) {
        queueClear(vpn->tunToInternetQueue);
    }

    if (vpn->internetToTunQueue) { 
        queueClear(vpn->internetToTunQueue);
    }
    // clean sockets
    if (vpn->socketfd > 0) {
        close(vpn->socketfd);
    } 
    
    if (vpn->tunfd > 0) {
        close(vpn->tunfd);
    }

    // clean encryption info
    encryptCleanUP(&vpn->encryptInfo);

    return 0;
}