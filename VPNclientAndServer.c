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

#include "VPNtun.c"
#include "VPNpacketqueue.c"
#include "VPNencryption.c"

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
    //cryptoInfo_t encryptInfo;

    boool running;

} VPNInfo_t;


//thread 1

//thread 2

//thread 3

//thread 4

//init

//start

//stop 

//cleanup