// ====================================================
// kod realizuje sczytanie poomiaru przyspieszenia i
// przyspieszenia kątowego z modułu LSM6DOSX na 
// arduino mega 2560 
// komunikacja przy pomocy protokołu IIC
// komunikacja w trybie 1 (defoult, brak IIC passthrough)
// podłączenie:
// VIN : 5v (i tak konwertowane na 3.3)
// GND : GND
// SDA : SDA (port 20) 
// SCL : SCL (port 21)
// ====================================================

#define SDA 20
#define SCL 21
#define INT4_PIN 2
// ============================================================================
// ADRES CHIPU MA POSTAĆ : ADRESCHIPU1 + SAD + R/W
// SAD - bit zapewniający 2 unikalne adresy (np dla 2 chipów) 
// R/W - Read : 1 Write : 0
// ============================================================================


// ============================================================================
// DEFINICJE KOMEND SAD[0] = 0
// ============================================================================
#define READ1 0xD5
#define WRITE1 0xD4

// ============================================================================
// DEFINICJE KOMEND SAD[0] = 1
// ============================================================================
#define READ2 0xD7
#define WRITE2 0xD6

// ============================================================================
// ustawienia LSM6DOSX
// ============================================================================
#define ACC_SENS_2g 0.061
#define GYR_SENS 8.75
// ============================================================================
// pod-adresy [sub] żyroskopu LSM6DOSX
// ============================================================================

#define OUTX_L_G 0x22
#define Zyroskop_X_Low 0x22
#define OUTX_H_G 0x23
#define Zyroskop_X_High 0x23

#define OUTY_L_G 0x24
#define Zyroskop_Y_Low 0x24
#define OUTY_H_G 0x25
#define Zyroskop_Y_High 0x25

#define OUTZ_L_G 0x26
#define Zyroskop_Z_Low 0x26
#define OUTZ_H_G 0x27
#define Zyroskop_Z_High 0x27

// ============================================================================
// pod-adresy [sub] akcelerometru LSM6DOSX
// ============================================================================

#define OUTX_L_A 0x28
#define Akcelerometr_X_Low 0x28
#define OUTX_H_A 0x29
#define Akcelerometr_X_High 0x29

#define OUTY_L_A 0x2A
#define Akcelerometr_Y_Low 0x2A
#define OUTY_H_A 0x2B
#define Akcelerometr_Y_High 0x2B

#define OUTZ_L_A 0x2C
#define Akcelerometr_Z_Low 0x2C
#define OUTY_Z_A 0x2D
#define Akcelerometr_Z_High 0x2D

// ============================================================================
// pod-adresy [sub] czujnika temperatury LSM6DOSX
// ============================================================================

#define OUT_TEMP_L 0x20
#define Termometr_Low 0x20
#define OUT_TEMP_H 0x21
#define Termometr_High 0x21

// ============================================================================
// fifo registers
// ============================================================================

#define FIFO_CTRL1  0x07
#define FIFO_CTRL2  0x08
#define FIFO_CTRL3  0x09
#define FIFO_CTRL4  0x0A
#define INT1_CTRL   0x0D
#define CTRL2_A     0x10
#define CTRL2_G     0x11
#define CTRL10_C    0x19

// ============================================================================
// interrupt sources
// ============================================================================

#define EMB_FUNC_STATUS_MAINPAGE 0x35

// not transmitted to sensor, local only check lsmsetup
#define FIFO_size 1512 

bool writeRegister(unsigned char subAdress, unsigned char data);
bool startSignal();
bool stopSignal();
bool waitForTWINT();
bool checkCodeTWSR(char expectedCode);
void I2CDataInterrupt();

void ATmega2560Setup()
{
  // 1. Włączenie zasilania dla modułu TWI (Two Wire Interface - interfejs dwuprzewodowy)
  PRR0 &= ~(1 << PRTWI);
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);

  // 2. Konfiguracja zegara I2C na 400 kHz (Fast Mode)
  TWSR &= ~((1 << TWPS1) | (1 << TWPS0)); // Prescaler = 1
  TWBR = 12;                               // SCL = 400 kHz przy F_CPU = 16 MHz

  // 3. Włączenie modułu TWI oraz linii SCL (Serial Clock) i SDA (Serial Data)
  TWCR = (1 << TWEN);

  TCCR1A = 0;

}


