#include "priorityQueue.h"
void swap(Ticket *a, Ticket *b)
{
    Ticket temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(PriorityQueue *pq, int idx)
{
    int largest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < pq->size && pq->items[left].priority > pq->items[largest].priority )
        largest = left;

    if (right < pq->size && pq->items[right].priority > pq->items[largest].priority)
        largest = right;

    if (largest != idx)
    {
        swap(&pq->items[idx], &pq->items[largest]);
        heapify(pq, largest);
    }
}

void pqInsert(PriorityQueue *pq, Ticket t)
{
  unsigned char idx = pq->size;
  pq->size += 1;
  pq->items[idx] = t;
  
  while(idx != 0 && pq->items[(idx - 1) /2].priority  < pq->items[idx].priority) // rodzic ma mniejszą wartość
  {
    swap(&pq->items[(idx - 1) /2] , &pq->items[idx]);
    idx = (idx - 1) /2;
  }
}

Ticket pqPop(PriorityQueue *pq)
{
  if(pq->size == 1)
  {
    pq->size -= 1;
    return pq->items[0];
  }

  Ticket root = pq->items[0];

  pq->items[0] = pq->items[pq->size - 1];
  pq->size -= 1;
  heapify(pq,0);
}