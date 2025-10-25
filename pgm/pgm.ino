#define PCF8574_LOW_MEMORY
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include "global.h"
#include "PCF8574.h"
#include "FRAM.h"
#include "DisplSeg.h"
#include "counter.hpp"
#include "Audio.h"
#include <FastLED.h>
#include "language.h"
#include "LedRgb.h"
#include "mp3.h"
#include "ota.h"
#include "myLcd.hpp"
#include <SPI.h>
#include "FS.h"
#include "SD_MMC.h"
#include "I2C_eeprom.h"
#include "updater.h"
#include <SimpleSyslog.h>
#include "counter.hpp"
#include "pcf.h"
#include "gift.h"
#include "test.h"
#include <LEDMatrix.h>

static const char SYSLOG_APPNAME[] = "PGM";

#define VERSION "5.69o"

SimpleSyslog *pSysLog = nullptr;
char ChipIdString[30];

uint8_t Incarnation = 0;
volatile FramStruct_t Fram;
uint64_t HammerTimeUs = 0;
uint32_t wTimerRandom = 0x55aa;

int cntOtwartaBoxer = 0;
int cntOtwartaKopacz = 0;
int cntGruchaZamknieta = 0;
    

#define log(format,args...)  printf(format, ## args)
#define syslog(format,args...) if(pSysLog) { pSysLog->printf(FAC_USER, PRI_DEBUG, format, ## args); } 

#include "lwip/inet.h"
#include "lwip/dns.h"

ip_addr_t SysLogSrv;

uint32_t ElektromagnetTimeout = 3000;

bool QuickStartBoxer = false;
bool QuickStartKopacz = false;
bool QuickStartHammer = false;

uint8_t NumberOfPLayers = 1;
uint8_t CurrentPlayer = 1;
uint16_t TournamentScores[4] = {0, 0, 0, 0}; // 1-based index
bool TournamentInProgress = false;

bool TriathlonInProgress = false;
uint16_t TriathlonScore = 0;

I2C_eeprom ee(0x50);
LEDMatrix matrix(Serial, 1);


// optional
void audio_info(const char *info){
    //Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    //Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
    IsMp3Playing = false;
}

char str[50];

char * uintToStr( const uint64_t num, char *str )
{
  uint8_t i = 0;
  uint64_t n = num;
  
  do
    i++;
  while ( n /= 10 );
  
  str[i] = '\0';
  n = num;
 
  do
    str[--i] = ( n % 10 ) + '0';
  while ( n /= 10 );

  return str;
}

typedef struct InterruptLog {
  uint64_t timestamp;
  bool value;
} InterruptLog_t;


#define TIME_ARRAY_SIZE  30
int64_t StartArray[TIME_ARRAY_SIZE] = {0};
int64_t StopArray[TIME_ARRAY_SIZE] = {0};
InterruptLog_t InterruptLogArray[TIME_ARRAY_SIZE] = {0};

volatile int64_t TimeSens4Start = 0;
volatile int64_t TimeSens4Stop = 0;
volatile int64_t TimeSens0Start = 0;
volatile int64_t TimeSens0Stop = 0;
volatile uint32_t rising = 0;
volatile uint32_t falling = 0;
bool CzyGruchaOpadla = false;

void ARDUINO_ISR_ATTR isrSens1() {
  if(InterruptTimeArrayIndex<TIME_ARRAY_SIZE) {
    InterruptLogArray[InterruptTimeArrayIndex].timestamp = esp_timer_get_time(); //us timer
    InterruptLogArray[InterruptTimeArrayIndex].value = digitalRead(SENS1_PIN);
  }
  InterruptTimeArrayIndex++;

  if(1==digitalRead(SENS1_PIN)) { //rising
    TimeSens0Start = esp_timer_get_time(); //us timer
    StartArray[rising] = TimeSens0Start;
    if(rising < TIME_ARRAY_SIZE) rising++;
    else rising=0;
  }
  else { //falling
    TimeSens0Stop = esp_timer_get_time();
    StopArray[falling] = TimeSens0Stop;
    if(falling < TIME_ARRAY_SIZE) falling++;
    else falling=0;    
    CzyGruchaOpadla = true;
  }
}

static bool Sens4InProgress = false;
static bool Sens4Done = false;

void ARDUINO_ISR_ATTR isrSens4() {
  uint64_t now = esp_timer_get_time(); //us timer

  if (1==digitalRead(SENS4_PIN)) { //rising
    if (Sens4InProgress==false) TimeSens4Start = now; //us timer
    Sens4InProgress = true;
    Sens4Done = false;
  }
  else { //falling
    TimeSens4Stop = now;
    if ((TimeSens4Stop - TimeSens4Start) < 500) { //debounce 500us
      TimeSens4Stop = 0;
    }
    else {
      Sens4InProgress = false;
      Sens4Done = true;
    }
  }
}

uint8_t GruchaOtwarta(void)
{
  if(0==digitalRead(SENS0_PIN)) {
    delay(2);
    if(0==digitalRead(SENS0_PIN)) return 1;
  }
  if(0==digitalRead(SENS2_PIN)) {
    delay(2);
    if(0==digitalRead(SENS2_PIN)) return 2;
  }

  return 0;
}
volatile int64_t TimeSens2Start = 0;
volatile int64_t TimeSens2Stop = 0;


volatile uint32_t IsrCoin = 0;

void ARDUINO_ISR_ATTR isrInt() {
  IsrIntCnt++;  
  timestampisrInt = millis();
  BaseType_t xYieldRequired;

     // Resume the suspended task.
     xYieldRequired = xTaskResumeFromISR( TaskPcfHandle );

     // We should switch context so the ISR returns to a different task.
     // NOTE:  How this is done depends on the port you are using.  Check
     // the documentation and examples for your port.
     portYIELD_FROM_ISR( xYieldRequired );
}


void ARDUINO_ISR_ATTR isrSens3() {
  if(InterruptTimeArrayIndex<TIME_ARRAY_SIZE) {
    InterruptLogArray[InterruptTimeArrayIndex].timestamp = esp_timer_get_time(); //us timer
    InterruptLogArray[InterruptTimeArrayIndex].value = digitalRead(SENS3_PIN);
  }
  InterruptTimeArrayIndex++;

  if(1==digitalRead(SENS3_PIN)) { //rising
    TimeSens2Start = esp_timer_get_time();
    StartArray[rising] = TimeSens2Start;
    if(rising < TIME_ARRAY_SIZE) rising++;
    else rising=0;
  }
  else { //falling
    TimeSens2Stop = esp_timer_get_time();
    StopArray[falling] = TimeSens2Stop;
    if(falling < TIME_ARRAY_SIZE) falling++;
    else falling=0;    
    CzyGruchaOpadla = true;
  }
}

int cnt = 0;
uint8_t songNo = 3;
bool migacz = false;
typedef enum state {
  CHOINKA,
  PREZENTACJA,
  POWITANIE,
  RESTART,
  GAME_START,
  POMIAR,
  TEST,
  GAME_END,
  SETUP_MENU,
  ERROR_STATE
} state_t;

state_t stan = CHOINKA;

enum GameMode_t {
  BOXER,
  KOPACZ,
  HAMMER
};

GameMode_t GameMode = BOXER;
uint16_t song = 1231;
uint16_t Wynik = 0;



void StroboskopRekord(int iterate) {
  printf("\n StroboskopRekord");
        for(int i=0;i<iterate;i++) {
          Zar.All = 0xFFFF;
          DisplayWynik(0xFFFF);
          DisplayPlayer(CurrentPlayer, 0xFFFF);
          DisplayRekord(0xFFFF);
          UpdateSPI();
          for(int i=0;i<num_leds1;i++) leds1[i] = CRGB::Gold;
          for(int i=0;i<num_leds2;i++) leds2[i] = CRGB::Black;
          fill_solid( leds3, num_leds3, CRGB(70,70,70));    
          Zar.Z14_HALOGEN1 = 0;
          Zar.Z15_HALOGEN2 = 1;
          delay(60);
  
          Zar.All = 0;
          DisplayWynik(Wynik);          
          DisplayPlayer(CurrentPlayer, Wynik);
          DisplayRekord(Fram.Record);          
          UpdateSPI();
          for(int i=0;i<num_leds1;i++) leds1[i] = CRGB::Black;
          for(int i=0;i<num_leds2;i++) leds2[i] = CRGB::Gold;
          fill_solid( leds3, num_leds3, CRGB(0,0,0));    
          Zar.Z14_HALOGEN1 = 1;
          Zar.Z15_HALOGEN2 = 0;
          delay(60);
        }
}

void Stroboskop(int iterate) {
  printf("\n Stroboskop");
        for(int i=0;i<iterate;i++) {
          Zar.All = 0xFFFF;
          DisplayWynik(888);
          DisplayRekord(888);
          UpdateSPI();
          for(int i=0;i<num_leds1;i++) leds1[i] = CRGB::White;
          for(int i=0;i<num_leds2;i++) leds2[i] = CRGB::White;
          //fill_solid( leds3, num_leds3, CRGB(100,100,100));    
          if(i%2) for(int i = 0; i<num_leds3 ; i+=3) leds3[i] = CRGB(70,70,70);
          else for(int i = 1; i<num_leds3 ; i+=3) leds3[i] = CRGB(70,70,70);
          Zar.Z14_HALOGEN1 = 1;
          Zar.Z15_HALOGEN2 = 1;
          delay(60);
  
          Zar.All = 0;
          DisplayWynik(0xFFFF);
          DisplayRekord(0xFFFF);
          UpdateSPI();
          for(int i=0;i<num_leds1;i++) leds1[i] = CRGB::Black;
          for(int i=0;i<num_leds2;i++) leds2[i] = CRGB::Black;
          fill_solid( leds3, num_leds3, CRGB(0,0,0));    
          Zar.Z14_HALOGEN1 = 0;
          Zar.Z15_HALOGEN2 = 0;
          delay(60);
          if(KEY_START1) {
            QuickStartBoxer = true;
            break;
          }
          else if(KEY_START2) {
            QuickStartKopacz = true;
            break;
          }
          else if(KEY_START_HAMMER && (Fram.BoxerModel == MONSTER)) {
            QuickStartHammer = true;
            break;
          }
        }
        DisplayRekord(Fram.Record);
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
            
        }
        file = root.openNextFile();
    }
}

void printIPAddressOfHost(const char* host) {
  IPAddress resolvedIP;
  if (!WiFi.hostByName(host, resolvedIP)) {
    Serial.println("DNS lookup failed.  Rebooting...");
    Serial.flush();
    //ESP.reset();
  }
  Serial.print(host);
  Serial.print(" IP: ");
  Serial.println(resolvedIP);
}

void DnsCallback(const char *name, const ip_addr_t *ipaddr, void * args ) {
  Serial.print("DnsCallback server name: ");
  Serial.println(inet_ntoa(name));
  Serial.print("IP: ");
  Serial.println(inet_ntoa(ipaddr));

  Serial.print("SysLogSrv: ");
  Serial.println(inet_ntoa(SysLogSrv));
}

void FramFlashDefault(void) {
    Fram.Credit = 0;
    Fram.InterwalPrezentacjiMinuty = 5;
    Fram.Language = 0; //0 - English
    Fram.Syslog = 0; //disabled
    Fram.SkalaSily = 100; //Skala sily 100%
    Fram.SkalaSilyKicker = 100; //Skala sily 100%
    Fram.SkalaSilyMlot = 100; //Skala sily 100%
    Fram.Mp3Volume = 12;
    Fram.FreePlay = 0;
    Fram.LedRgb = 0;
    Fram.PcbRev = 55;
    Fram.Record = 600;
    Fram.TestCredit = 0;
    Fram.PulseTime = 100;
    Fram.BoxerMat = 0;
    Fram.Boxer3Player = 0;
    for (int i=0;i<10;i++) {
        Fram.CoinBankPuls[i] = 0;
        Fram.CoinBankCred[i] = 0;
    }
}