bool LSM6DOSXSetup()
{
  bool success = 1;
  unsigned char frequency = 0x40;
  
  success &= writeRegister(FIFO_CTRL4, 0x00); // reset fifo

  if(!success)
  {
    Serial.print("1");
  }
  success &= writeRegister(CTRL2_G, frequency);
  success &= writeRegister(CTRL2_A, frequency);
  success &= writeRegister(CTRL10_C, 0x20);

  success &= writeRegister(FIFO_CTRL1, 0xD8); 
  success &= writeRegister(FIFO_CTRL2, 0x00); 
  success &= writeRegister(FIFO_CTRL3, 0x44);
  success &= writeRegister(FIFO_CTRL4, 0x46);

  success &= writeRegister(INT1_CTRL, 0x08);

  if(success == 1)
  {
    Serial.print("lsm initialized\n");
    return true;
  }

  return false;
}


void resetLSMFiFo()
{
  Serial.print("fifo reset\n");
  writeRegister(FIFO_CTRL4, 0x00);
  delayMicroseconds(2); 
  writeRegister(FIFO_CTRL4, 0x46);
}

void FORCE_TWI_RESET()
{
  Serial.print("force reset \n");
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);

    for (int i = 0; i < 9; i++) {
        if (digitalRead(SDA) == HIGH) break;
        
        pinMode(SCL, OUTPUT);
        digitalWrite(SCL, LOW);
        delayMicroseconds(10);
        pinMode(SCL, INPUT_PULLUP);
        delayMicroseconds(10);
    }

    pinMode(SDA, OUTPUT);
    digitalWrite(SDA, LOW);
    delayMicroseconds(10);
    pinMode(SCL, INPUT_PULLUP);
    delayMicroseconds(10);
    digitalWrite(SDA, HIGH);
    ATmega2560Setup();
    LSM6DOSXSetup();
}
bool waitForTWINT()
{
    uint16_t timeout = 1000;
    while (!(TWCR & (1 << TWINT)))
    {
      delayMicroseconds(1);
        if (--timeout == 0)
        {
          Serial.print("twint wait\n");
          FORCE_TWI_RESET();
            return false; 
        }
    }
    return true;
}


bool checkCodeTWSR(char code)
{
  char TWSRerrorCode = TWSR & 0xF8;
  if(TWSRerrorCode == code) // SLA + R, got ack back
  {
    return true;
  }

  Serial.println(TWSRerrorCode, HEX);
  return false;
}


// ============================================================================
// funckja realizuje wysłanie sygnału rozpoczęcia komunikacji. 
// Opis w dokumentacji : str 50
// Data line : HIGH -> LOW
// SCL : HIGH
// operacja jest wykonywana automatycznie przez procesor 
// (instrukcja na stronie 244 +)
// ============================================================================ 
bool startSignal()
{  // <-- Sprawdź, czy ten nawias otwiera ciało funkcji!
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if(!waitForTWINT()) {return false;}

    unsigned char status = TWSR & 0xF8;
    if (status != 0x08 && status != 0x10) 
    {
        stopSignal();
        return false;
    }

    return true;
}

bool stopSignal()
{
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
  while (TWCR & (1 << TWSTO));
  return true;
}

// ============================================================================
// funckja realizuje wysłanie pod adresu rejestru "niewolnika" 
// ============================================================================ 
bool transmitSubAdress(unsigned char subAdress)
{
  TWDR = WRITE1;
  TWCR = (1 << TWINT) | (1 << TWEN);
  waitForTWINT();
  if (!checkCodeTWSR(0x18)) return false;

  TWDR = subAdress;
  TWCR = (1 << TWINT) | (1 << TWEN);
  waitForTWINT();
  if (!checkCodeTWSR(0x28)) return false;

  return true;
}

// ============================================================================
// funckja realizuje wysłanie adresu "niewolnika" 
// "każde urządzenie porówna pierwsze 7 bitów adresowych"
// Opis w dokumentacji : str 50
// ============================================================================ 

bool transmitReadAdressAck()
{
  if(!startSignal()) return false;
  TWDR = READ1;
  TWCR = (1 << TWINT) | (1 << TWEN);
  waitForTWINT();
  if (!checkCodeTWSR(0x40)) return false;

  return true;
}

void getProcesorTime(uint16_t &ticks, volatile uint16_t &overflows)
{
  uint8_t oldSREG = SREG;
  cli();
  ticks = TCNT1;
  SREG = oldSREG;

  if ((TIFR1 & (1 << TOV1)) && (ticks < 32768)) {
    overflows++;
  }

}
bool readRegister(unsigned char subAdress,unsigned char &lowByte, unsigned char &highByte)
{
  if(!startSignal()) return false;

  //transmit sub adress
  if(!transmitSubAdress(subAdress)) {
      stopSignal();
      return false;
  }
  if(!transmitReadAdressAck()) {
      stopSignal();
      return false;
  }

  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  waitForTWINT();
  lowByte = TWDR;

  TWCR = (1 << TWINT) | (1 << TWEN);
  waitForTWINT();
  highByte = TWDR;

  stopSignal();
  
  return true;
}

