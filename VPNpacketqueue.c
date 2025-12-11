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
    pthread_mutex_t mutex; // thread queue 
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
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

    pthread_mutex_init(&queueSpace->mutex, NULL);     // Mutex for queue

    // cond_init are: A condition (short for "condition variable") is a synchronization
    // device that allows threads to suspend execution and relinquish the
    // processors until some predicate on shared data is satisfied.
    pthread_cond_init(&queueSpace->notEmpty, NULL);  // variable for when queue has data
    pthread_cond_init(&queueSpace->notFull, NULL);   // variable for when queue has space

    *queue = queueSpace;
    return 0;
}

// pushes packet to the end of our queue
int push(packetQueue_t *queue, packet_t **packet) {

    //locks the queue while pushing to the thread
    pthread_mutex_lock(queue);

    //check if queue is full then wait if it is (gets rid of lock while waiting)
    while (queue->size == queue->maxCapacity) {
        //waits untill condition is met
        pthread_cond_wait(&queue->notFull, &queue->mutex);
        
    }

    //add packet to end of queue
    queue->packets[queue->tail] = packet;

    //update tail (circular queue)
    queue->tail = (queue->tail + 1) % queue->capacity;

    //update queueSize
    queue->curSize = queue->curSize +1
    

    pthread_cond_signal(&queue->notEmpty);

    pthread_mutex_unlock(queue);

    return 0;
}


// pops the head of our queue
// if queue is empty return ERROR (UP FOR DEBATE)
// need to implement a timeout
int pop(packetQueue_t *queue, packet_t **packet) {

    //locks the queue while popping from the thread
    pthread_mutex_lock(queue);

    //checks to make sure queue is not empty 
    // while it is wait till its not (this gets rid of lock while waiting)
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex); 
    }
    


    // grab popped packet 
    *packet = queue->packets[queue->head];
    queue->packets[queue->head] = NULL;

    // update queue indicies
    queue->head = (queue->head + 1) % queue->capacity;
    queue->tail = queue->tail - 1;

    pthread_cond_signal(&queue->notFull);
    pthread_mutex_unlock(queue);


    return 0;
}


// clears the queue
int queueClear(packetQueue_t *queue) {

    // frees thread variables
    pthread_mutex_destroy(&queue->mutex);   
    pthread_cond_destroy(&queue->notEmpty);  
    pthread_cond_destroy(&queue->notFull);  

    free(queue->packets);
    free(queue);
}