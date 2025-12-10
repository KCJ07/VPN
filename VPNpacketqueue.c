// Kevin Johanson
// Thread Safe Packet queue for VPN 
// FIFO queue logic

#include <stdio.h>
#include <pthread.h>

// structure for queue
typedef struct packetQueue {
    packet_t **packets; // array of pointers to packets
    int maxCapacity; // maximumum queue size
    int curSize; // current ammount of packets
    int head; // head index
    int tail; // tail index
    pthread_mutex_t mutex; // thread for accessing queue
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
    // sem_t semaphore; not sure about this 
    int queueID; // id for the current queue
} packetQueue_t;

//initalize multi thread packet queue
int queueInit(packetQueue_t **queue, int capacity, int queueID) {

    // malloc space for struct
    packetQueue_t *queueSpace= malloc(sizeof(packetQueue_t))
    if (!queueSpace) { // checks if malloc failed
        perror("Failed to Malloc"); 
        return;
    }

    //malloc space for packet pointer array
    queueSpace->packets = malloc(sizeof (packet_t *) * capacity);
    if (!queueSpace->packets) { // checks if malloc failed
        perror("Failed to Malloc"); 
        return;
    }

    // sets array of pointers to packets to all 0 
    memset(q->packets, 0, sizeof(packet_t *) * capacity);

    //initialze rest of struct
    queueSpace->maxCapacity = capacity;
    queueSpace->size = 0;
    queueSpace->head = 0;
    queueSpace->tail = 0;
    queueSpace->queueID = queueID; 

    pthread_mutex_init(&queueSpace->mutex, NULL);     // Mutex for exclusive queue access

    // cond_init are: A condition (short for "condition variable") is a synchronization
    // device that allows threads to suspend execution and relinquish the
    // processors until some predicate on shared data is satisfied.
    pthread_cond_init(&queueSpace->notEmpty, NULL);  // variable for when queue has data
    pthread_cond_init(&queueSpace->notFull, NULL);   // variable for when queue has space

    *queue = queueSpace;
    return 0;
}

// push for our queue
int push(packetQueue_t *queue, packet_t **packet) {
    
}


// pop for our queue
int pop(packetQueue_t *queue, packet_t **packet) {

}

// implements non blocking thread communication for pop function
// regualr Pop blocks forever if queue is emtpy
int tryPop(packetQueue_t *queue, packet_t **packet, int timeout) {

}

// clears the queue
int queueClear(packetQueue_t *queue) {

}