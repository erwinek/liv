#ifndef PCF_H
#define PCF_H

#include <PCF8574.h>

#define INTERRUPTED_PIN 33

extern volatile uint8_t PcfKey;
extern volatile uint8_t PcfOut;
extern volatile uint8_t PcfCoin;
extern volatile uint8_t PcfGP;

extern TaskHandle_t TaskPcfHandle;
extern uint32_t TicketOut;
#define ON 0
#define OFF 1

  
#define SET_COUNTER { (PcfOut |= (0x01<<0)); vTaskResume( TaskPcfHandle ); }
#define CLR_COUNTER { (PcfOut &= ~(0x01<<0)); vTaskResume( TaskPcfHandle ); }

#define SET_GRUSZKA2 { (PcfOut |= (0x01<<2)); vTaskResume( TaskPcfHandle ); }
#define CLR_GRUSZKA2 { (PcfOut &= ~(0x01<<2)); vTaskResume( TaskPcfHandle ); }
#define SET_GRUSZKA { (PcfOut |= (0x01<<3)); vTaskResume( TaskPcfHandle ); }
#define CLR_GRUSZKA { (PcfOut &= ~(0x01<<3)); vTaskResume( TaskPcfHandle ); }

#define KEY_START1 (0==(PcfKey&0x20))
#define KEY_START2 (0==(PcfKey&0x10))
#define KEY_3PLAYER (0==(PcfKey&0x08))
#define KEY_START_HAMMER (0==(PcfKey&0x08))
#define KEY_DOWN (0==(PcfKey&0x01))
#define KEY_SETUP (0==(PcfKey&0x02))
#define KEY_UP (0==(PcfKey&0x04))


bool PcfInit();

extern volatile uint32_t IsrIntCnt;
extern uint32_t timestampisrInt;



#endif //PCF_H