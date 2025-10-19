#pragma once
#include "I2C_eeprom.h"


#define FRAM_B_INCARNATION                   0    // pami?c EEPROM adres    0 - Incarnation number
#define FRAM_DATA_STRUCT     10  


#define HIT 0
#define KIDS 1
#define GIFT 2
#define COMBAT 3
#define COMBAT_KIDS 4
#define DOUBLE_HIT_GIFT_KIDS 5
#define BLACK_WHITE 6
#define GIFT_ESPANIOL 7
#define FIST 8
#define MONSTER 9
#define DOUBLE_HIT_GIFT_KIDS_ESPANIOL 10
#define DOUBLE_HIT 11


typedef struct FramStore {
    uint8_t InterwalPrezentacjiMinuty = 5;
    uint8_t Language = 0; //0 - English
    uint8_t Syslog = 0; //disabled
    uint8_t SkalaSily = 100; //Skala sily 100%
    uint8_t SkalaSilyKicker = 100; //Skala sily 100%
    uint8_t Mp3Volume = 12;
    uint8_t FreePlay = 0;
    uint8_t LedRgb = 0;
    uint8_t PcbRev = 0;
    uint16_t Record = 0;
    uint32_t TotalCounter = 0;
    uint32_t TempCounter = 0;
    uint32_t TestCredit = 0;
    float CoinBankPuls[10] = {1, 3, 5, 0, 10, 20, 50, 0, 1};
    float CoinBankCred[10] = {1, 3, 5, 0, 10, 20, 50, 0, 1};
    float QuarterCoinCnt = 0;
    float Credit;
    uint8_t PulseTime = 100;
    uint8_t Kids = 0; // 1 - Boxer Kids, small stren and less Led RGB
    uint8_t TicketRecord = 0;
    uint8_t TicketCredit = 0;
    uint8_t TicketScore = 0;
    uint8_t GiftAward = 0;

    uint32_t GiftWinRatioMale = 0;
    uint32_t GiftWinRatioSrednie = 0;
    uint32_t GiftWinRatioDuze = 0;

    uint32_t GiftCntMale = 0;
    uint32_t GiftCntSrednie = 0;
    uint32_t GiftCntDuze = 0;
    
    uint32_t GiftCntOut555 = 0;
    uint32_t GiftCntOut666 = 0;
    uint32_t GiftCntOut777 = 0;

    uint8_t BoxerModel = 0; // 0 - Hit, 1-Kids, 2-Gift, 3-Combat
    uint8_t BoxerMat = 0; // 1 - Led RBG only black and white
    uint8_t Boxer3Player = 0; // 1 - Led RBG only black and white
    
    uint8_t SkalaSilyMlot = 100; //Skala sily 100%

    uint16_t RecordBoxer = 0;
    uint16_t RecordKicker = 0;
    uint16_t RecordHammer = 0;
    uint16_t RecordTriathlon  = 0;
    
} FramStruct_t;

extern volatile FramStruct_t Fram;
extern I2C_eeprom ee;




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