void FramLoad(void)
{
  Serial.print("FramLoad ");
  ee.readBlock(FRAM_DATA_STRUCT, (uint8_t*)&Fram, sizeof(Fram));

  if ((Fram.PcbRev < 1) || (Fram.PcbRev >= 0xFF))  {
    FramFlashDefault();
  }
  //Fram.PcbRev = 55;

  Serial.print("Kredyt = ");Serial.print(Fram.Credit);
  if ((Fram.Credit > 99) || (Fram.Credit < 0)) Fram.Credit = 0;
  //Fram.Credit = 0;

  Serial.print("TotalCounter = ");Serial.print(Fram.TotalCounter);
  Serial.print("TempCounter = ");Serial.print(Fram.TempCounter);
  
  if ((Fram.Record>999) ||(Fram.Record<100)) Fram.Record = 600;
  if ((Fram.RecordBoxer>999) ||(Fram.RecordBoxer<100)) Fram.RecordBoxer = 600;
  if ((Fram.RecordKicker>999) ||(Fram.RecordKicker<100)) Fram.RecordKicker = 600;
  if ((Fram.RecordHammer>999) ||(Fram.RecordHammer<100)) Fram.RecordHammer = 600;
  if ((Fram.RecordTriathlon>9999) ||(Fram.RecordTriathlon<100)) Fram.RecordTriathlon = 2000;
  
  Fram.RecordBoxer = 333;
  Fram.RecordKicker = 444;
  Fram.RecordHammer = 555;
  Fram.RecordTriathlon = 2222;

  Serial.print("Record = "); Serial.print(Fram.Record);

  if ((Fram.SkalaSily > 120) || (Fram.SkalaSily < 80))  Fram.SkalaSily = 100;
  if ((Fram.SkalaSilyKicker > 120) || (Fram.SkalaSilyKicker < 80)) Fram.SkalaSilyKicker = 100;
  if ((Fram.SkalaSilyMlot > 150) || (Fram.SkalaSilyMlot < 50)) Fram.SkalaSilyMlot = 100;

  if(Fram.Mp3Volume>30) Fram.Mp3Volume = 12;

  if(Fram.FreePlay>1) Fram.FreePlay = 0;

  if(Fram.Language > 11) Fram.Language = 0; //TODO replace with MAX_LANGUAGE
  Serial.print("Language = ");Serial.print(Fram.Language);

  if ((Fram.InterwalPrezentacjiMinuty < 5) || (Fram.InterwalPrezentacjiMinuty > 240)) Fram.InterwalPrezentacjiMinuty = 5;

  if(Fram.Syslog > 1) Fram.Syslog = 0;
  Serial.print("Fram.Syslog = "); Serial.print(Fram.Syslog);

  if(Fram.LedRgb > 1) Fram.LedRgb = 0;
  Serial.print("Fram.LedRgb = "); Serial.print(Fram.LedRgb);

  if ((Fram.PulseTime < 50) || (Fram.PulseTime > 250)) Fram.PulseTime = 100;
  Serial.print(" Fram.PulseTime = "); Serial.print(Fram.PulseTime);

  if (Fram.GiftCntMale > Fram.GiftWinRatioMale) Fram.GiftCntMale = Fram.GiftWinRatioMale;
  if (Fram.GiftCntSrednie > Fram.GiftWinRatioSrednie) Fram.GiftCntSrednie = Fram.GiftWinRatioSrednie;
  if (Fram.GiftCntDuze > Fram.GiftWinRatioDuze) Fram.GiftCntDuze = Fram.GiftWinRatioDuze;

  if (Fram.BoxerMat > 1) Fram.BoxerMat = 0;
  if (Fram.Boxer3Player > 1) Fram.Boxer3Player = 0;

  if (Fram.Kids > 10) Fram.Kids = 0;

  Fram.RecordBoxer = 333;
  Fram.RecordKicker = 444;
  Fram.RecordHammer = 555;

}



#include "lwip/inet.h"
#include "lwip/dns.h"

void setup() {
 /* @note Please do not use the interrupt of GPIO36 and GPIO39 when using ADC.
 * @note Please do not use the interrupt of GPIO36 and GPIO39 when using ADC or Wi-Fi with sleep mode enabled.
 *       Please refer to the comments of `adc1_get_raw`.
 *       Please refer to section 3.11 of 'ECO_and_Workarounds_for_Bugs_in_ESP32' for the description of this issue.
 *       As a workaround, call adc_power_acquire() in the app. This will result in higher power consumption (by ~1mA),
 *       but will remove the glitches on GPIO36 and GPIO39.*/
  //adc_power_acquire();

  esp_timer_get_time();
  esp_timer_init();
  // --------======== SERIAL UART ========---------  
  Serial.setRxBufferSize(4096);
  Serial.begin(1000000);  
  Serial.println("Booting...");  

  //---------======== FRAM =======----------
  Wire.begin(); // Wire communication begin
  Wire.setClock(100000);
  ee.begin();
  if (ee.isConnected()) Serial.println("FRAM OK");
  else Serial.println("ERROR FRAM");
  uint32_t eeSize = ee.determineSize();
  printf("\n eeSize=%d", eeSize);
  ee.setDeviceSize(eeSize);

  Incarnation = ee.readByte(FRAM_B_INCARNATION);
  Serial.print("Restarted "); Serial.print(Incarnation); Serial.println(" times");
  Incarnation++;
  ee.writeByte(FRAM_B_INCARNATION, Incarnation);

  randomSeed(millis());
  randomSeed(Incarnation);
  FramLoad();

  // --------======== SPI ========---------  
  Serial.println("SPI");
  SpiInit();
  DISABLE_BANK_COIN_NAY;
  UpdateSPI();

  // --------======== OTA ========---------
  Serial.println("OTA");
  // OtaInit();

  if (Fram.Syslog == 1)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.println("Connecting Wifi...");
    for (int i = 0; i < 30; i++)
    {
        delay(500);
        Serial.print(".");
        if(WiFi.status() == WL_CONNECTED) break;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
            Serial.println("");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            const ip_addr_t *dnssrv = dns_getserver(0);
            Serial.print("DNS server: ");
            Serial.println(inet_ntoa(dnssrv));
            dns_gethostbyname("erwinek.ddns.net", &SysLogSrv, DnsCallback, 0);

            uint64_t chipid;
            chipid = ESP.getEfuseMac();                                                               // The chip ID is essentially its MAC address(length: 6 bytes).
            snprintf(ChipIdString, 19, "PGM52_%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid); // print High 2 bytes, print Low 4bytes.
            printf(ChipIdString);
            Serial.print("ChipIdString: ");
            Serial.println(ChipIdString);

            char serverName[] = "erwinek.ddns.net";
            IPAddress syslogServerIP;
            WiFi.hostByName(serverName, syslogServerIP);
            Serial.print("DNS server: ");
            Serial.println(inet_ntoa(syslogServerIP));

            pSysLog = new SimpleSyslog(ChipIdString, VERSION, "erwinek.ddns.net", 514);
            syslog("Connected to WiFi!");
    }
  }
  
  //---------======== PCF =======----------
  Serial.println("PCF");
  PcfInit();

  myLcd.init();
  myLcd.setCursor(0,0);
  myLcd.print("--==ProGames==--");
  myLcd.setCursor(0,1);
  //String StrMode[6] = {"HIT", "HK", "HG", "COM", "CK", "HKG"};
  char StrModel[10] = "HIT";
  if (Fram.BoxerModel==HIT)  strncpy(StrModel, "HIT", 3);
  else if (Fram.BoxerModel==KIDS)  strncpy(StrModel, "HK", 3);
  else if (Fram.BoxerModel==GIFT)  strncpy(StrModel, "HG", 3);
  else if (Fram.BoxerModel==COMBAT)  strncpy(StrModel, "COM", 3);
  else if (Fram.BoxerModel==COMBAT_KIDS)  strncpy(StrModel, "CK", 3);
  else if (Fram.BoxerModel==DOUBLE_HIT_GIFT_KIDS)  strncpy(StrModel, "HKG", 3);
  else if (Fram.BoxerModel==GIFT_ESPANIOL)  strncpy(StrModel, "HGE", 3);
  else if (Fram.BoxerModel==FIST)  strncpy(StrModel, "FST", 3);
  else if (Fram.BoxerModel==DOUBLE_HIT_GIFT_KIDS_ESPANIOL)  strncpy(StrModel, "HKGE", 4);

  myLcd.print(StrModel);
  myLcd.print(" ");
  myLcd.print(VERSION);

  // --------------=========== SD CARD ===========------------
  Serial.println("SD_CARD");
  gpio_pullup_en(GPIO_NUM_12);//hardware pull-up removed so enable internal
  gpio_pullup_en(GPIO_NUM_2);//hardware pull-up removed so enable internal
  bool ret;

  for(int i=0;i<10;i++) {
    ret = SD_MMC.begin("/sdcard", true, false, 60000000);
    //bool setPins(int clk, int cmd, int d0);
    //bool setPins(int clk, int cmd, int d0, int d1, int d2, int d3);
    //bool begin(const char * mountpoint="/sdcard", bool mode1bit=false, bool format_if_mount_failed=false, int sdmmc_frequency=BOARD_MAX_SDMMC_FREQ, uint8_t maxOpenFiles = 5);
    if(ret) break;
  }
  printf("\n BOARD_MAX_SDMMC_FREQ = %ll", BOARD_MAX_SDMMC_FREQ);
  if(!ret)
  {
      Serial.println("Card Mount Failed");
      myLcd.setCursor(0,0);
      myLcd.print("---SD ERROR---");
      stan = ERROR_STATE;
  }
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
 
  Serial.print("Total bytes: ");
  Serial.println(SD_MMC.totalBytes());
 
  Serial.print("Used bytes: ");
  Serial.println(SD_MMC.usedBytes());
  listDir(SD_MMC, "/", 0);
  //listDir(SD_MMC, "/mp3", 0);
  
  //---------======== DAC MP3 =======----------
  Serial.println("DAC MP3");
  AudioInit();

  //---------======== SENSORS =======----------
  pinMode(SENS0_PIN, INPUT);
  pinMode(SENS1_PIN, INPUT);
  attachInterrupt(SENS1_PIN, isrSens1, CHANGE);

  pinMode(SENS2_PIN, INPUT);
  pinMode(SENS3_PIN, INPUT);
  attachInterrupt(SENS3_PIN, isrSens3 , CHANGE);

  if (Fram.BoxerModel==MONSTER) {
    pinMode(SENS4_PIN, INPUT);
    attachInterrupt(SENS4_PIN, isrSens4 , CHANGE);
  }

  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(INT_PIN, isrInt , FALLING);

  LedInit();

  if(Fram.LedRgb==1) {
    FastLED.addLeds<WS2812B, 32, RBG>(leds1, num_leds1); 
    FastLED.addLeds<WS2812B, 16, RBG>(leds2, num_leds2); 
    FastLED.addLeds<WS2812B, 17, RGB>(leds3, num_leds3); 
  }
  else {
    FastLED.addLeds<WS2812B, 32, RGB>(leds1, num_leds1); 
    FastLED.addLeds<WS2812B, 16, RGB>(leds2, num_leds2); 
    FastLED.addLeds<WS2812B, 17, RGB>(leds3, num_leds3); 
  }

  

  ENABLE_BANK_COIN_NAY; //enable Coin and Bank

  Serial.println("Setup complete");

  DisplayPlayer(1, 0xFFFF);
  DisplayPlayer(2, 0xFFFF);
  DisplayPlayer(3, 0xFFFF);

  if (Fram.BoxerModel==MONSTER) {
    matrix.begin(1000000, true);
  
    matrix.clearScreen(1);
    matrix.clearScreen(2);

    matrix.setBrightness(50, 1);
    matrix.setBrightness(50, 2);
    
    matrix.displayText("ProGames", 0, 0, 2, 255, 255, 255, "fonts/ComicNeue-Bold-48.bdf", 1);
    matrix.displayText("ProGames", 0, 0, 2, 255, 255, 255, "fonts/ComicNeue-Bold-48.bdf", 2, 2);
  }
}


