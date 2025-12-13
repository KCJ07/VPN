// Server file for VPN
// incorporates individual threading per client
// 6 total unique threads:
// Client Reader - Reads packets from client
// Decrypter - decrypts packets
// Encrypter -  encrypts packets
// Web Reader - reads packets from web
// Web Sender - sends packets to web
// Client Sender - sends packets to client

// Architecture:
// (outgoing) reads from client -> decrpyts packets -> forwards to web 
// (incoming) reads from web -> encrypts data -> sends to client

#include "VPNtun.c"
#include "queue.c"


// client structure containing all info on each client
typedef struct client{
    bool connected;
    pthread_t clientThread;
    sockaddr_in_t clientAddr;
    vpn_ctx_t VPN; 
}client_t;

// list of clients struct
typedef struct {
    client_ctx_t **clients;
    int maxClients;
    int numClients;
    pthread_mutex_t clientsMutex;
} server_t;

int initThreads(vpn_ctx_t *ctx) {
    if (!ctx) return -1;
    
    printf("Initializing 6-thread system...\n");
    
    // TODO: Initialize the 6 threads and 4 queues here
    // This would create:
    // 1. Client Reader Thread
    // 2. Decrypt Worker Thread  
    // 3. Internet Sender Thread
    // 4. Internet Reader Thread
    // 5. Encrypt Worker Thread
    // 6. Client Sender Thread
    
    return 0;
}

void cleanThreads(vpn_ctx_t *ctx) {
    if (!ctx) return;
    
    printf("Cleaning up 6-thread system...\n");
    
    // TODO: Stop all 6 threads and clean up 4 queues
}

// starts a client with its 6 threads
// parameters are client info struct
void *startClient(void *arg) {
    client_t *client = (client_t *) arg;

    printf("New Client Connected");

    initThreads(&client->VPN);

    // keeps client thread alive
    while (client->connected && serverOn) {
        sleep(1);
    }

    // after client is no longer alive run these
    cleanThreads(&client->VPN);

    client->connected = false;
    client->VPN.connected = false;
}

int main(int argc, char *argv[]) {

    server_ctx_t server;
    int sockfd;

    memset(&server, 0, sizeof(server));
    server.maxClients = 10;
    server.clients = malloc(sizeof(client_ctx_t *) * server.maxClients);
    pthread_mutex_init(&server.clientsMutex, NULL);

    // creating socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(5000);

    bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    printf("server listening on UDP port 5000");


    // loop and accept clients
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        char buffer[1024];


        int len = recvfrom(sockfd, bugger, sizeof(buffer), 0, (struct sockaddr *)&clientADdr, &addrLen);
        
        if (len > 0) {
            //create new client info
            client_t *client = malloc(sizeof(client_t));
            memset(client, 0, sizeof(client_t));
            memcpy(&client->clientAddr, &clientAddr, sizeof(clientAddr));
            client->connected = true;

            // start thread with our worker thread for each individual client
            pthread_create(&client->clientThread, NULL, startClient, client);
    
        }


    }

    return 0;
}
