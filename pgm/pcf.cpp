#include <PCF8574.h>
#include "FRAM.h"
#include "counter.hpp"
#include "pcf.h"
#include "myLcd.hpp"

#define ENABLE_TICKET1  { (PcfOut |= (0x01<<1));   }
#define DISABLE_TICKET1 { (PcfOut &= ~(0x01<<1));  }

#define DISABLE_TICKET2  { (PcfOut |= (0x01<<5));  }
#define ENABLE_TICKET2   { (PcfOut &= ~(0x01<<5)); }


uint32_t timestampisrInt = 0;
volatile uint32_t IsrIntCnt = 0;

#define INTERRUPTED_PIN 33

PCF8574 pcf8574_out(0x20, 21, 22); //address, SDA, SCL
PCF8574 pcf8574_key(0x21, 21, 22); 
PCF8574 pcf8574_coin(0x22, 21, 22);  
PCF8574 pcf8574_gp(0x23  , 21, 22); //address, SDA, SCL


volatile uint8_t PcfKey = 0xFF;
volatile uint8_t PcfOut = 0xFF;
volatile uint8_t PcfCoin = 0xFF; 
volatile uint8_t PcfGP = 0xFF; 

uint32_t CoinBankTimer[8] = {0};
uint32_t NayaxTimer = 0;

TaskHandle_t TaskPcfHandle;
uint32_t TicketOut = 0;

#define ON 0
#define OFF 1

  
bool PcfHandle(void)
{

  bool ret = false;
  uint8_t tempKey = (uint8_t)pcf8574_key.digitalReadAll();
  uint8_t tempCoin = (uint8_t)pcf8574_coin.digitalReadAll();
  uint8_t tempGp = (uint8_t)pcf8574_gp.digitalReadAll();



  if (TicketOut > 0) {
    PcfOut &= ~(0x01<<5); // ENABLE_TICKET2;
  }
  else PcfOut |= (0x01<<5); // DISABLE_TICKET2;
  
  pcf8574_out.digitalWriteAll(PcfOut);    

  if(PcfKey != tempKey) {
    if ((tempKey & 0x80) == 0)
    {
      printf("\n tempKey & 0x80 = 0");

      if ((PcfKey & 0x80) != 0)
      {
        printf(" PcfKey & 0x80 != 0");
        if (TicketOut > 0) {
          TicketOut--;
          if(TicketOut==0) PcfOut |= (0x01<<5); // DISABLE_TICKET2;
        }
        printf("\n TicketOut=%d", TicketOut);
      }
    }

    PcfKey = tempKey;
    ret = true;
  }
  if (tempCoin != PcfCoin)
  {
    PcfCoin = tempCoin;
    ret = true;
  }
  if (tempGp != PcfGP)
  {
    PcfGP = tempGp;
    printf("\n PcfGP=%d", PcfGP);
    ret = true;
  }
  
  return ret;
}




const TickType_t xDelay30ms = 30 / portTICK_PERIOD_MS;