void Choinka() 
{
  static uint64_t timestamp = 0;
  static uint64_t timerDisplay = millis();
  static uint64_t timerDisplay3Player = millis();
  static uint8_t colourCnt = 0;
  static uint8_t LedEffectCnt = 0;
  static uint64_t LedEffectTimer = millis()/1000;
  static uint8_t starthue = 0;
  static uint8_t LedLicznik = 0;

  if ((millis()/1000 - LedEffectTimer) > 10) {
    LedEffectTimer = millis()/1000;
    if (LedEffectCnt < 3) LedEffectCnt++;
    else LedEffectCnt = 0;
  }

  QuickStartBoxer = false;
  QuickStartKopacz = false;
  QuickStartHammer = false;
  
  matrix.displayText("*$* Insert Coin *$*", 10, 170, 2, 255, 255, 0, "fonts/9x18B.bdf", 1, 500);
  matrix.displayText("PRO-GAMES POLAND", 25, 5, 2, 255, 255, 0, "fonts/9x18B.bdf", 2, 0);
  matrix.displayText("* Monster 3in1 *", 40, 25, 2, 255, 255, 0, "fonts/7x13.bdf", 3, 0);

  matrix.displayText("ProGames", 5, 300, 2, 255, 255, 255, "fonts/7x13.bdf", 31, 0, 2);
  matrix.displayText("Poland", 12, 315, 2, 255, 255, 255, "fonts/7x13.bdf", 32, 0, 2);
  matrix.displayText("Insert", 10, 390, 2, 255, 255, 0, "fonts/7x13.bdf", 33, 400, 2);
  matrix.displayText("Coin", 20, 400, 2, 255, 255, 0, "fonts/7x13.bdf", 34, 400, 2);

  switch(LedEffectCnt) {
    case 0:
      //matrix boxer
      matrix.loadGif("anim/boxer.gif", 36, 36+2, 192-(36*2), 192-(36*2), 0, 1);
      
      
      //matrix hammer
      matrix.loadGif("anim/6h.gif", 0, 512-64, 64, 64, 10, 2);
      matrix.loadGif("anim/7.gif", 0, 0, 64, 64, 11, 2);
  
      if(Fram.BoxerMat==1) {
        fill_solid( leds1, num_leds1, CRGB::Black); 
        for(int i=LedLicznik%2;i<num_leds1;i+=2) leds1[i] = CRGB::White;
        
        fill_solid( leds2, num_leds2, CRGB::Black); 
        for(int i=LedLicznik%2;i<num_leds2;i+=2) leds2[i] = CRGB::White;

        fill_solid( leds3, num_leds3, CRGB::Black); 
        for(int i=LedLicznik%2;i<num_leds3;i+=2) leds3[i] = CRGB::White;        
      }
      else {
        fill_rainbow( leds1, num_leds1, starthue, 20); 
        fill_rainbow( leds2, num_leds2, starthue, 20); 
        if(Fram.BoxerModel==FIST) {
          fill_rainbow( leds3, num_leds3, starthue, 5); 
          fadeLightBy(leds3, num_leds3, 50);
          starthue+=5;
        }
        else {
          fill_rainbow( leds3, num_leds3, starthue, 20); 
          starthue+=3;
        }
        
      }
        DisplayRekord(Fram.Record);
    break;
    case 2:
    
      matrix.loadGif("anim/7t4e-resize.gif", 0, 200, 64, 64, 11, 2);
      matrix.loadGif("anim/2.gif", 0, 0, 192, 192, 0);
      if(Fram.BoxerMat==1) {
        LedNightRiderWhite(leds1, num_leds1);
        LedNightRiderWhite(leds2, num_leds2);
        LedNightRiderWhite(leds3, num_leds3);
      }
      else {
        NightRiderRGB(leds1, num_leds1);
        LedRunThree(&leds2[LiczbaLedFrontLeft], LiczbaLedCzacha );
        effect_juggle( &leds2[LiczbaLedFrontLeft+LiczbaLedCzacha+LiczbaLedFrontRight], LiczbaLedBramka);
        if(LiczbaLedFrontLeft>0) fadeInOutThree(&leds2[0], LiczbaLedFrontLeft );
        if(LiczbaLedFrontRight>0) fadeInOutThree(&leds2[LiczbaLedFrontLeft+LiczbaLedCzacha], LiczbaLedFrontRight );
        dot_beat16( leds3, num_leds3 );
      }
      if(migacz) DisplayRekord(Fram.Record);
      else DisplayRekord(0xFFFF);
    break;
    case 1:
      matrix.loadGif("anim/1.gif", 0, 200, 64, 64, 11, 2);      
      matrix.loadGif("anim/3.gif", 0, 0, 192, 192, 0);
      if(Fram.BoxerMat==1) {        
        fadeInOutWhite( leds1, num_leds1 );
        fadeInOutWhite( leds2, num_leds2 );
        fadeInOutWhite( leds3, num_leds3 );
      }
      else {
        fadeInOutThree( leds1, num_leds1 );
        fadeInOutThree( leds2, num_leds2 );
        fadeInOutThree( leds3, num_leds3 );
      }
    break;
    case 3:
      matrix.loadGif("anim/4.gif", 0, 0, 192, 192, 0);
      matrix.loadGif("anim/gwiazdki.gif", 0, 0, 64, 512, 10, 2);      
      fadeInOutThree( leds1, num_leds1 );
      StroboScope(leds2, num_leds2, CHSV(starthue, 255, 255));
      StroboScope1on3(leds3, num_leds3, CHSV(starthue, 255, 255));
      starthue += 13;
    break;
    DisplayRekord(Fram.Record);
    break;
  }
  
  if(millis() - timestamp > 300) {
    timestamp = millis();  
    LedLicznik++;
    migacz =!migacz;
    if(migacz) Zar.All = 0x00000555;
    else Zar.All = 0x00000aaa;
    if(migacz) {
      Zar.Z22_SKRZYNIA = 0;
      Zar.Z13_FIST = 1;
      Zar.Z12_START_3PLAYER = 1;
    }
    else {
      Zar.Z22_SKRZYNIA = 1;
      Zar.Z13_FIST = 0;
      Zar.Z12_START_3PLAYER = 0;
    }
  }

  switch(LedLicznik%3) {
    case 0:
      Zar.Z19_LED1 = 1;
      Zar.Z20_LED2 = 0;
      Zar.Z21_LED3 = 0;
    break;
    case 1:
      Zar.Z19_LED1 = 0;
      Zar.Z20_LED2 = 1;
      Zar.Z21_LED3 = 0;
    break;
    case 2:
      Zar.Z19_LED1 = 0;
      Zar.Z20_LED2 = 0;
      Zar.Z21_LED3 = 1;
    break;
  }

  if(millis() - timerDisplay > 120) {
    timerDisplay = millis();
    DisplayWynikEffect(0); 
  }  
  if(millis() - timerDisplay3Player > 500) {
    timerDisplay3Player = millis();
    Display3PlayerEffect(1, 1);
    Display3PlayerEffect(2, 1);
    Display3PlayerEffect(3, 1);
  }  
  
  if (KEY_START1 || KEY_START2 || Sens4Done) {
    Sens4Done = 0;
    printf("\n First Inser the coin");
    static uint64_t TimerFirstInsertTheCoin = 0;
    if ((TimerFirstInsertTheCoin==0) || ((millis() - TimerFirstInsertTheCoin) > 2000)) {
      TimerFirstInsertTheCoin = millis();
      PlayRandomLangMp3(wcPlayWrzucMonete);       // wrzuc monetê
      Mp3AddSong(302);
    }
  }
}

bool Prezentacja()
{
   static uint8_t starthue = 0;
   static uint64_t timeout;
   static uint64_t timestamp;
   static uint64_t timerDisplay = 0;
   static uint8_t songNo = 105;
   static bool FirstEntry = true;
   static uint16_t endSong = 0;
   if(FirstEntry) {
     timeout = millis();
     FirstEntry = false;
     if(songNo < 109) songNo++;
     else songNo = 101;
     PlayMp3(songNo);    
   }

  
  //fill_rainbow( leds1, num_leds1, starthue, 10);
  fill_rainbow( leds2, num_leds2, starthue, 20);
  fill_rainbow( leds3, num_leds3, starthue, 30);
  starthue++;

  CRGB colourArray[] = {CRGB::Yellow, CRGB::Orange, CRGB::Blue, CRGB::Green, CRGB::Red};
  static uint8_t blenCnt = 0;
  static uint8_t colourCnt = 0;
  
  CRGB blended = blend(colourArray[colourCnt%5], colourArray[(colourCnt+1)%5], blenCnt);  // fade in
  blenCnt += 5;
  if(blenCnt>250) {
    colourCnt++;
    blenCnt = 0;
  }
  fill_solid(leds1, num_leds1, blended);

  if(millis() - timestamp > 300) {
     timestamp = millis();  
     migacz =!migacz;
     if(migacz) Zar.All = 0xFFFF;
     else Zar.All = 0x0000;

     if (((millis() - timeout) / 1000) > 5 * 60) { 
      FirstEntry = true;
      return false;
     }
  }
  if(millis() - timerDisplay > 150) {
    timerDisplay = millis();
    DisplayWynikEffect(1);
    DisplayRekordEffect(1);
    Display3PlayerEffect(1, 0);
    Display3PlayerEffect(2, 0);
    Display3PlayerEffect(3, 0);
  }

  if(!Mp3IsPlaying() && FirstEntry==false) {
    endSong++;   
  
    if(endSong > 50) {
      endSong = 0;
      FirstEntry = true;
      return false;
    }
  }
  else endSong = 0;

  if (KEY_START1 || KEY_START2) {
    static uint64_t TimerFirstInsertTheCoin = millis();
    if ((millis() - TimerFirstInsertTheCoin) > 5000) {
      TimerFirstInsertTheCoin = millis();
      PlayRandomLangMp3(wcPlayWrzucMonete);       // wrzuc monetê
      Mp3AddSong(302);
    }
  }

  return true;
}

void BlinkStart() {
  static uint64_t timestamp = millis();
  static bool state = true;
  if(millis() - timestamp > 300) {
    timestamp = millis();
    state = !state;
    if(state==true) {
      Zar.Z10_START_BOXER = 1;
      Zar.Z11_START_KOPACZ = 1;
    }    
    else {
      Zar.Z10_START_BOXER = 0;
      Zar.Z11_START_KOPACZ = 0;
    }
  }
}
static bool firstEntry = true;
static bool firstEntryGameStart = true;

