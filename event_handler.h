#ifndef EVENTHANDLER
#define EVENTHANDLER

typedef struct PriorityQueue PriorityQueue;

typedef enum {
    TWI_STATUS_START_TX           = 0x08,  ///< Warunek START został wysłany
    TWI_STATUS_REP_START_TX       = 0x10,  ///< Powtórzony warunek START został wysłany
    TWI_STATUS_SLAW_ACK           = 0x18,  ///< Adres SLA+W (Slave Address + Write - adres urządzenia podrzędnego + zapis) został wysłany; odebrano ACK (Acknowledge - potwierdzenie)
    TWI_STATUS_SLAW_NACK          = 0x20,  ///< Adres SLA+W został wysłany; odebrano NOT ACK (Not Acknowledge - brak potwierdzenia)
    TWI_STATUS_DATA_TX_ACK        = 0x28,  ///< Bajt danych został wysłany; odebrano ACK
    TWI_STATUS_DATA_TX_NACK       = 0x30,  ///< Bajt danych został wysłany; odebrano NOT ACK
    TWI_STATUS_ARBITRATION_LOST   = 0x38   ///< Utrata arbitrażu w SLA+W lub bajtach danych
} TWSR_MASTER_t;

typedef struct {
  unsigned char priority;
  union{
    TWSR_MASTER_t charCode;
    int intCode;
  };
  
}Ticket;

void getNewEvent(PriorityQueue *pq);
void handleEvent(TWSR_MASTER_t a);
void generateEvent(TWSR_MASTER_t event,unsigned char P, PriorityQueue *pq);

#endif // EVENTHANDLER