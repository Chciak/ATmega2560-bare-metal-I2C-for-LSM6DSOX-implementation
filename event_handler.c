#include "event_handler.h"
#include "priorityQueue.h"

void handleEvent(TWSR_MASTER_t a)
{
  if(a == TWI_STATUS_ARBITRATION_LOST)
  {

  }
}

void getNewEvent(PriorityQueue *pq)
{
  Ticket newTicket = pqPop(pq);

  handleEvent(newTicket.charCode);
  
}

void generateEvent(TWSR_MASTER_t event, unsigned char P,PriorityQueue *pq)
{
  Ticket newTicket;
  newTicket.priority = P;
  newTicket.charCode = event;
  pqInsert(pq, newTicket);
}
