#ifndef PRIORITYQUEUE
#define PRIORITYQUEUE

#include "event_handler.h"

#define MAX_PQ 255

struct PriorityQueue{
    Ticket items[MAX_PQ];
    unsigned char size;
} ;

void swap(Ticket *a, Ticket *b);
void pqInsert(PriorityQueue *pq, Ticket t);
Ticket pqPop(PriorityQueue *pq);


#endif //PRIORITYQUEUE