bool GameStart()
{  
  bool ret = false;
  static int timeout = 0;
  static uint64_t TimerGruszkaOpada = 0;

  cntOtwartaBoxer = 0;
  cntOtwartaKopacz = 0;
  cntGruchaZamknieta = 0;

  if (firstEntryGameStart==true) {
    matrix.clearScreen(1);
    matrix.clearScreen(2);

    delay(1);    
    firstEntryGameStart = false;    
  }

  DisplayGameMatrix();
  matrix.displayText("Press Start!", 30, 150, 2, 255, 255, 0, "fonts/9x18B.bdf", 7, 500);
  matrix.displayText("Select Boxer Kicker Hammer", 5, 170, 2, 255, 255, 0, "fonts/7x13.bdf", 8, 500);

  matrix.displayText("Start", 15, 60, 2, 255, 255, 0, "fonts/7x13.bdf", 37, 300, 2);
  matrix.loadGif("anim/palec-resize.gif", 0, 72, 64, 64, 10, 2);
  matrix.displayText("Boxer", 15, 72+36, 2, 255, 255, 0, "fonts/7x13.bdf", 38, 300, 2);
  
  matrix.displayText("Start", 15, 318, 2, 255, 255, 0, "fonts/7x13.bdf", 39, 300, 2);  
  matrix.loadGif("anim/palec-resize.gif", 0, 330, 64, 64, 11, 2);
  matrix.displayText("Kicker", 15, 330+36, 2, 255, 255, 0, "fonts/7x13.bdf", 40, 300, 2);  

  matrix.displayText("Start", 15, 428-12, 2, 255, 255, 0, "fonts/7x13.bdf", 41, 300, 2);  
  matrix.displayText("Hammer", 15, 428, 2, 255, 255, 0, "fonts/7x13.bdf", 42, 300, 2);  
  matrix.loadGif("anim/palec_dol.gif", 15, 440, 44, 64, 12, 2);
  
  delay(1);
  
  Zar.Z9_Logo = 1;

  Zar.Z19_LED1 = 1;
  Zar.Z20_LED2 = 1;
  Zar.Z21_LED3 = 1;
  Zar.Z22_SKRZYNIA = 1;
  if (Fram.BoxerModel==FIST) {
    Zar.Z8 = 1;
    static uint64_t timestmp = millis();
    static uint8_t cnt = 0;
    if(millis()-timestmp > 333) {
      timestmp = millis();
      cnt++;
      Zar.All &= ~0x0000000F; //clear Z1,Z2,Z3
      Zar.All |= cnt & 0x0000000F; //set Z1,Z2,Z3
    }
  }
  if (Fram.BoxerModel==MONSTER) {
    Zar.Z9_Logo = 0;
    static uint64_t timestmp = millis();
    static bool blink=true;
    if(millis()-timestmp > 333) {
      timestmp = millis();
      blink = !blink;
      Zar.Z13_FIST = !blink;
      Zar.Z12_START_3PLAYER = blink;
    }
 }

  if(!Mp3IsPlaying()) {
    DisplayRekord(Fram.Record);
    if(firstEntry==true) {
      firstEntry = false;
      TimeSens0Start = 0;
      TimeSens0Stop = 0;
      TimeSens2Start = 0;
      TimeSens2Stop = 0;
      TimeSens4Start = 0;
      TimeSens4Stop = 0;
      rising = 0;
      falling = 0;

      if(TournamentInProgress==false) {
        for(int i=1;i<=3;i++) DisplayPlayer(i, 0xFFFF);
        for(int i=1;i<=3;i++) TournamentScores[i] = 0;
        if (Fram.Boxer3Player==1 && Fram.Credit > 3) PlayMp3(Mp3SelectNumberOfPlayers[0][Fram.Language]);
        else Mp3AddSong(wcPlayGameStart[0][Fram.Language]);
      }
    }
    else {
      if (GameMode == BOXER) {
        PlayRandomMp3(MP3_GameStart);
      }
      else if (GameMode == KOPACZ) {
        PlayRandomMp3(MP3_GameStartKopacz);
      }
      else if (GameMode == HAMMER) {
        PlayRandomMp3(MP3_GameStartKopacz);
      }
    }
  }

  LedGameStart();

  BlinkStart();

  if(KEY_START1 || QuickStartBoxer) {
    QuickStartBoxer = false;
    printf("\n KEY_START1");
    GameMode = BOXER;
    if((millis() - timeout) > ElektromagnetTimeout) {
      if (ElektromagnetTimeout < 60000) ElektromagnetTimeout += 2000;
      timeout = millis();
      for(int i=0;i<3;i++) {
        CLR_GRUSZKA;
        vTaskDelay(10);
        delay(110);        
        SET_GRUSZKA;
        for(int i=0;i<100;i++) {
          delay(5);
          if(GruchaOtwarta()) break;
        }
        if(GruchaOtwarta()) {
          TimerGruszkaOpada = millis();
          break;
        }
      }
    }
  }
  
  if(KEY_START2 || QuickStartKopacz) {
    QuickStartKopacz = false;
    printf("\n KEY_START2");
    GameMode = KOPACZ;
    if((millis() - timeout) > ElektromagnetTimeout) {
      if (ElektromagnetTimeout < 60000) ElektromagnetTimeout += 2000;
      timeout = millis();
      for(int i=0;i<3;i++) {
        CLR_GRUSZKA2;
        vTaskDelay(10);
        delay(140);        
        SET_GRUSZKA2;
        vTaskDelay(10);
        for(int i=0;i<100;i++) {
          delay(5);
          if(GruchaOtwarta()) break;
        }
        if(GruchaOtwarta()) {
          TimerGruszkaOpada = millis();
          break;
        }
      }
    }
  }

  if (Fram.BoxerModel == MONSTER && (KEY_START_HAMMER || QuickStartHammer)) {
    QuickStartHammer = false;
    printf("\n KEY_START_HAMMER");
    GameMode = HAMMER;
    ret = true;
    Sens4Done = 0;
    Wynik = 0;
    DisplayWynik(Wynik);
    DisplayPlayer(CurrentPlayer, Wynik);
  }

  DisplayRekord(Fram.Record);

  static int OtwartaSamoistnie = 0;
  if(GruchaOtwarta()) {
    ElektromagnetTimeout = 3000;
    
      TimeSens0Start = 0;
      TimeSens0Stop = 0;
      TimeSens2Start = 0;
      TimeSens2Stop = 0;
      TimeSens4Start = 0;
      TimeSens4Stop = 0;
      rising = 0;
      falling = 0;
      InterruptTimeArrayIndex = 0;
      delay(10);
      if(++OtwartaSamoistnie > 20) {
        printf("\n Gruszka OtwartaSamoistnie przeszla przez czujnik w %d ms", millis() - TimerGruszkaOpada);
        if (NumberOfPLayers > 1) {
          TournamentInProgress = true;
          if (TournamentScores[1] > 0) DisplayPlayer(1, TournamentScores[1]);
          if (TournamentScores[2] > 0) DisplayPlayer(2, TournamentScores[2]);
          else DisplayPlayer(2, 0xEEEE);
          if (TournamentScores[3] > 0) DisplayPlayer(3, TournamentScores[3]);      
          else if (NumberOfPLayers > 2) DisplayPlayer(3, 0xEEEE);
          else DisplayPlayer(3, 0xFFFF);
          if (1==CurrentPlayer) { PlayMp3(Mp3PlayerOne[0][Fram.Language]); DisplayPlayer(1, Wynik); }
          else if (2==CurrentPlayer) { PlayMp3(Mp3PlayerTwo[0][Fram.Language]); DisplayPlayer(2, Wynik); }
          else if (3==CurrentPlayer) { PlayMp3(Mp3PlayerThree[0][Fram.Language]); DisplayPlayer(3, Wynik); }
        }
        else if (GameMode==BOXER) PlayRandomMp3(wcPlayGong);
        else if (GameMode==KOPACZ) PlayRandomMp3(wcPlayGwizdek);
        else PlayRandomMp3(wcPlayGong); //hammer
        
        Zar.Z14_HALOGEN1 = 1;
        Zar.Z15_HALOGEN2 = 1;
        firstEntry = true;
        Wynik = 0;
        ret = true;
      }
  }
  else OtwartaSamoistnie = 0;

  if(CzyGruchaOpadla) {
    ElektromagnetTimeout = 3000;
    TimeSens0Start = 0;
    TimeSens0Stop = 0;
    TimeSens2Start = 0;
    TimeSens2Stop = 0;
    TimeSens4Start = 0;
    TimeSens4Stop = 0;
    rising = 0;
    falling = 0;
    InterruptTimeArrayIndex = 0;

    printf("\n Gruszka przeszla przez czujnik w %d ms", millis() - TimerGruszkaOpada);
    Wynik = 0;
    DisplayWynik(Wynik);
    DisplayPlayer(CurrentPlayer, Wynik);
    if (NumberOfPLayers > 1) {
          TournamentInProgress = true;
          if (TournamentScores[1] > 0) DisplayPlayer(1, TournamentScores[1]);
          if (TournamentScores[2] > 0) DisplayPlayer(2, TournamentScores[2]);
          else DisplayPlayer(2, 0xEEEE);
          if (TournamentScores[3] > 0) DisplayPlayer(3, TournamentScores[3]);      
          else if (NumberOfPLayers > 2) DisplayPlayer(3, 0xEEEE);
          else DisplayPlayer(3, 0xFFFF);
          if (1==CurrentPlayer) { PlayMp3(Mp3PlayerOne[0][Fram.Language]); DisplayPlayer(1, Wynik); }
          else if (2==CurrentPlayer) { PlayMp3(Mp3PlayerTwo[0][Fram.Language]); DisplayPlayer(2, Wynik); }
          else if (3==CurrentPlayer) { PlayMp3(Mp3PlayerThree[0][Fram.Language]); DisplayPlayer(3, Wynik); }
    }
    else if (GameMode==BOXER) PlayRandomMp3(wcPlayGong);
    else PlayRandomMp3(wcPlayGwizdek);
    ret = true;
    Zar.Z14_HALOGEN1 = 1;
    Zar.Z15_HALOGEN2 = 1;
  }
  DisplayWynik(Wynik);
  
  if (TournamentInProgress==true) {
    DisplayPlayer(CurrentPlayer, 0, true);
  }
  else {
    int i = 1;
    for(i=1;i<=NumberOfPLayers;i++) DisplayPlayer(i, i, true);
    for(i;i<=3;i++) DisplayPlayer(i, 0xFFFF);
  if ((Fram.Credit >= 2) && (KEY_3PLAYER) && (Fram.BoxerModel==FIST)) {
    if(NumberOfPLayers < Fram.Credit) NumberOfPLayers++;
    else NumberOfPLayers = 1;
    if (NumberOfPLayers > 3) NumberOfPLayers = 1;

    switch (NumberOfPLayers)
    {
    case 2:
      PlayMp3(Mp3TwoPlayers[0][Fram.Language]);
      break;

    case 3:
      PlayMp3(Mp3ThreePlayers[0][Fram.Language]);
      break;

    default:
      PlayMp3(Mp3OnePlayer[0][Fram.Language]);
      NumberOfPLayers = 1;
      break;
    };
    delay(300);
    }
  }

  
  
  vTaskResume( TaskPcfHandle );

  return ret;
}

uint32_t KonwersjaCzasuNasSile(int32_t sensorTime)
{
  uint32_t result = 0;
  if(sensorTime<0) sensorTime = -sensorTime;
  printf("\n sensorTime=%d", sensorTime);
  if (sensorTime < 500) return 111;  
  if (sensorTime > 30000) return 99; //oszust
  result = (30000 - sensorTime)/40;
  if(sensorTime<5000) result = (result * 14) / 10;
  else if(sensorTime<12000) result = (result * 12) / 10;
  else if(sensorTime<19000) result = (result * 11) / 10;  

  if(result<10) result = sensorTime%9;

  printf("\n result=%d", result);

  result = result * (10 + Fram.Kids) / 10;
  printf("\n resultKids=%d", result);

  //Skala Sily
  if(GameMode == BOXER) {
    result = (result * Fram.SkalaSily) / 100;
  }
  else if(GameMode == KOPACZ) {
    result = (result * Fram.SkalaSilyKicker) / 100;
  }
  else {
    printf("!!! GameMode ERROR !!!");
  }
  printf("\n result po korekcie=%d", result);
  
  if ((Fram.BoxerModel == GIFT) || (Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS) || (Fram.BoxerModel == GIFT_ESPANIOL) || (Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS_ESPANIOL)) {
    result = AlgorytmGift(result);
  } 

  if(result > 999) result = 999;

  if (TournamentInProgress) { //zabezpiecznie przeciw remisowi w turnieju 3 graczy
    for (int i = 1; i<CurrentPlayer; i++) {
      if (result == TournamentScores[i]) {
        result = result - 1;
      } 
    }
  }

  return result;
}

