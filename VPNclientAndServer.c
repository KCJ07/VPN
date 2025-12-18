// Kevin Johanson
// VPN client/server - doesn't hide IP
// 4-thread architecture:
// Thread 1: TUN Reader - reads from TUN interface
// Thread 2: Internet Sender - sends to internet
// Thread 3: Internet Reader - reads from internet  
// Thread 4: TUN Sender - writes to TUN interface
//
//  TUN -> Queue -> Internet -> Queue -> TUN


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
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


// packet counter structure
typedef struct packetTest {
    uint64_t tunRead;
    uint64_t tunSent;
    uint64_t webSent;
    uint64_t webRecv;
    pthread_mutex_t lock;
} PacketTest_t;

PacketTest_t packetTest;

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


// Initialize test tracker
void testInit() {
    packetTest.tunRead = 0;
    packetTest.tunSent = 0;
    packetTest.webSent = 0;
    packetTest.webRecv = 0;
    pthread_mutex_init(&packetTest.lock, NULL);
}

// Cleanup
void testCleanup() {
    pthread_mutex_destroy(&packetTest.lock);
}

// Increment counter (thread-safe)
void testCount(uint64_t *counter) {
    pthread_mutex_lock(&packetTest.lock);
    (*counter)++;
    pthread_mutex_unlock(&packetTest.lock);
}

// Print results
void testPrint() {
    pthread_mutex_lock(&packetTest.lock);
    printf("\n=== PACKET TEST ===\n");
    printf("TUN Read:     %lu\n", packetTest.tunRead);
    printf("TUN Sent:     %lu\n", packetTest.tunSent);
    printf("Web Sent:     %lu\n", packetTest.webSent);
    printf("Web Received: %lu\n", packetTest.webRecv);
    printf("===================\n\n");
    pthread_mutex_unlock(&packetTest.lock);
}


//thread 1
//Read packets from TUN device
void* tunReader(void *arg) {
    VPNInfo_t* vpn = (VPNInfo_t *)arg;


    // main loop for worker
    while (vpn->running) {
        packet_t* packet = malloc(sizeof(packet_t));
        if (!packet) {
            sleep(1);
            continue;
        }

        //start the actuall reading from TUN
        int len = readTUN(vpn->tunfd, packet->data, PACKET_MAX_SIZE);
        if (len > 0) {
            packet->length = len;
            // count the packet for testing
            testCount(&packetTest.tunRead);

            //what do I do with the packet IP?

            //push packet to queue
            push(vpn->tunToInternetQueue, &packet);
        } else {
            free(packet);
            usleep(10000); // Sleep 10ms to avoid busy loop
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
            testCount(&packetTest.tunSent); // test counter
            free(packet);
        } else {
             usleep(10000); // Sleep 10ms if no data
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
            testCount(&packetTest.webSent); // test counter
            free(packet);
        } else{
            usleep(10000); // sleep 10ms for no data
        }

    }
    return NULL;
}

