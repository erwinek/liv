//Zarowki + Display7Seg
int latchPin = 5;
int clkPin = 18;
int dtPin = 23;
int ZAR_OE = 19;

//public
#define HC595_LEN 4
#define DISPL_7SEG_NUMBER 12 
#define DISPL_7SEG_3_PLAYER 10

#define SPI_MAX_DATA_SIZE (HC595_LEN + DISPL_7SEG_NUMBER + DISPL_7SEG_3_PLAYER)

#define NUM0  0b0111111
#define NUM1  0x06
#define NUM2  0b1011011
#define NUM3  0b1001111
#define NUM4  0b1100110
#define NUM5  0b1101101
#define NUM6  0b1111101
#define NUM7  0x7
#define NUM8  0x7F
#define NUM9  0b1101111

static const uint8_t Digit7Seg[10] = {NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9};
static const uint8_t CircleAround[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
static const uint8_t UpAndDown[] = {0x63, 0x5C, 0x63, 0x5C, 0x63, 0x5C, 0x63};

typedef union Zarowki {
  uint32_t All;
  struct {
    uint32_t Z0:1;
    uint32_t Z1:1;
    uint32_t Z2:1;
    uint32_t Z3:1;
    uint32_t Z4:1;
    uint32_t Z5:1;
    uint32_t Z6:1;
    uint32_t Z7:1;
    
    uint32_t Z8:1;    
    uint32_t Z9_Logo:1;
    uint32_t Z10_START_BOXER:1;
    uint32_t Z11_START_KOPACZ:1;
    uint32_t Z12_START_3PLAYER:1; //KORONA MONSTER
    uint32_t Z13_FIST:1; //logo monster
    uint32_t Z14_HALOGEN1:1;
    uint32_t Z15_HALOGEN2:1;
    
    uint32_t Z16_MOTOR1:1; 
    uint32_t Z17_MOTOR2:1;
    uint32_t Z18_MOTOR3:1;
    uint32_t Z19_LED1:1;
    uint32_t Z20_LED2:1;
    uint32_t Z21_LED3:1;
    uint32_t Z22_SKRZYNIA:1; 
    uint32_t Z23:1;
    
 
  };
} Zarowki_t;


Zarowki_t Zar;

uint16_t Displ1;
uint16_t Displ2;
uint16_t Displ3;

typedef union SpiLeds {
  uint8_t All;
  struct {
    uint8_t INH_BANK:1;
    uint8_t INH_COIN:1;
    uint8_t INH_NAYAX:1;
    uint8_t NotUsed1:1;
    uint8_t NotUsed2:1;
    uint8_t NotUsed3:1;
    uint8_t LED2:1;
    uint8_t LED1:1;   
  };
} SpiLed_t;

SpiLed_t SpiLed;

#define DISABLE_BANK_COIN_NAY (SpiLed.All = 0x00)
#define ENABLE_BANK_COIN_NAY (SpiLed.All |= 0x07)

uint8_t SpiDataLen = HC595_LEN + DISPL_7SEG_NUMBER;

void SpiInit(void)
{
  pinMode(latchPin, OUTPUT);
  pinMode(clkPin, OUTPUT); 
  pinMode(dtPin, OUTPUT);
  pinMode(ZAR_OE, OUTPUT);
  digitalWrite(ZAR_OE, HIGH);
  SpiLed.All = 0;

  if (1==Fram.Boxer3Player) {
    SpiDataLen = SPI_MAX_DATA_SIZE;
  }
  printf("\n\n SpiDataLen=%d \n\n",  SpiDataLen);
}

void UpdateSPI(void);

//priv
uint8_t DataSpi[SPI_MAX_DATA_SIZE+1] = {0};

void DisplayWynik(uint16_t value)
{ 
  DataSpi[9] = 0; //pusty zamiast zera
  DataSpi[10] = 0;//pusty zamiast zera
  DataSpi[11] = Digit7Seg[value%10];
  if(value>=10) DataSpi[10] = Digit7Seg[(value/10)%10];
  if(value>=100) DataSpi[9] = Digit7Seg[(value/100)%10];
  if(value==0xFFFF) { DataSpi[9]=0; DataSpi[10]=0; DataSpi[11]=0; }

  UpdateSPI();
}

void DisplayWynikEffect(uint8_t value)
{ 
  static uint8_t cnt = 0;

  if(value==0) {
    DataSpi[11] = CircleAround[cnt];
    DataSpi[10] = CircleAround[cnt];
    DataSpi[9] = CircleAround[cnt];
    cnt++;
    if(cnt>=sizeof(CircleAround))  cnt = 0;
  }
  else {
    DataSpi[11] = UpAndDown[cnt];
    DataSpi[10] = UpAndDown[cnt+1];
    DataSpi[9] = UpAndDown[cnt];
    cnt++;
    if(cnt>=sizeof(UpAndDown)-1)  cnt = 0;
  }
}

void DisplayRekord(uint16_t value)
{ 
  static uint64_t timestamp = millis();
  DataSpi[5] = 0; //pusty zamiast zera
  DataSpi[6] = 0;//pusty zamiast zera
  DataSpi[7] = Digit7Seg[value%10];
  if(value>=10) DataSpi[6] = Digit7Seg[(value/10)%10];
  if(value>=100) DataSpi[5] = Digit7Seg[(value/100)%10];
  if(value==0xFFFF) { DataSpi[5]=0; DataSpi[6]=0; DataSpi[7]=0; }

  UpdateSPI();
}

void DisplayRekordEffect(uint16_t value)
{ 
  static uint8_t cnt = 0;

  if(value==0) {
    DataSpi[7] = CircleAround[cnt];
    DataSpi[6] = CircleAround[cnt];
    DataSpi[5] = CircleAround[cnt];
    cnt++;
    if(cnt>=sizeof(CircleAround))  cnt = 0;
  }
  else {
    DataSpi[5] = UpAndDown[cnt];
    DataSpi[6] = UpAndDown[cnt+1];
    DataSpi[7] = UpAndDown[cnt];
    cnt++;
    if(cnt>=sizeof(UpAndDown)-1)  cnt = 0;
  }
}


void DisplayCredit(uint16_t value)
{ 
  static uint64_t timestamp = millis();
  DataSpi[3] = 0;
  DataSpi[2] = Digit7Seg[value%10];
  if(value>=10) DataSpi[3] = Digit7Seg[(value/10)%10];
  if(value==0xFFFF) { DataSpi[2]=0; DataSpi[3]=0;}

  UpdateSPI();
}

void DisplayPlayer(uint8_t player, uint16_t value, bool blink = false)
{
  uint8_t displaySelector = SpiDataLen-13 + ((player-1)*3);
  static uint64_t timestamp = millis();
  static bool state = false;

  if (1==Fram.Boxer3Player) {
    if(millis() - timestamp > 300) {
      state = !state;
      timestamp = millis();
    }
    
    if(blink==true && state==false) {
    DataSpi[displaySelector] = 0; 
  	DataSpi[displaySelector+1] = 0;
  	DataSpi[displaySelector+2] = 0;
    }
    else {
      DataSpi[displaySelector] = 0; 
  	  DataSpi[displaySelector+1] = 0;
  	  DataSpi[displaySelector+2] = Digit7Seg[value%10];
  	  if(value>=10) DataSpi[displaySelector+1] = Digit7Seg[(value/10)%10];
  	  if(value>=100) DataSpi[displaySelector] = Digit7Seg[(value/100)%10];
  	  if(value==0xFFFF) { DataSpi[displaySelector]=0; DataSpi[displaySelector+1]=0; DataSpi[displaySelector+2]=0; }
      if(value==0xEEEE) { DataSpi[displaySelector]=0x8; DataSpi[displaySelector+1]=0x8; DataSpi[displaySelector+2]=0x8; }
    }
  	UpdateSPI();
  }
}

void Display3PlayerEffect(uint8_t player, uint8_t value)
{ 
  static uint8_t cnt = 0;
  uint8_t displaySelector = SpiDataLen-13 + ((player-1)*3);

  if (1==Fram.Boxer3Player) {

  if(value==0) {
    DataSpi[displaySelector] = CircleAround[cnt];
    DataSpi[displaySelector+1] = CircleAround[cnt];
    DataSpi[displaySelector+2] = CircleAround[cnt];
    if (player==3) cnt++;
    if(cnt>=sizeof(CircleAround))  cnt = 0;
  }
  else {
    DataSpi[displaySelector] = UpAndDown[cnt];
    DataSpi[displaySelector+1] = UpAndDown[cnt+1];
    DataSpi[displaySelector+2] = UpAndDown[cnt];
    cnt++;
    if(cnt>=sizeof(UpAndDown)-1)  cnt = 0;
  }
}

}



void UpdateSPI(void)
{
  DataSpi[SpiDataLen-1] = (uint8_t)(Zar.All & (uint8_t)0xFF);
  DataSpi[SpiDataLen-2] = (uint8_t)(Zar.All >>8 & (uint8_t)0xFF);
  DataSpi[SpiDataLen-3] = (uint8_t)(Zar.All >>16 & (uint8_t)0xFF);
  DataSpi[SpiDataLen-4] = SpiLed.All;

  digitalWrite(ZAR_OE, HIGH);
  for(int i=0; i<SpiDataLen; i++ ) {
    shiftOut(dtPin, clkPin, MSBFIRST, DataSpi[i]);
  }
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
  digitalWrite(ZAR_OE, LOW);
}
