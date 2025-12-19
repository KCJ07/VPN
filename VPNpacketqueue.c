// Kevin Johanson
// Thread Safe Packet queue for VPN 
// FIFO queue logic

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

// structure for queue
typedef struct packetQueue {
    packet_t **packets; // array of pointers to packets
    int maxCapacity; // maximumum queue size
    int curSize; // current ammount of packets in queue
    int head; 
    int tail; 
    pthread_mutex_t mutex; 
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
    int queueID; 
} packetQueue_t;

//initalize multi thread packet queue
int queueInit(packetQueue_t **queue, int capacity, int queueID) {

    // malloc space for struct
    packetQueue_t *q = malloc(sizeof(packetQueue_t));
    if (!q) { // checks if malloc failed
        perror("Failed to Malloc"); 
        return 1;
    }

    //malloc space for packet pointer array
    q->packets = malloc(sizeof (packet_t *) * capacity);
    if (!q->packets) { // checks if malloc failed
        perror("Failed to Malloc"); 
        return 1;
    }
 
    memset(q->packets, 0, sizeof(packet_t *) * capacity);

    //initialze rest of struct
    q->maxCapacity = capacity;
    q->curSize = 0;
    q->head = 0;
    q->tail = 0;
    q->queueID = queueID; 

    pthread_mutex_init(&q->mutex, NULL); // Mutex/lock for queue


    pthread_cond_init(&q->notEmpty, NULL);  
    pthread_cond_init(&q->notFull, NULL);  

    *queue = q;
    return 0;
}

// pushes packet to the end of our queue
int push(packetQueue_t *queue, packet_t **packet) {

   pthread_mutex_lock(&queue->mutex);

    //check if queue is full then wait if it is (gets rid of lock while waiting)
    while (queue->curSize == queue->maxCapacity) {
        //waits untill condition is met
        pthread_cond_wait(&queue->notFull, &queue->mutex);
        
    }


    queue->packets[queue->tail] = *packet;

    //update tail (circular queue)
    queue->tail = (queue->tail + 1) % queue->maxCapacity;


    queue->curSize = queue->curSize +1;
    

    pthread_cond_signal(&queue->notEmpty);

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}


// pops the head of our queue
// if queue is empty return ERROR (UP FOR DEBATE)

int pop(packetQueue_t *queue, packet_t **packet) {

    //locks the queue while popping from the thread
    pthread_mutex_lock(&queue->mutex);

    //checks to make sure queue is not empty 
    // while it is wait till its not (this gets rid of lock while waiting)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1; // Wait max 1 second

    while (queue->curSize == 0) {
        int ret = pthread_cond_timedwait(&queue->notEmpty, &queue->mutex, &ts);
        if (ret == ETIMEDOUT) {
            pthread_mutex_unlock(&queue->mutex);
            *packet = NULL;
            return -1;
        }
    }


    // grab popped packet 
    *packet = queue->packets[queue->head];
    queue->packets[queue->head] = NULL;

    // update queue indicies
    queue->head = (queue->head + 1) % queue->maxCapacity;
    queue->curSize = queue->curSize - 1;

    pthread_cond_signal(&queue->notFull);
    pthread_mutex_unlock(&queue->mutex);


    return 0;
}


// clears the queue
int queueClear(packetQueue_t *queue) {


    pthread_mutex_destroy(&queue->mutex);   
    pthread_cond_destroy(&queue->notEmpty);  
    pthread_cond_destroy(&queue->notFull);  

    free(queue->packets);
    free(queue);

    return 0;
}