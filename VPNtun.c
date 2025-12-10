// Kevin Johanson
// Thread Safe Packet queue for VPN 
// FIFO queue logic

#include <stdio.h>
#include <pthread.h>

// structure for queue
typedef struct packetQueue {
    packet_t **packets; // array of pointers to packets
    int capacity; // maximumum queue size
    int size; // current ammount of packets
    int head;
    int tail;
    pthread_mutex_t mutex; // thread for accessing queue
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
    // sem_t semaphore; not sure about this 
    int queueID; // id for the current queue
} packetQueue_t;

//initalize multi thread packet queue
int queueInit(packetQueue_t **queue, int capacity, int queueID) {

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