//thread 4
void* webReader(void *arg) {
    VPNInfo_t* vpn = (VPNInfo_t *)arg;

    // make non-blicking. should fix not sending any packets from tun or recieving any from web
    int flags = fcntl(vpn->socketfd, F_GETFL, 0);
    fcntl(vpn->socketfd, F_SETFL, flags | O_NONBLOCK);


    while (vpn->running) {
        packet_t* packet = malloc(sizeof(packet_t));
        if (!packet) {
            usleep(10000);
            continue;
        }


        // recieve from raw socket
        struct sockaddr_in srcAddr;
        socklen_t addrLen = sizeof(srcAddr);

        int len = recvfrom(vpn->socketfd, packet->data, PACKET_MAX_SIZE, 0, (struct sockaddr *)&srcAddr, &addrLen);
        
        if (len > 0) {
            packet->length = len;
            testCount(&packetTest.webRecv);
            // push the packet recieved to the internet to tun queue
            push(vpn->internetToTunQueue, &packet);

        } else {
            free(packet);
            usleep(10000); // sleep 10ms for no data 
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

    // Set TUN to non-blocking WHAT DOES THIS MEAN/DO
    int flags = fcntl(vpn->tunfd, F_GETFL, 0);
    fcntl(vpn->tunfd, F_SETFL, flags | O_NONBLOCK);

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


    //Enable promiscuous mode to receive all packets (attempt to fix not sending tun packets)
    setsockopt(vpn->socketfd, IPPROTO_IP, IP_RECVERR, &one, sizeof(one));

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


// Test results tracking
typedef struct {
    int passed;
    int failed;
} TestResults;

TestResults results = {0, 0};

#define TEST_PASS(name) do { \
    printf("✓ PASS: %s\n", name); \
    results.passed++; \
} while(0)

#define TEST_FAIL(name, reason) do { \
    printf("✗ FAIL: %s - %s\n", name, reason); \
    results.failed++; \
} while(0)

// Global VPN for signal handler
VPNInfo_t global_vpn;

void signal_handler(int sig) {
    printf("\nReceived signal %d, stopping VPN...\n", sig);
    global_vpn.running = false;
}

// Test 1: Encryption module
void test_encryption() {
    printf("\n=== Testing Encryption Module ===\n");
    
    cryptoInfo_t crypto;
    uint8_t key[AES_KEY_SIZE];
    
    // Test key generation
    if (makeKey(key, AES_KEY_SIZE) == 0) {
        TEST_PASS("Key generation");
    } else {
        TEST_FAIL("Key generation", "makeKey failed");
        return;
    }
    
    // Test crypto init
    if (InitEncryption(&crypto, key) == 0) {
        TEST_PASS("Encryption initialization");
    } else {
        TEST_FAIL("Encryption initialization", "InitEncryption failed");
        return;
    }
    
    // Test encrypt/decrypt (should do nothing but return success)
    packet_t testPacket;
    memset(&testPacket, 0, sizeof(testPacket));
    strcpy((char*)testPacket.data, "Test data");
    testPacket.length = 10;
    
    if (encryptPacket(&crypto, &testPacket) == 0) {
        TEST_PASS("Packet encryption (placeholder)");
    } else {
        TEST_FAIL("Packet encryption", "encryptPacket failed");
    }
    
    if (decryptPacket(&crypto, &testPacket) == 0) {
        TEST_PASS("Packet decryption (placeholder)");
    } else {
        TEST_FAIL("Packet decryption", "decryptPacket failed");
    }
    
    // Test cleanup
    encryptCleanUP(&crypto);
    TEST_PASS("Encryption cleanup");
}

// Test 2: Queue operations
void test_queue() {
    printf("\n=== Testing Queue Module ===\n");
    
    packetQueue_t *queue = NULL;
    
    // Test queue init
    if (queueInit(&queue, 10, 1) == 0) {
        TEST_PASS("Queue initialization");
    } else {
        TEST_FAIL("Queue initialization", "queueInit failed");
        return;
    }
    
    // Test push operation
    packet_t *packet1 = malloc(sizeof(packet_t));
    memset(packet1, 0, sizeof(packet_t));
    strcpy((char*)packet1->data, "Packet 1");
    packet1->length = 9;
    
    if (push(queue, &packet1) == 0) {
        TEST_PASS("Queue push");
    } else {
        TEST_FAIL("Queue push", "push failed");
        free(packet1);
        queueClear(queue);
        return;
    }
    
    // Test pop operation
    packet_t *packet2 = NULL;
    if (pop(queue, &packet2) == 0 && packet2 != NULL) {
        TEST_PASS("Queue pop");
        if (strcmp((char*)packet2->data, "Packet 1") == 0) {
            TEST_PASS("Queue data integrity");
        } else {
            TEST_FAIL("Queue data integrity", "Data mismatch");
        }
        free(packet2);
    } else {
        TEST_FAIL("Queue pop", "pop failed or returned NULL");
    }
    
    // Test cleanup
    queueClear(queue);
    TEST_PASS("Queue cleanup");
}

// Test 3: TUN interface (requires root)
void test_tun() {
    printf("\n=== Testing TUN Module ===\n");
    
    if (getuid() != 0) {
        printf("⚠ SKIP: TUN tests require root privileges\n");
        return;
    }
    
    char tunName[IFNAMSIZ];
    strncpy(tunName, "tun99", IFNAMSIZ);  // Use different name to avoid conflicts
    
    // Test TUN creation
    int tunfd = createTUN(tunName);
    if (tunfd >= 0) {
        TEST_PASS("TUN device creation");
        
        // Test TUN configuration
        if (TUNconfig(tunName, "10.99.0.1", "255.255.255.0", 1500) == 0) {
            TEST_PASS("TUN device configuration");
        } else {
            TEST_FAIL("TUN device configuration", "TUNconfig failed");
        }
        
        close(tunfd);
    } else {
        TEST_FAIL("TUN device creation", "createTUN failed");
    }
}

// Test 4: Full VPN initialization
void test_vpn_init() {
    printf("\n=== Testing VPN Initialization ===\n");
    
    if (getuid() != 0) {
        printf("⚠ SKIP: VPN init requires root privileges\n");
        return;
    }
    
    VPNInfo_t vpn;
    
    if (vpnInit(&vpn) == 0) {
        TEST_PASS("VPN initialization");
        
        // Check that all components were initialized
        if (vpn.tunfd > 0) {
            TEST_PASS("TUN file descriptor created");
        } else {
            TEST_FAIL("TUN file descriptor", "Invalid fd");
        }
        
        if (vpn.socketfd > 0) {
            TEST_PASS("Raw socket created");
        } else {
            TEST_FAIL("Raw socket", "Invalid fd");
        }
        
        if (vpn.tunToInternetQueue != NULL) {
            TEST_PASS("TUN->Internet queue created");
        } else {
            TEST_FAIL("TUN->Internet queue", "NULL pointer");
        }
        
        if (vpn.internetToTunQueue != NULL) {
            TEST_PASS("Internet->TUN queue created");
        } else {
            TEST_FAIL("Internet->TUN queue", "NULL pointer");
        }
        
        // Cleanup
        vpnCleanup(&vpn);
        TEST_PASS("VPN cleanup");
        
    } else {
        TEST_FAIL("VPN initialization", "vpnInit failed");
    }
}

// Test 5: Thread creation (quick test)
void test_threads() {
    printf("\n=== Testing Thread Creation ===\n");
    
    if (getuid() != 0) {
        printf("⚠ SKIP: Thread test requires root privileges\n");
        return;
    }
    
    VPNInfo_t vpn;
    
    if (vpnInit(&vpn) != 0) {
        TEST_FAIL("Thread test", "vpnInit failed");
        return;
    }
    
    if (vpnStart(&vpn) == 0) {
        TEST_PASS("Thread creation");
        
        printf("   Threads running for 2 seconds...\n");
        sleep(2);
        
        if (vpnStop(&vpn) == 0) {
            TEST_PASS("Thread joining");
        } else {
            TEST_FAIL("Thread joining", "vpnStop failed");
        }
    } else {
        TEST_FAIL("Thread creation", "vpnStart failed");
    }
    
    vpnCleanup(&vpn);
}

// Main test runner
int main(int argc, char *argv[]) {
    // (void)argc;
    // (void)argv;
    
    // printf("\n");
    // printf("╔════════════════════════════════════════╗\n");
    // printf("║     VPN Component Test Suite          ║\n");
    // printf("╚════════════════════════════════════════╝\n");
    
    // // Check if running as root
    // if (getuid() != 0) {
    //     printf("\n⚠  WARNING: Not running as root!\n");
    //     printf("   Some tests will be skipped.\n");
    //     printf("   Run with: sudo ./VPN\n");
    // }
    
    // // Run all tests
    // test_encryption();
    // test_queue();
    // test_tun();
    // test_vpn_init();
    // //test_threads();
    
    // // Print summary
    // printf("\n");
    // printf("╔════════════════════════════════════════╗\n");
    // printf("║          Test Summary                  ║\n");
    // printf("╠════════════════════════════════════════╣\n");
    // printf("║  Passed: %-4d                          ║\n", results.passed);
    // printf("║  Failed: %-4d                          ║\n", results.failed);
    // printf("╚════════════════════════════════════════╝\n");
    
    // if (results.failed == 0) {
    //     printf("\n✓ All tests passed! VPN is ready to run.\n");
        
    //     if (getuid() == 0) {
    //         printf("\n┌─────────────────────────────────────┐\n");
    //         printf("│  Start full VPN? (yes/no): ");
    //         char response[10];
    //         if (scanf("%9s", response) == 1 && strcmp(response, "yes") == 0) {
    //             printf("└─────────────────────────────────────┘\n");
    //             printf("\n=== Starting Full VPN ===\n\n");
                
    //             signal(SIGINT, signal_handler);
    //             signal(SIGTERM, signal_handler);
                
    //             if (vpnInit(&global_vpn) < 0) {
    //                 fprintf(stderr, "Failed to initialize VPN\n");
    //                 return 1;
    //             }
                
    //             if (vpnStart(&global_vpn) < 0) {
    //                 fprintf(stderr, "Failed to start VPN\n");
    //                 vpnCleanup(&global_vpn);
    //                 return 1;
    //             }
                
    //             printf("VPN running. Press Ctrl+C to stop.\n\n");
                
    //             while (global_vpn.running) {
    //                 sleep(1);
    //             }
                
    //             vpnStop(&global_vpn);
    //             vpnCleanup(&global_vpn);
                
    //             printf("\n=== VPN Stopped ===\n");
    //         } else {
    //             printf("└─────────────────────────────────────┘\n");
    //             printf("\nTests complete. Exiting.\n");
    //         }
    //     }
        
    //     return 0;
    // } else {
    //     printf("\n✗ Some tests failed. Fix errors before running VPN.\n");
    //     return 1;
    // }




    vpnInit(&global_vpn);
    testInit();
    vpnStart(&global_vpn);

    system("ip route add 8.8.8.8/32 dev tun0");  // Route Google DNS through VPN
    system("ip route add 1.1.1.1/32 dev tun0");  // Route Cloudflare DNS through VPN
    
    printf("VPN running. Press Ctrl+C to stop.\n\n");
    
    while (global_vpn.running) {
        sleep(2);  // Print every 5 seconds
        testPrint();
    }
    
    vpnStop(&global_vpn);
    
    printf("\n=== FINAL TEST RESULTS ===\n");
    testPrint();
    testCleanup();
    
    vpnCleanup(&global_vpn);
}