bool Naliczanie(uint16_t value)
{
  bool ret = 0;
  static uint16_t tempWynik = 0;
  if (tempWynik==0) PlayMp3(361);
  vTaskDelay(2);
  NightRiderRGB(leds1, num_leds1);
  circleLedStr2();
  UpdateCredit();

  matrix.displayText(String(tempWynik).c_str(), 192/2 - String(tempWynik).length()*10, 62, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf", 4, 0);
  matrix.displayText("SCORE", 10, 140, 2, 255, 255, 255, "fonts/9x18B.bdf", 32, 0, 2);
  matrix.displayText(String(tempWynik).c_str(), (64 - (String(tempWynik).length()*19))/2, 155, 2, 255, 255, 255, "fonts/Verdana-24-r.bdf", 33, 0, 2);
  matrix.loadGif("anim/naliczanieHam.gif", 0, 0, 64, 512, 10, 2);

  if (Fram.BoxerModel==MONSTER) {
    uint16_t liczbaDiodSkalaMlot = (tempWynik * 16)/999;
    for(int i=0;i<LiczbaLedMlotScianka;i++) leds2[i+LiczbaLedCzacha+LiczbaLedBramka+LiczbaLedMlot] = CRGB::Black; 
    for(int i=0;i<liczbaDiodSkalaMlot;i++) leds2[i+LiczbaLedCzacha+LiczbaLedBramka+LiczbaLedMlot] = CRGB::Yellow; 
  }

  if(tempWynik>999) {
    tempWynik = 0;
    Wynik = value;
    DisplayWynik(Wynik);
    DisplayPlayer(CurrentPlayer, Wynik);
    return true;
  }

  if (KEY_START1) {
    QuickStartBoxer = true;
    tempWynik = 0;
    Wynik = value;
    DisplayWynik(Wynik);
    DisplayPlayer(CurrentPlayer, Wynik);
    ret = true;
  }
  else if (KEY_START2) {
      QuickStartKopacz = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    } 
  
  if (tempWynik < value) { //naliczanie
    LedPercent((tempWynik*100)/999);

  if (Fram.BoxerModel==KIDS) {
    num_leds3 = NUM_LEDS3_KIDS;
  }
  else if (Fram.BoxerModel==COMBAT) {
    num_leds3 = NUM_LEDS3_COMBAT;
    num_leds2 = NUM_LEDS2_COMBAT;
    LiczbaLedCzacha = 14;
    LiczbaLedBoki = 11;
  }
  else if (Fram.BoxerModel==COMBAT_KIDS) {
    num_leds3 = NUM_LEDS3_COMBAT_KIDS;
    num_leds2 = NUM_LEDS2_COMBAT_KIDS;
    LiczbaLedCzacha = 14;
    LiczbaLedBoki = 9;
  }
  else if (Fram.BoxerModel==MONSTER) {
    LiczbaLedCzacha = 14;
    LiczbaLedBoki = 0;
  }
    if(LiczbaLedBoki>0) LedPercent2((tempWynik*100)/999);
    tempWynik++;
    if(tempWynik + 33 < value) tempWynik+=13;
    DisplayWynik(tempWynik);
    DisplayPlayer(CurrentPlayer, tempWynik);
    //vTaskDelay(2);
    if (KEY_START1) {
      QuickStartBoxer = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    }
    else if (KEY_START2) {
      QuickStartKopacz = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    }
    if(tempWynik + 10 > value) delay(100);
    if (KEY_START1) {
      QuickStartBoxer = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    }
    else if (KEY_START2) {
      QuickStartKopacz = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    }
    if(tempWynik + 3 > value) delay(200);
    if (KEY_START1) {
      QuickStartBoxer = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    }
    else if (KEY_START2) {
      QuickStartKopacz = true;
      tempWynik = 0;
      Wynik = value;
      DisplayWynik(Wynik);
      DisplayPlayer(CurrentPlayer, Wynik);
      return true;
    }
  } 
  else {
    tempWynik = 0;
    ret = true;
    Wynik = value;
    DisplayWynik(Wynik);
    DisplayPlayer(CurrentPlayer, Wynik);
  }

  if(tempWynik > 100) Zar.Z0 = 1;
  if(tempWynik > 200) Zar.Z1 = 1;
  if(tempWynik > 300) Zar.Z2 = 1;
  if(tempWynik > 400) Zar.Z3 = 1;
  if(tempWynik > 500) Zar.Z4 = 1;
  if(tempWynik > 600) Zar.Z5 = 1;
  if(tempWynik > 700) Zar.Z6 = 1;
  if(tempWynik > 800) Zar.Z7 = 1;
  if(tempWynik > 900) Zar.Z8 = 1;
  if(tempWynik >= 999) Zar.Z9_Logo = 1;

  return ret;
}

void KomunikatyOsiagniec(uint16_t pomiar)
{
  if ((GameMode==BOXER) || (GameMode==HAMMER)) {
    printf("\n Komunikaty Osiagniec BOXER");

      if (pomiar<=299) {
        PlayRandomMp3AndWait(wcPlayStopien01);
        PlayRandomMp3AndWait(wcPlayStopien1);    
        PlayRandomLangMp3AndWait(wcPlayStopien1Language);
      }
      else if (pomiar<=599) {
        PlayRandomMp3AndWait(wcPlayStopien02);
        PlayRandomMp3AndWait(wcPlayStopien2);
        PlayRandomLangMp3AndWait(wcPlayStopien2Language);
      }
      else if (pomiar<=799) {
        PlayRandomMp3AndWait(wcPlayStopien03);
        PlayRandomMp3AndWait(wcPlayStopien3);
        PlayRandomLangMp3AndWait(wcPlayStopien3Language);
      }
      else {
        PlayRandomMp3AndWait(wcPlayStopien04);
        PlayRandomMp3AndWait(wcPlayStopien4);
        PlayRandomLangMp3AndWait(wcPlayStopien4Language);
      }
    }
  else {
    printf("\n Komunikaty Osiagniec KOPACZ");
    if (pomiar<=299) {
      PlayRandomMp3AndWait(wcPlayStopien01);
      PlayRandomMp3AndWait(wcPlayStopien1Kopacz); 
    }
    else if (pomiar<=599) {
      PlayRandomMp3AndWait(wcPlayStopien02);
      PlayRandomMp3AndWait(wcPlayStopien2Kopacz);
    }
    else if (pomiar<=799) {
      PlayRandomMp3AndWait(wcPlayStopien03);
      PlayRandomMp3AndWait(wcPlayStopien3Kopacz);
    }
    else {
      PlayRandomMp3AndWait(wcPlayStopien04);
      PlayRandomMp3AndWait(wcPlayStopien4Kopacz);
    }
  }
}

void DebugSensor(void)
{
  Serial.print("\n rising = ");
  Serial.println(rising);
  for (uint32_t i = 0; i < rising; i++)
    Serial.println(uintToStr(StartArray[i], str));
  Serial.print("\n falling = ");
  Serial.println(falling);
  for (uint32_t i = 0; i < falling; i++)
    Serial.println(uintToStr(StopArray[i], str));
  
  Serial.print("\n InterruptLogArray ");
  syslog("InterruptLogArray");
  for (uint32_t i = 0; i < InterruptTimeArrayIndex; i++)  {
    uintToStr(InterruptLogArray[i].timestamp, str);
    syslog(str);
    printf("\n %d ", InterruptLogArray[i].value);
    Serial.println(str);
  }
}

#define RED 0
#define GREEN 1

void Matrix_SetText1(char* text, bool force=false);
void Matrix_SetText2(char* text, bool force=false);

void Matrix_SetText1(char* text, bool force) {
  
}

void Matrix_SetText2(char* text, bool force) {
  
}


bool Pomiar()
{
  bool ret = false;
  static uint8_t starthue = 0;
  static uint32_t wynik = 0;
  firstEntryGameStart = true;

  Zar.Z9_Logo = 1;
  if(Fram.BoxerModel==MONSTER) {
    Zar.Z9_Logo = 0;
    Zar.Z13_FIST = 1;
  }

  if (wynik == 0)
  {
    static uint64_t timestamp = millis();
    if (millis() - timestamp > 100) {
      timestamp = millis();    
      if(GruchaOtwarta()==1) {
        GameMode = BOXER;
        matrix.displayText("Hit the punch!", 25, 160, 2, 255, 255, 0, "fonts/9x18B.bdf", 7, 300);
        matrix.displayText("Hit", 15, 250, 2, 255, 255, 255, "fonts/9x18B.bdf", 15, 300, 2);
        matrix.displayText("Punch", 10, 265, 2, 255, 255, 255, "fonts/9x18B.bdf", 16, 300, 2);
      }
      if(GruchaOtwarta()==2) {
        GameMode = KOPACZ;
        matrix.displayText("Kick the ball!", 25, 160, 2, 255, 255, 0, "fonts/9x18B.bdf", 7, 300);    
        matrix.displayText("Kick", 10, 250, 2, 255, 255, 255, "fonts/9x18B.bdf", 15, 300, 2);
        matrix.displayText("Ball", 10, 265, 2, 255, 255, 255, "fonts/9x18B.bdf", 16, 300, 2);
      }
      if (GameMode == HAMMER) {
        matrix.displayText("Use the hammer!", 25, 160, 2, 255, 255, 0, "fonts/9x18B.bdf", 7, 300);    
        matrix.displayText("Use", 15, 250, 2, 255, 255, 255, "fonts/9x18B.bdf", 15, 300, 2);
        matrix.displayText("Hammer", 10, 265, 2, 255, 255, 255, "fonts/9x18B.bdf", 16, 300, 2);
        matrix.loadGif("anim/2Vga-resize.gif", 0, 60, 64, 64, 10, 2);
        matrix.loadGif("anim/RedArrowDown.gif", 0, 512-73, 64, 73, 21, 2);      
      }
    }

    DisplayPlayer(CurrentPlayer, Wynik, true);
    DisplayGameMatrix();
    
    if (false == Mp3IsPlaying())
    {
      DisplayRekord(Fram.Record);
      DisplayWynik(Wynik);      

      if (GameMode == BOXER) {
        PlayRandomMp3(wcPlayPomiar);
      }
      else if (GameMode == KOPACZ) {
        PlayRandomMp3(wcPlayPomiarKicker);
      }
    }

    if (LiczbaLedFrontLeft>0) dot_beat(&leds2[0], LiczbaLedFrontLeft);
    if (LiczbaLedFrontRight>0) dot_beat(&leds2[LiczbaLedFrontLeft + LiczbaLedCzacha], LiczbaLedFrontRight);

    if (GameMode == BOXER)
    {
      fadeInOut(&leds2[LiczbaLedFrontLeft], LiczbaLedCzacha, GREEN); // green
      fill_solid(&leds2[LiczbaLedFrontLeft + LiczbaLedCzacha + LiczbaLedFrontLeft], LiczbaLedBramka, CRGB::Red);
      fill_solid(&leds2[LiczbaLedCzacha+LiczbaLedBramka], LiczbaLedMlot, CRGB::Red);
      fill_solid(&leds2[LiczbaLedCzacha+LiczbaLedBramka], LiczbaLedMlot, CRGB::Red);
      fill_rainbow(&leds2[LiczbaLedCzacha+LiczbaLedBramka+LiczbaLedMlot], LiczbaLedMlotScianka, starthue, 20);
    }
    else if (GameMode == KOPACZ)
    {
      fadeInOut(&leds2[LiczbaLedFrontLeft + LiczbaLedCzacha + LiczbaLedFrontRight], LiczbaLedBramka, GREEN); // green
      fill_solid(&leds2[LiczbaLedFrontLeft], LiczbaLedCzacha, CRGB::Red);
      fill_solid(&leds2[LiczbaLedFrontLeft + LiczbaLedCzacha + LiczbaLedFrontRight + LiczbaLedBramka], LiczbaLedMlot, CRGB::Red);
      fill_rainbow(&leds2[LiczbaLedCzacha+LiczbaLedBramka+LiczbaLedMlot], LiczbaLedMlotScianka, starthue, 20);
    } 
    else if (GameMode == HAMMER)
    {
      fill_solid(leds2, num_leds2, CRGB::Red);
      fadeInOut(&leds2[LiczbaLedCzacha+LiczbaLedBramka], LiczbaLedMlot, GREEN); // green
      fadeInOut(&leds2[LiczbaLedCzacha+LiczbaLedBramka], LiczbaLedMlot, GREEN); // green
      effect_theater_chase(&leds2[LiczbaLedCzacha+LiczbaLedBramka+LiczbaLedMlot], LiczbaLedMlotScianka, 85); // green
      
    }

    if(Fram.BoxerMat==1) {
      whiteSnake(leds1, num_leds1);
      whiteSnake(leds3, num_leds3);
    }
    else {
      fill_rainbow(leds1, num_leds1, starthue, 20);
      if(Fram.BoxerModel==FIST) {
        fill_rainbow(&leds3[0], 106, starthue, 10);
        fill_rainbow(&leds3[106*3], 106, starthue, 10);
        starthue+=2;
        fill_solid(&leds3[106*1], 106, CRGB::Red);
        fill_solid(&leds3[106*2], 106, CRGB::Red);

        uint8_t inner = beatsin8(30, 106/4, 106/4*3);    // Move 1/4 to 3/4
        uint8_t outer = beatsin8(30, 0, 106-1);               // Move entire length
        uint8_t middle = beatsin8(30, 106/3, 106/3*2);   // Move 1/3 to 2/3

        if(middle<106) leds3[106+middle] = CRGB::White;
        if(inner<106) leds3[106+inner] = CRGB::Blue;
        if(outer<106) leds3[106+outer] = CRGB::Black;
        if(middle<106) leds3[2*106+middle] = CRGB::White;
        if(inner<106) leds3[2*106+inner] = CRGB::Blue;
        if(outer<106) leds3[2*106+outer] = CRGB::Black;
        fadeLightBy(leds3, num_leds3, 50);
      }
      else fill_rainbow(leds3, num_leds3, starthue, 20);
      nscale8(leds3, num_leds3, 128); // 128 = 50% jasności
      starthue += 5;
    }

    static uint8_t counterBlackDaszek = 0;
    counterBlackDaszek++;
    leds1[counterBlackDaszek%LiczbaLedDaszek] = CRGB::Black;

    static int cntOtwarta = 0;

    if (Sens4Done && (GameMode == HAMMER)) {
      HammerTimeUs = TimeSens4Stop - TimeSens4Start;
      matrix.clearScreen(1);
      matrix.clearScreen(2);
      delay(1);      
      Sens4Done = false;
      Wynik = 0;
      TimeSens4Stop = 0;
      TimeSens4Start = 0;      
      matrix.loadGif("anim/naliczanie.gif", 0, 0, 192, 192, 0);
      matrix.loadGif("anim/strzal_mlot-resize.gif", 0, 512-132, 64, 132, 10, 2);
      delay(1);      
      PlayMp3(280);
      wynik = KonwersjaCzasuNasSile(HammerTimeUs);
      wynik = (wynik * 73) / 100; //obnizamy na dziendobry dla mlota
      wynik = (wynik * Fram.SkalaSilyMlot) / 100;
      HammerTimeUs = 0;
      Stroboskop(10);
      Zar.Z14_HALOGEN1 = 0;
      Zar.Z15_HALOGEN2 = 0;
      if(Fram.Credit>=1) Fram.Credit--;
    }

    if ((TimeSens0Stop > 0) && (TimeSens0Start > 0))
    {
      matrix.clearScreen();
      delay(1);
      matrix.loadGif("anim/naliczanie.gif", 0, 0, 192, 192, 0);
      delay(1);
      wynik = TimeSens0Stop - StartArray[0];
      PlayMp3(280);      
      //DebugSensor();
      //printf("\n TimeSens0Stop - StartArray[0]=%d", wynik);
      wynik = KonwersjaCzasuNasSile(wynik);
      TournamentScores[CurrentPlayer] = wynik;
      //printf("\n wynik=%d", wynik);

      Stroboskop(10);
      Zar.Z14_HALOGEN1 = 0;
      Zar.Z15_HALOGEN2 = 0;
      if(Fram.Credit>=1) Fram.Credit--;
    }
    else if ((TimeSens2Stop > 0) && (TimeSens2Start > 0))
    {
      matrix.clearScreen();
      delay(1);
      matrix.loadGif("anim/naliczanie.gif", 0, 0, 192, 192, 0);
      delay(1);

      PlayMp3(233);      
      wynik = TimeSens2Stop - StartArray[0];
      DebugSensor();
      int wynikInterruptLogArray = InterruptLogArray[InterruptTimeArrayIndex-1].timestamp - InterruptLogArray[0].timestamp;
      printf("\n wynikInterruptLogArray=%d", wynikInterruptLogArray);

      if(wynik<500) {
        printf("\n wynik = wynikInterruptLogArray");
        wynik = wynikInterruptLogArray;
      }

      printf("\n TimeSens2Stop - StartArray[0] = %d", wynik);
      wynik = KonwersjaCzasuNasSile(wynik);
      TournamentScores[CurrentPlayer] = wynik;
      printf("\n wynik=%d", wynik);
      
      Stroboskop(10);
      Zar.Z14_HALOGEN1 = 0;
      Zar.Z15_HALOGEN2 = 0;
      if(Fram.Credit>=1) Fram.Credit--;
    }
    else if ((wynik==0) && (cntOtwartaKopacz>5) && (cntGruchaZamknieta>5) && (GameMode == KOPACZ)) {
        matrix.clearScreen();
        delay(1);
        matrix.loadGif("anim/naliczanie.gif", 0, 0, 192, 192, 0);
        delay(1);
        PlayMp3(233);
        wynik = InterruptLogArray[InterruptTimeArrayIndex-1].timestamp - InterruptLogArray[0].timestamp;
        DebugSensor();
        printf("\n KonwersjaCzasuNasSile 1515");
        wynik = KonwersjaCzasuNasSile(wynik);
        TournamentScores[CurrentPlayer] = wynik;
        Stroboskop(10);
        Zar.Z14_HALOGEN1 = 0;
        Zar.Z15_HALOGEN2 = 0;
        if(Fram.Credit >= 1) Fram.Credit--;
    }
    else if ((wynik==0) && (cntOtwartaBoxer>5) && (cntGruchaZamknieta>5) && (GameMode == BOXER)) { 
      //jesli grucha w gorze to takze zrob pomiar
        matrix.clearScreen();
        delay(1);
        matrix.loadGif("anim/naliczanie.gif", 0, 0, 192, 192, 0);
        delay(1);
        PlayMp3(280);
        wynik = InterruptLogArray[InterruptTimeArrayIndex-1].timestamp - InterruptLogArray[0].timestamp;
        printf("\n KonwersjaCzasuNasSile 1529");
        wynik = KonwersjaCzasuNasSile(wynik);
        TournamentScores[CurrentPlayer] = wynik;
        Stroboskop(10);
        Zar.Z14_HALOGEN1 = 0;
        Zar.Z15_HALOGEN2 = 0;
        if(Fram.Credit >= 1) Fram.Credit--;
    }

    if (GruchaOtwarta() > 0) cntOtwarta = 0;  
    if (1==GruchaOtwarta()) cntOtwartaBoxer++;  
    if (2==GruchaOtwarta()) cntOtwartaKopacz++;  
    if (0==GruchaOtwarta()) cntGruchaZamknieta++;  
    //printf("\n Pomiar cntOtwartaBoxer=%  cntOtwartaKopacz=%d cntGruchaZamknieta=%d", cntOtwartaBoxer, cntOtwartaKopacz, cntGruchaZamknieta);
  }
  else
  {
    Matrix_SetText1("Wow !!!");
    Matrix_SetText2("Processing...");
    
    if (true==Naliczanie(wynik))
    {
      vTaskDelay(2);
      Wynik = wynik;
      char WynikStr[10];      

      matrix.loadGif("anim/taniec.gif", 0, 0, 192, 192, 0, 1);
      matrix.loadGif("anim/gwiazdki.gif", 0, 0, 64, 512, 10, 2);
      
      if (GameMode==BOXER) {
        Matrix_SetText1("Reaction Time", true);
        snprintf(WynikStr, 10, "%dms", 1000-Wynik);
      } else if (GameMode==KOPACZ) {
        Matrix_SetText1("Speed of ball", true);
        snprintf(WynikStr, 10, "%dkm/h", Wynik/6);
      } else {  
        Matrix_SetText1("Strength", true);
        snprintf(WynikStr, 10, "%dkN", Wynik);
      }
      Matrix_SetText2(WynikStr, true);
      
      if(Fram.BoxerModel == GIFT_ESPANIOL) {
        if ((Wynik==333) || (Wynik==444)) RunEngineUntilGift(1);
        if ((Wynik==555) || (Wynik==666)) RunEngineUntilGift(2);
        if ((Wynik==777) || (Wynik==888)) RunEngineUntilGift(3);
      }else if(Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS_ESPANIOL) {
        if ((Wynik==555) || (Wynik==666)) RunEngineUntilGift(1);
        if ((Wynik==777) || (Wynik==888)) RunEngineUntilGift(2);
      }
      else if(Fram.BoxerModel == GIFT) {
        if(Wynik==555) RunEngineUntilGift(1);
      }      
      if ((Fram.BoxerModel == GIFT) || (Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS)) {
        if(Wynik==666) RunEngineUntilGift(2);
        if(Wynik==777) RunEngineUntilGift(3);
      }
      
      if(KEY_START1) QuickStartBoxer = true;
      else if(KEY_START2) QuickStartKopacz = true;
      TicketOut += ((Fram.TicketScore * Wynik) /100);
      if(Wynik > Fram.Record) {
          QuickStartBoxer = false;
          Fram.Record = Wynik;
          printf("\n Nowy Rekord=%d", Fram.Record);
          PlayRandomMp3(wcPlayFanfaryRekord);
          TicketOut += Fram.TicketRecord;
          uint8_t starthue = 128;
          for(int i=0;i<30;i++) {
            UpdateCredit();
            DisplayRekord(0xFFFF);
            fill_rainbow( leds3, num_leds3, starthue, 30);
            fill_solid(leds1, num_leds1, CRGB::Gold); 
            fill_solid(leds2, num_leds2, CRGB::Black); 
            delay(100);
            DisplayRekord(Fram.Record);
            fill_rainbow( leds3, num_leds3, starthue, 30);
            fill_solid(leds1, num_leds1, CRGB::Black); 
            fill_solid(leds2, num_leds2, CRGB::Gold); 
            delay(100);
            if(false==Mp3IsPlaying()) break;
          }
          Fram.Credit++;
          DisplayCredit(Fram.Credit);
          PlayMp3(wcPlayPlus1Kredyt[0][Fram.Language]);
          for(int i=0;i<30;i++) {
            UpdateCredit();
            DisplayCredit(0xFFFF); //todo dorobic znikanie
            fill_rainbow( leds3, num_leds3, starthue, 30);
            fill_solid(leds1, num_leds1, CRGB::Gold); 
            fill_solid(leds2, num_leds2, CRGB::Black); 
            delay(100);
            DisplayCredit(Fram.Credit);
            fill_rainbow( leds3, num_leds3, starthue, 30);
            fill_solid(leds1, num_leds1, CRGB::Black); 
            fill_solid(leds2, num_leds2, CRGB::Gold); 
            delay(100);
            if(false==Mp3IsPlaying()) break;
          }
          //CzekajDoKoncaMp3LubKlawiszStart();
      }
      else if((!QuickStartBoxer) && (!QuickStartKopacz)) KomunikatyOsiagniec(Wynik);
      TournamentScores[CurrentPlayer] = Wynik;
      wynik = 0;
      TimeSens0Stop = 0;
      TimeSens0Start = 0;
      TimeSens2Start = 0;
      TimeSens2Stop = 0;
      rising = 0;
      falling = 0;
      if (NumberOfPLayers > 1) CurrentPlayer++;
      if (CurrentPlayer > NumberOfPLayers) {
        uint8_t winner = 1;
        TournamentInProgress = false;
        for (uint8_t i = 2; i <= NumberOfPLayers; i++) {
          if (TournamentScores[i] > TournamentScores[winner]) winner = i;
          printf("\n TournamentScores[0]=%d", TournamentScores[0]);
          printf("\n TournamentScores[1]=%d", TournamentScores[1]);
          printf("\n TournamentScores[2]=%d", TournamentScores[2]);
          printf("\n TournamentScores[3]=%d", TournamentScores[3]);
          printf("\n winner=%d", winner);
          
        }
        PlayMp3(Mp3TheWinnerIs[0][Fram.Language]);
        while(Mp3IsPlaying()) {
          vTaskDelay(10);
          StroboScope(leds1, num_leds1, CRGB::Red);
          StroboScope(leds2, num_leds2, CRGB::Green);
          StroboScope3Player(leds3, num_leds3, CRGB::Blue);
          DisplayPlayer(winner, TournamentScores[winner], true);
        }
        switch(winner) {
          case 1:
            PlayMp3(Mp3PlayerOne[0][Fram.Language]);
            break;
          case 2:
            PlayMp3(Mp3PlayerTwo[0][Fram.Language]);
            break;
          case 3:
            PlayMp3(Mp3PlayerThree[0][Fram.Language]);
            break;
        }

        while(Mp3IsPlaying()) {
          vTaskDelay(10);
          StroboScope(leds1, num_leds1, CRGB::White);
          StroboScope(leds2, num_leds2, CRGB::Blue);
          StroboScope(leds3, num_leds3, CRGB::Red);
          DisplayPlayer(winner, TournamentScores[winner], true);
        }

        PlayRandomMp3(wcPlayFanfaryRekord);
        
        while(Mp3IsPlaying()) {
          vTaskDelay(10);
          fadeInOutWhite(leds1, num_leds1);
          fadeInOutWhite(leds2, num_leds2);
          fadeInOutWhite(leds3, num_leds3);
          DisplayPlayer(winner, TournamentScores[winner], true);
        }

        CurrentPlayer = 1;
        NumberOfPLayers = 1; 
        TournamentInProgress = false;
        for (int i=0;i<3;i++) TournamentScores[i] = 0;
        firstEntry = true;
      }
      ret = true;
    }
  }
  return ret;
}



bool EndGame() {
  bool ret = false;
  Sens4Done = false;
  TimeSens4Stop = 0;
  TimeSens4Start = 0;
  matrix.clearScreen(1);
  matrix.clearScreen(2);
  return ret;
}

uint8_t EditMode = 0;

void MenuLong(const char name[], volatile uint32_t &variable, uint32_t downLimit, uint32_t upLimit, uint8_t step) {
  static bool migacz = false;
  static uint8_t fastEditUp = 0;
  static uint8_t fastEditDown = 0;
  static unsigned long aktualnyCzas = millis();
  if ((millis() - aktualnyCzas) > 300) {
    aktualnyCzas = millis();
    migacz = !migacz;
  }
  static uint32_t editVal;
  if(EditMode==0) editVal = variable;

  myLcd.print(name);
  myLcd.setCursor(4,1);
  myLcd.print("     ");
  myLcd.setCursor(0,1);
  myLcd.print("val=");
  
  if ((EditMode==1) && migacz) myLcd.print("      ");
  else myLcd.print(editVal);

  if(KEY_SETUP) {
      if(EditMode<1) {
        EditMode++;
      }
      else {
        EditMode = 0;
        variable = editVal;
      }
      delay(300);
  }
  if(KEY_UP && EditMode==1) {
      if(editVal < upLimit) editVal+=step;
      else editVal = downLimit;
      if (fastEditUp > 2) delay(10);
      else delay(300);
      vTaskResume( TaskPcfHandle );
      if (KEY_UP) fastEditUp++;
      else fastEditUp = 0;
  }
  else fastEditUp = 0;

  if(KEY_DOWN && EditMode==1) {
      if(editVal > downLimit) editVal-=step;
      else editVal = upLimit;
      if (fastEditDown > 2) delay(10);
      else delay(300);
      vTaskResume( TaskPcfHandle );
      if (KEY_DOWN) fastEditDown++;
      else fastEditDown = 0;
  }
  else fastEditDown = 0;
}

void MenuWord(const char name[], volatile uint16_t &variable, uint16_t downLimit, uint16_t upLimit, uint8_t step) {
  static bool migacz = false;
  static uint8_t fastEditUp = 0;
  static uint8_t fastEditDown = 0;
  static unsigned long aktualnyCzas = millis();
  if ((millis() - aktualnyCzas) > 300) {
    aktualnyCzas = millis();
    migacz = !migacz;
  }
  static uint16_t editVal;

  if(EditMode==0) editVal = variable;

  myLcd.print("val=");
  if ((EditMode==1) && migacz) myLcd.print("   ");
  else myLcd.print(editVal);
  
  myLcd.setCursor(0,1);
  myLcd.print(name);

  if(KEY_SETUP) {
      if(EditMode<1) {
        EditMode++;
      }
      else {
        EditMode = 0;
        variable = editVal;
      }
      delay(300);
  }
  if(KEY_UP && EditMode==1) {
      if(editVal < upLimit) editVal++;
      else editVal = downLimit;
      if (fastEditUp > 2) delay(5);
      else delay(300);
      if (KEY_UP) fastEditUp++;
      else fastEditUp = 0;
  }
  else fastEditUp = 0;

  if(KEY_DOWN && EditMode==1) {
      if(editVal > downLimit) editVal--;
      else editVal = upLimit;
      if (fastEditDown > 2) delay(5);
      else delay(300);
      if (KEY_DOWN) fastEditDown++;
      else fastEditDown = 0;
  }
  else fastEditDown = 0;
}


#define FIRMWARE_UPDATE  0
#define MP3_UPDATE  1


void MenuUpdate(uint8_t update)
{
  if (update == FIRMWARE_UPDATE)
  {
      myLcd.print("Update Firmware");
      if (true == KEY_SETUP) {
        xSemaphoreTake( xSemaphore, ( TickType_t ) 10000 );
        vTaskDelete( Task1 );
        //connect Wifi
        myLcd.setCursor(0, 1);
        myLcd.print("Wifi...");
        int wifiState = StartWifiClient();
        if(wifiState==WL_CONNECTED) {
          myLcd.print("CONNECTED");
          delay(1000);
          myLcd.setCursor(0, 1);
          myLcd.print("                ");
          myLcd.setCursor(0, 1);
          myLcd.print("Download...");
          delay(1000);
          if(true==DownloadFirmwareUpdate()) {
            myLcd.print("OK");
            delay(1000);
            myLcd.setCursor(0, 1);
            myLcd.print("                ");
            myLcd.setCursor(0, 1);
            myLcd.print("Update...");
            UpdateFromMMC();
            myLcd.print("OK");
            delay(3000);
            ESP.restart();
          }   
          else {
            myLcd.print("error");
            delay(3000);
          }
        }
        else {
          if(wifiState==WL_NO_SSID_AVAIL) myLcd.print("NO_SSID");
          else myLcd.print(wifiState);
        }
     }
  }
  else if (update == MP3_UPDATE)
  {
    if(UpdateProgress==0) {
      myLcd.print("MP3_Update");
      if (true == KEY_SETUP) {
        xSemaphoreTake( xSemaphore, ( TickType_t ) 10000 );
        vTaskDelete( Task1 );

        int wifiState = StartWifiClient();
        myLcd.setCursor(0, 1);
        myLcd.print("Wifi...");
        if(wifiState==WL_CONNECTED) {
          myLcd.print("CONNECTED");
          if(StartMp3Update()) {
            UpdateProgress = 1; //Start update if Wifi connected and web read
            myLcd.setCursor(0, 1);
            myLcd.print("progress:          ");
            delay(1000);
          }
        }
        else {
          if(wifiState==WL_NO_SSID_AVAIL) myLcd.print("NO_SSID");
          else myLcd.print(wifiState);
        }
      }
    }
    else {
      RunMp3Update();
      myLcd.setCursor(10, 1);
      myLcd.print(UpdateProgress);      
    }
  }
}

bool MenuByte(const char name[], volatile uint8_t &variable, uint8_t downLimit, uint8_t upLimit, uint8_t step) {
  static bool migacz = false;
  static uint8_t fastEditUp = 0;
  static uint8_t fastEditDown = 0;
  static unsigned long aktualnyCzas = millis();
  if ((millis() - aktualnyCzas) > 100) {
    migacz = !migacz;
    aktualnyCzas = millis();
  }
  static uint8_t editVal;

  if(EditMode==0) editVal = variable;

  myLcd.print("val=");
  if ((EditMode==1) && migacz) myLcd.print("    ");
  else myLcd.print(editVal);
  
  myLcd.setCursor(0,1);
  myLcd.print(name);

  if(KEY_SETUP) {
      if(EditMode<1) {
        EditMode++;
      }
      else {
        EditMode = 0;
        variable = editVal;
        delay(300);
        return true;
      }
      
      delay(300);
  }
  if(KEY_UP && EditMode==1) {
      if(editVal < upLimit) editVal++;
      else editVal = downLimit;
      if (fastEditUp > 2) delay(10);
      else delay(300);
      vTaskResume( TaskPcfHandle );
      if (KEY_UP) fastEditUp++;
      else fastEditUp = 0;
  }
  else fastEditUp = 0;

  if(KEY_DOWN && EditMode==1) {
      if(editVal > downLimit) editVal--;
      else editVal = upLimit;
      if (fastEditDown > 2) delay(10);
      else delay(300);
      vTaskResume( TaskPcfHandle );
      if (KEY_DOWN) fastEditDown++;
      else fastEditDown = 0;
  }
  else fastEditDown = 0;

  return false;
}

void SetupMenu(void)
{
  static uint8_t NumerSetup = 1;
  static uint8_t fastRunUp1 = 0;
  static uint8_t fastRunUp2 = 0;
  static uint8_t fastRunDown1 = 0;
  static uint8_t fastRunDown2 = 0;
  static bool migacz = false;
  static unsigned long aktualnyCzas = millis();

  if ((millis() - aktualnyCzas) > 300) {
    aktualnyCzas = millis();
    migacz = !migacz;
    if(migacz) aktualnyCzas += 100;
  }

  myLcd.setCursor(0,0);
  myLcd.print("Setup");
  myLcd.print(NumerSetup);
  myLcd.print(":");
  

    if(KEY_UP && EditMode==0) { 
    NumerSetup++;
    PlayMp3(952);
    delay(300);
    myLcd.clear();
  }
  if(KEY_DOWN && EditMode==0) { 
    NumerSetup--;
    if(NumerSetup==0) NumerSetup = 40;
    PlayMp3(952);
    delay(300);
    myLcd.clear();
  }
  switch(NumerSetup) {
    
    case 10:
      MenuUpdate(FIRMWARE_UPDATE);
    break;
    case 11:
      MenuUpdate(MP3_UPDATE);      
    break;
    case 12:
      MenuByte("DemoMusicInterval", Fram.InterwalPrezentacjiMinuty, 2, 240, 1);
    break;
    case 13:
      MenuWord("Restart Record", Fram.Record, 600, 600, 1);
    break;
    case 14:
      MenuByte("Strenght Boxer", Fram.SkalaSily, 80, 120, 1);
    break;
    case 15:
      MenuByte("Strenght Kicker", Fram.SkalaSilyKicker, 80, 120, 1);
    break;
    case 16:
      MenuLong("TotalCounter", Fram.TotalCounter, 0, 0, 0);
    break;
    case 17: //TODO
      static uint32_t TestCreditBak = Fram.TestCredit;
      MenuLong("TestCredit", Fram.TestCredit, 0, 0xFFFF, 1);
      if(Fram.TestCredit > TestCreditBak) {
        Fram.Credit += (Fram.TestCredit - TestCreditBak);
        TestCreditBak = Fram.TestCredit;
      }
    break;
    case 18: //TODO
      //MenuByte("Reset", TestCredit, 0, 1, 1);
    break;
    case 19: //Wersja Jezykowa
      MenuByte("Language", Fram.Language, 0, MAX_LANGUAGE, 1);
    break;
    case 20: //FreePlay
    {
      bool changed = MenuByte("FreePlay", Fram.FreePlay, 0, 1, 1);
      if (changed) {
        if(Fram.FreePlay==false) Fram.Credit = 0;
      }
      break;
    }

    case 21: 
      MenuByte("TicketRecord", Fram.TicketRecord, 0, 250, 1);
    break;

    case 22: 
      MenuByte("TicketCredit", Fram.TicketCredit, 0, 10, 1);
    break;

    case 23: 
      MenuByte("TicketScore100", Fram.TicketScore, 0, 50, 1);
    break;

    case 29: 
      MenuByte("Kids  ", Fram.Kids, 0, 10, 1);
    break;

    case 30:
      MenuLong("Ratio555", Fram.GiftWinRatioMale, 0, 10000, 1);
    break;      

    case 31:
      MenuLong("Ratio666", Fram.GiftWinRatioSrednie, 0, 10000, 1);
    break;

    case 32:
      MenuLong("Ratio777", Fram.GiftWinRatioDuze, 0, 10000, 1);
    break;

    case 33:
      MenuLong("GiftCnt555", Fram.GiftCntOut555, 0, 1000000, 0);
    break;      

    case 34:
      MenuLong("GiftCnt666", Fram.GiftCntOut666, 0, 1000000, 0);
    break;

    case 35:
      MenuLong("GiftCnt777", Fram.GiftCntOut777, 0, 1000000, 1);
    break;

    case 36:
      MenuByte("PulseTime", Fram.PulseTime, 50, 250, 10);
    break;    
      
    case 37:
      MenuByte("PcbRev ", Fram.PcbRev, 0, 200, 1);
    break;
    case 38:
      MenuByte("LedRgb ", Fram.LedRgb, 0, 1, 1);
    break;
    case 39:
      MenuByte("Syslog ", Fram.Syslog, 0, 1, 1);
    break;
    case 40:
    {
      uint8_t TestEnable = 0;
      MenuByte("Test    ", TestEnable, 0, 1, 1);
      if (TestEnable) stan = TEST;
    }      
    break;
    case 41:
      MenuByte("Model    ", Fram.BoxerModel, 0, 11, 1);  // 0 - Hit, 1-Kids, 2-Gift, 3-Combat, 4 CombatKids, 5-DoubleHitKidsGift, 6-B&W, 7-GiftEspaniol, 8-Fist, 9-Monster, 10-DoubleHitGiftKidsEspaniol, 11-DoubleHit
    break;
    case 42:
      MenuByte("BoxerMat      ", Fram.BoxerMat, 0, 1, 1);
    break;
    case 43:
      MenuByte("Boxer3Player   ", Fram.Boxer3Player, 0, 1, 1);
    break;
    case 44:
      MenuByte("Strenght Hammer", Fram.SkalaSilyMlot, 50, 150, 1);
    break;
    
  
  }

  // --=== SETUP COIN/BANK Pulse/Credit
  char tempStr[5];
  if ((NumerSetup>=1) && (NumerSetup <= 9)) {
    if(NumerSetup<=4) {
      myLcd.print("Coin");
      myLcd.print(NumerSetup);
    }
    else if (NumerSetup<=8) {
      myLcd.print("Bank");
      myLcd.print(NumerSetup-4);
    }
    else {
      myLcd.print("Nayax");
    }

    myLcd.setCursor(0,1);
    myLcd.print("Mc:");
    if ((EditMode==1) && migacz) myLcd.print("    ");
    else {
      snprintf( tempStr, 5, "%04f", Fram.CoinBankPuls[NumerSetup-1]);
      myLcd.print(tempStr);
    }
    myLcd.print(" ");
    
    myLcd.setCursor(8,1);
    myLcd.print("Cr:");
    if ((EditMode==2) && migacz) myLcd.print("    ");
    else {
      snprintf( tempStr, 5, "%04f", Fram.CoinBankCred[NumerSetup-1]);
      myLcd.print(tempStr);
    }
    myLcd.print(" ");

    if(KEY_SETUP) {
      if(EditMode<2) {
        EditMode++;
      }
      else EditMode = 0;
      delay(300);
    }
    if(KEY_UP && EditMode==1) {
      fastRunUp1++;
      if(Fram.CoinBankPuls[NumerSetup-1] < 1) Fram.CoinBankPuls[NumerSetup-1] += 0.05;
      else Fram.CoinBankPuls[NumerSetup-1]+=1.00;
      if(fastRunUp1 < 4) delay(200);
      delay(100);
    }
    else fastRunUp1 = 0;

    if(KEY_DOWN && EditMode==1) {
      fastRunDown1++;
      if(Fram.CoinBankPuls[NumerSetup-1]>1) Fram.CoinBankPuls[NumerSetup-1]--;
      else if(Fram.CoinBankPuls[NumerSetup-1]>=0) Fram.CoinBankPuls[NumerSetup-1]-=0.05;      
      if(Fram.CoinBankPuls[NumerSetup-1]<0) Fram.CoinBankPuls[NumerSetup-1] = 0;
      if(fastRunDown1 < 4) delay(200);
      delay(100);
    }
    else fastRunDown1 = 0;

    if(KEY_UP && EditMode==2) {
      fastRunUp2++;
      if (Fram.CoinBankCred[NumerSetup-1] < 1) Fram.CoinBankCred[NumerSetup-1] += 0.05;
      else Fram.CoinBankCred[NumerSetup-1]+=1.00;
      if(fastRunUp2 < 4) delay(200);
      delay(100);
    }
    else fastRunUp2 = 0;

    if(KEY_DOWN && EditMode==2) {
      fastRunDown2++;
      if (Fram.CoinBankCred[NumerSetup-1] > 1) Fram.CoinBankCred[NumerSetup-1]--;
      else if (Fram.CoinBankCred[NumerSetup-1] >= 0) Fram.CoinBankCred[NumerSetup-1]-=0.05;
      if (Fram.CoinBankCred[NumerSetup-1] < 0) Fram.CoinBankCred[NumerSetup-1] = 0;
      if(fastRunDown2 < 4) delay(200);
      delay(100);
    }
    else fastRunDown2 = 0;
  }
}


void UpdateCredit(void) {
  static float KredytBak = Fram.Credit;
  
  if(KredytBak!=Fram.Credit) {
    //if(Fram.Credit > KredytBak) PlayRandomMp3(MP3_Coin); w zlym momencie plumka
    KredytBak = Fram.Credit;
    Fram.Credit = Fram.Credit;
    vTaskDelay(1);
    DisplayCredit(Fram.Credit);
    vTaskResume(TaskPcfHandle);
  }
  if(stan != TEST) DisplayCredit(Fram.Credit);
}
void ProgMp3(void) {

    File fileMp3;
  
  while ( Serial.available() )
  {
    static bool fileInProgres = false;
    uint8_t frag = Serial.read();
    static uint32_t size = 0;
    static uint32_t received = 0;
     
    if(fileInProgres==true) {
      fileMp3.write(frag);
      received++;
      
      if(received>=size) {
        printf("\n EOF");
        fileInProgres = false;
        size = 0;
        received = 0;
        fileMp3.close();
      }

      if(received%1000==0) {
        printf("\n r=%d", received);
        break;
      }
    }
    else if (frag==':') {
      printf("!");
      //xSemaphoreTake( xSemaphore, ( TickType_t ) 10 );
      String SizeStr = Serial.readStringUntil(',');
      size = SizeStr.toInt();
      
      printf("\n file size =%d", size);

      String FileName = Serial.readStringUntil(';');
      FileName = "/" + FileName;
      printf("\n file name = %S", FileName);

      fileMp3 = SD_MMC.open(FileName, FILE_WRITE);      
      fileInProgres = true;
    }    
  }
}
void FramUpdate(void)
{
  ee.updateBlock(FRAM_DATA_STRUCT, (uint8_t*)&Fram, sizeof(Fram));
}

void DisplayGameMatrix(void) {
  static uint64_t timestamp = millis();
  if (millis() - timestamp > 100) {
    timestamp = millis();    
    matrix.deleteElement(0); //grafika w background Choinka
    matrix.displayText("RECORD", 5, 26, 2, 255, 0, 0, "fonts/9x18B.bdf", 1, 0);
    matrix.displayText(String(Fram.Record).c_str(), 192/2 - String(Wynik).length()*10, 16, 2, 255, 0, 0, "fonts/ComicNeue-Bold-48.bdf", 2, 0);

    matrix.displayText("RECORD", 5, 10, 2, 255, 0, 0, "fonts/9x18B.bdf", 30, 0, 2);
    matrix.displayText(String(Fram.Record).c_str(), 15, 30, 2, 255, 0, 0, "fonts/9x18B.bdf", 31, 0, 2);

    matrix.displayText("SCORE", 5, 75, 2, 0, 255, 0, "fonts/9x18B.bdf", 3, 0);
    matrix.displayText(String(Wynik).c_str(), 192/2 - String(Wynik).length()*10, 62, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf", 4, 0);

    matrix.displayText("SCORE", 10, 140, 2, 255, 255, 255, "fonts/9x18B.bdf", 32, 0, 2);
    matrix.displayText(String(Wynik).c_str(), (64 - (String(Wynik).length()*19))/2, 155, 2, 255, 255, 255, "fonts/Verdana-24-r.bdf", 33, 0, 2);

    matrix.displayText("Credit", 5, 115, 2, 0, 0, 255, "fonts/9x18B.bdf", 5, 0);
    if (Fram.Credit==55)  matrix.displayText("Free Play", 192/2 - 15, 115, 2, 0, 0, 255, "fonts/9x18B.bdf", 6, 0);
    else matrix.displayText(String(Fram.Credit).c_str(), 192/2 - 15, 115, 2, 0, 0, 255, "fonts/9x18B.bdf", 6, 0);
  }
}

void loop() {

  static state_t stanBak;
  //ArduinoOTA.handle();
  Mp3Handle();
  MechCount.Loop();
  FramUpdate();
  myLcd.handle();
 
  wTimerRandom++;

  if(Fram.FreePlay) Fram.Credit = 55;
  
  if(stan != SETUP_MENU) {
    if(KEY_UP) {
      if (Fram.Mp3Volume<30) Fram.Mp3Volume++;
      myLcd.setCursor(0,0);
      myLcd.print(" Volume = ");
      myLcd.print(Fram.Mp3Volume);    
      audio.setVolume(Fram.Mp3Volume);
      printf("\n vol=%d",Fram.Mp3Volume);
      delay(300);
      delay(1);
      printf("\nPLAY_GIF 1\n"); 
    }
    if(KEY_DOWN) {
      if (Fram.Mp3Volume>0)Fram.Mp3Volume--;
      myLcd.setCursor(0,0);
      myLcd.print(" Volume = ");
      myLcd.print(Fram.Mp3Volume);
      audio.setVolume(Fram.Mp3Volume);
      printf("\n KEY_DOWN vol=%d",Fram.Mp3Volume);
      delay(300);
      printf("\nSTOP_GIF\n"); 
    }
  }
  if(KEY_SETUP) {
    if(stan != SETUP_MENU) {
      stan = SETUP_MENU;
      while(KEY_SETUP) {
        PlayMp3(955);
        delay(300);
        vTaskResume( TaskPcfHandle );
      }
    }
  }
  switch(stan) {
    case(ERROR_STATE):
    break;
    case(SETUP_MENU):
      SetupMenu();
    break;
    case(TEST):
      myLcd.setCursor(0, 0);
      myLcd.print("--== TEST ==--");
      vTaskDelay(2);
      myLcd.setCursor(0, 1);
      vTaskDelay(2);
      myLcd.print("mp3=");
      myLcd.print(numMp3Files);
      
      Test();
    break;

    case(CHOINKA):
      myLcd.setCursor(0,0);
      myLcd.print("--= CHOINKA =--");
      Choinka();
      if(Fram.Credit >= 1) stan = POWITANIE;
      firstEntryGameStart = true;
      if((millis()/1000)%(60*Fram.InterwalPrezentacjiMinuty)==0) //co 5 minut
      { 
        stan = PREZENTACJA; 
      }
    break;
    case(PREZENTACJA):
    {
      myLcd.setCursor(0,0);
      myLcd.print("-=PREZENTACJA=-");

      bool ret = Prezentacja();     
      if(!ret) stan = CHOINKA; 
      if(Fram.Credit >= 1) {
          stan = POWITANIE;
      }
    }
    break;
    case(POWITANIE):
      printf("\n POWITANIE");
      printf("\nSTOP_GIF\n"); 
      myLcd.setCursor(0,0);
      myLcd.print("-= POWITANIE =-");
      if(GruchaOtwarta()) CzyGruchaOpadla = true;
      else CzyGruchaOpadla = false;
      Zar.All = 0x00000000;
      PlayMp3AndWait(wcPlayPowitanie[0][Fram.Language]);
      stan = GAME_START;
      DisplayRekord(Fram.Record);
    break;
    case(RESTART):
      printf("\n RESTART");
      printf("\nSTOP_GIF\n"); 
      myLcd.setCursor(0,0);
      myLcd.print("-= RESTART =-");
      if(GruchaOtwarta()) CzyGruchaOpadla = true;
      else CzyGruchaOpadla = false;
      Zar.All = 0x00000000;
      Sens4Done = false;
      TimeSens4Stop = 0;
      TimeSens4Start = 0;

      stan = GAME_START;
      DisplayRekord(Fram.Record);
    break;
    case(GAME_START): 
      myLcd.setCursor(0,0);
      myLcd.print("-= GAME_START =-");
      if(GameStart()) {
        stan = POMIAR;
        DisplayRekord(Fram.Record);
        DisplayWynik(Wynik);
        DisplayPlayer(CurrentPlayer, Wynik);
        matrix.clearScreen(1);
        matrix.clearScreen(2);
      }
    break;
    case(POMIAR):
      printf("");
      printf("");
      myLcd.setCursor(0,0);
      myLcd.print("-= POMIAR =-");
      if(true==Pomiar()) {
        printf("\n Pomiar = true Wynik=%d", Wynik);
        if(Fram.Credit >= 1) stan = RESTART;
        else stan = GAME_END;   
        delay(1);
      }
    break;
    case(GAME_END):
    {
      QuickStartBoxer = false;
      QuickStartKopacz = false;
      PlayMp3AndWait(wcGameOver0Language[0][Fram.Language]);
      PlayRandomMp3AndWait(wcPlayGameOver);
      myLcd.setCursor(0,0);
      myLcd.print("-= GAME_END =-");
      EndGame();
      stan = CHOINKA;
    }
    break;
  }

  if(stanBak != stan) {
    stanBak = stan;
    printf("\n stan=%d", stan);
  }

  
  UpdateCredit();
  UpdateSPI();  


  delay(10);
  vTaskDelay(10);

  static uint64_t WifiTimer = millis();
  if (millis() - WifiTimer > 500) {
    static bool LedBlue = false;
    WifiTimer = millis();
    if (WiFi.status() == WL_CONNECTED) {
      LedBlue = !LedBlue;
      SpiLed.LED2 = LedBlue;
    }        
    else SpiLed.LED2 = 1; //off
  }  
}