bool repeatReadRegister(unsigned char subAdress, unsigned char buffer[], int howManyBytes)
{
  if (!startSignal())               return false;
  if(!transmitSubAdress(subAdress)) { stopSignal(); return false;}
  if(!transmitReadAdressAck())      { stopSignal(); return false;}

  for(int i = 0; i < howManyBytes; i++)
  {
    if(i == howManyBytes - 1)
    {
      TWCR = (1 << TWINT) | (1 << TWEN);
      if (!waitForTWINT() || !checkCodeTWSR(0x58)) { stopSignal(); return false; }
    }
    else
    {
      TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
      if (!waitForTWINT() || !checkCodeTWSR(0x50)) { stopSignal(); return false; }
    }

    buffer[i] = TWDR;
  }

  stopSignal();
  return true;
}

#define WRITE_DELEY 30
bool writeRegister(unsigned char subAdress, unsigned char data) 
{
    if (!startSignal()) return false;
    
    TWDR = WRITE1;
    TWCR = (1 << TWINT) | (1 << TWEN);
    delayMicroseconds(WRITE_DELEY);
    waitForTWINT();
    if (!checkCodeTWSR(0x18)) { stopSignal(); return false; } 
    
    // Wysłanie adresu rejestru docelowego
    delayMicroseconds(WRITE_DELEY);

    TWDR = subAdress; 
    TWCR = (1 << TWINT) | (1 << TWEN);
    waitForTWINT();
    if (!checkCodeTWSR(0x28)) { stopSignal(); return false; } 
    
    // Wysłanie danych
    delayMicroseconds(WRITE_DELEY);
    TWDR = data; 
    TWCR = (1 << TWINT) | (1 << TWEN);
    waitForTWINT();
    if (!checkCodeTWSR(0x28)) { stopSignal(); return false; }
    stopSignal();
    return true;
}

int16_t get16bitRegisterData(unsigned char registerAdress)
{
  unsigned char L, H;
  int16_t data_raw;
  if(readRegister(registerAdress ,L, H))
  {
    data_raw = (int16_t)((H << 8) | L);
  } else {
    Serial.println("Blad odczytu danych!");
  }
  return data_raw;
}

double getTemperature()
{
  double temp;
  temp = get16bitRegisterData(OUT_TEMP_L)/256 + 25;
  return temp;
}

double getAccelerationX() { return get16bitRegisterData(Akcelerometr_X_Low) * ACC_SENS_2g / 1000.0; }
double getAccelerationY() { return get16bitRegisterData(Akcelerometr_Y_Low) * ACC_SENS_2g / 1000.0; }
double getAccelerationZ() { return get16bitRegisterData(Akcelerometr_Z_Low) * ACC_SENS_2g / 1000.0; }
double getGyroX() { return get16bitRegisterData(Zyroskop_X_Low) * GYR_SENS / 1000.0; }
double getGyroY() { return get16bitRegisterData(Zyroskop_Y_Low) * GYR_SENS / 1000.0; }
double getGyroZ() { return get16bitRegisterData(Zyroskop_Z_Low) * GYR_SENS / 1000.0; }

volatile uint16_t timer1_overflows = 0;
ISR(TIMER1_OVF_vect) {
  timer1_overflows++;
}

volatile unsigned char FIFObuf[FIFO_size];
volatile bool canReadFiFo = false;

void I2CDataInterrupt()
{
  canReadFiFo = true;
}

int main(void) {
    init();
    Serial.begin(9600);
    delay(50);
    ATmega2560Setup();
    delay(50);

    while(!LSM6DOSXSetup())
    {
      LSM6DOSXSetup();
    }

    delay(50);

    sei();
    pinMode(INT4_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(INT4_PIN), I2CDataInterrupt, RISING);
    delay(50);
    resetLSMFiFo();

    
    
    
    unsigned char LH[2] = {0x01,0x01};
    uint16_t ticks;
    int timeout = 0;

    while(true) {
      if(canReadFiFo == true)
      {
        if(!repeatReadRegister(0x78, FIFObuf, FIFO_size))
        {
          canReadFiFo = false;
          continue;
          Serial.print("ReadFaild\n");
        }
        canReadFiFo = false;
        timeout = 0;
        resetLSMFiFo();
        Serial.write((const uint8_t*)FIFObuf, sizeof(FIFObuf[0]) * FIFO_size);
      }
      else
      {
        timeout += 1;
      }

      // if(timeout == 10000)
      // {
      //   resetLSMFiFo();

      //   timeout = 0;
      // }
    }
}