void TaskPcf(void *pvParameters)
{
  for (;;)
  {
    uint32_t timestamp = millis();
    uint32_t intDelay = 0;
    if(timestampisrInt) {
      intDelay = timestamp - timestampisrInt;
      timestampisrInt = 0;
      //printf("\n IsrIntCnt = %d intDelay = %d", IsrIntCnt, intDelay);
    }
        
      if (PcfHandle())
      {
        printf("\n PcfKey=%X PcfCoin=%X PcfGp=%X", PcfKey, PcfCoin, PcfGP);
        
        if ((~PcfGP) & (0x01 << 2)) { //sensor Gift
          
        }
        
        if ((~PcfGP) & (0x01 << 1)) {
          if(NayaxTimer==0) {
            NayaxTimer = timestamp;
          }
        }
        else if(NayaxTimer > 0) {
          uint32_t delta = timestamp - NayaxTimer;
          NayaxTimer = 0;
          Serial.print("\n deltaNay=");
          Serial.println(delta);
          if ((delta >= 40) && (delta <= 200)) {
              Serial.print("NayaxCred=");
              Serial.println(Fram.CoinBankCred[8]);
              Fram.Credit += Fram.CoinBankCred[8];
              TicketOut += Fram.CoinBankCred[8] * Fram.TicketCredit;
              Fram.TotalCounter += Fram.CoinBankPuls[8];
              Fram.TempCounter += Fram.CoinBankPuls[8];
              MechCount.AddCounts(Fram.CoinBankPuls[8]); 
          }
        }

        for (int i = 0; i < 8; i++)
        {
          if (((~PcfCoin) & (0x01 << i))) {
            if(CoinBankTimer[i]==0) CoinBankTimer[i] = timestamp - intDelay;
            printf("\n CoinBankTimer[%d]=%X", i, timestamp);
          }
          else if (CoinBankTimer[i] != 0)
          {
            uint32_t delta = timestamp - CoinBankTimer[i] - intDelay;
            Serial.print("\n deltaT=");
            Serial.println(delta);
            const uint8_t tolerance = 90;
            uint8_t min_pulse = 10;
            if(Fram.PulseTime > tolerance) min_pulse = Fram.PulseTime - tolerance;
            uint8_t max_pulse = Fram.PulseTime + tolerance;            

            if ((delta > min_pulse) && (delta < max_pulse))
            {
              Serial.print("CoinBankCred=");
              Serial.println(Fram.CoinBankCred[i]);
              Fram.Credit += Fram.CoinBankCred[i];
              TicketOut += Fram.CoinBankCred[i] * Fram.TicketCredit;
              Fram.TotalCounter += Fram.CoinBankPuls[i];
              Fram.TempCounter += Fram.CoinBankPuls[i];
              MechCount.AddCounts(Fram.CoinBankPuls[i]);
            }
            CoinBankTimer[i] = 0;
          }
        }
      }

    
    
    vTaskSuspend( NULL );
  }
}

bool PcfInit()
{
    // Start library
    if(!pcf8574_out.begin()) {
      Serial.println("Error pcf8574_out");
    }
    if(!pcf8574_key.begin()) {
      Serial.println("Error pcf8574_key");
    }    
    if(!pcf8574_coin.begin()) {
      Serial.println("Error pcf8574_coin");
    }
    if(!pcf8574_gp.begin()) {
      Serial.println("Error pcf8574_gpn");
    }

    pcf8574_out.pinMode(P0, OUTPUT, HIGH);
    pcf8574_out.pinMode(P1, OUTPUT, HIGH);
    pcf8574_out.pinMode(P2, OUTPUT, HIGH);
    pcf8574_out.pinMode(P3, OUTPUT, HIGH);
    pcf8574_out.pinMode(P4, OUTPUT, HIGH);
    pcf8574_out.pinMode(P5, OUTPUT, HIGH);
    pcf8574_out.pinMode(P6, OUTPUT, HIGH); //mute
    pcf8574_out.pinMode(P7, OUTPUT, HIGH); //stby

    pcf8574_key.pinMode(P0, INPUT);
    pcf8574_key.pinMode(P1, INPUT);
    pcf8574_key.pinMode(P2, INPUT);
    pcf8574_key.pinMode(P3, INPUT);
    pcf8574_key.pinMode(P4, INPUT);
    pcf8574_key.pinMode(P5, INPUT);
    pcf8574_key.pinMode(P6, INPUT);
    pcf8574_key.pinMode(P7, INPUT);

    pcf8574_coin.pinMode(P0, INPUT);
    pcf8574_coin.pinMode(P1, INPUT);
    pcf8574_coin.pinMode(P2, INPUT);
    pcf8574_coin.pinMode(P3, INPUT);
    pcf8574_coin.pinMode(P4, INPUT);
    pcf8574_coin.pinMode(P5, INPUT);
    pcf8574_coin.pinMode(P6, INPUT);
    pcf8574_coin.pinMode(P7, INPUT);  

    pcf8574_gp.pinMode(P0, INPUT);
    pcf8574_gp.pinMode(P1, INPUT);
    pcf8574_gp.pinMode(P2, INPUT);
    pcf8574_gp.pinMode(P3, INPUT);

    xTaskCreate(
                  TaskPcf,            // Task function. 
                  "TaskPcf",          // String with name of task. 
                  10000,              // Stack size in bytes. 
                  NULL,               // Parameter passed as input of the task 
                  (configMAX_PRIORITIES - 1), // Priority of the task. 
                  &TaskPcfHandle);    // Task handle. 

  
  SET_GRUSZKA;
  SET_GRUSZKA2;

  return true;
}
