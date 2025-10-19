#include "Queue.h"
#include <freertos/queue.h>
#include "pcf.h"

extern uint32_t wTimerRandom;

QueueHandle_t queue;
char txBuffer[50];
char rxBuffer[50];
bool IsMp3Playing = false;
Audio audio;
Queue<int> Mp3Queue = Queue<int>(); // Queue of max 256 int

extern void UpdateCredit(void);

SemaphoreHandle_t xSemaphore = NULL;

TaskHandle_t Task1;
void Task1code( void * pvParameters ) {
  for(;;) {
    if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
    {
      audio.loop();
      xSemaphoreGive( xSemaphore );
    }
    vTaskDelay(10);
    //FastLED.show();
  }
}

// 0 - gra, 1 - cisza
void MP3_SetMute(uint8_t OnOff) {
  if (Fram.PcbRev >= 54) {
    if(OnOff) { 
      PcfOut |= (0x01<<6); 
      vTaskResume( TaskPcfHandle ); //apply ASAP
    }
    else {
      PcfOut &= ~(0x01<<6);
      vTaskResume( TaskPcfHandle );
    }
  }
  else { //no mute on PGM_v5.3

  }
}

// 0 - gra, 1 - cisza
void MP3_SetSby(uint8_t OnOff) {
  if (Fram.PcbRev >= 54) {
    if(OnOff) { 
      PcfOut &= ~(0x01<<7);
      vTaskResume( TaskPcfHandle ); //apply ASAP
    }
    else {
      PcfOut |= (0x01<<7);
      vTaskResume( TaskPcfHandle );
    }
  }
  else { 
    if(OnOff) { 
      PcfOut &= ~(0x01<<6);
      vTaskResume( TaskPcfHandle ); //apply ASAP
    }
    else {
      PcfOut |= (0x01<<6);      
      vTaskResume( TaskPcfHandle );
    }
  }
}


static const uint16_t wcPlayPomiar[] = {333, 337};
static const uint16_t wcPlayPomiarKicker[] = {336, 338};

static const uint16_t wcPlayGong[] = {330, 332};
static const uint16_t wcPlayGwizdek[] = {331};
static const uint16_t wcPlayDing[] = {305};

static const uint16_t wcPlayWelcome[]={97,98,98,99,100};

static const uint16_t wcPlayStopien01[]   ={200, 211, 214, 218, 221};
static const uint16_t wcPlayStopien02[]   ={230,231,234,240,250};
static const uint16_t wcPlayStopien03[]   ={260,266,267,270};
static const uint16_t wcPlayStopien04[]   ={280,290};

static const uint16_t wcPlayStopien1[]    ={201,202,212,213,216,217,219,220,223,224,225};
static const uint16_t wcPlayStopien2[]    ={232,233,236,235,236,237,238,239,241,242};
static const uint16_t wcPlayStopien3[]    ={261,262,263,264,265,268,269,271,272};
static const uint16_t wcPlayStopien4[]    ={281,282,283,284,285,286,287,288,289,291,293,294,295};

static const uint16_t wcPlay01Kopacz[]={400,401,402,403,404};

static const uint16_t wcPlayStopien1Kopacz[]={411,412};
static const uint16_t wcPlayStopien2Kopacz[]={420,421};
static const uint16_t wcPlayStopien3Kopacz[]={431,432,433};
static const uint16_t wcPlayStopien4Kopacz[]={441,442};




static const uint16_t MP3_GameStart[] = {106,334};
static const uint16_t MP3_GameStartKopacz[] = {335, 339};
static const uint16_t MP3_WynikL2[] = {1231, 1232, 1233, 1234};
static const uint16_t MP3_WynikL3[] = {1261, 1262, 1263, 1264};
static const uint16_t MP3_WynikL4[] = {1281, 1282, 1283, 1284, 1285, 1286};
static const uint16_t MP3_RekordExtraCredit[] = {1308};


static const uint16_t MP3_Coin[] = {302,303,304};

#define PlayRandomLangMp3AndWait(a) PlayMp3AndWait(a[wTimerRandom%(sizeof(a)/sizeof(a[0]))][Fram.Language]);
#define PlayRandomLangMp3(a) PlayMp3(a[wTimerRandom%(sizeof(a)/sizeof(a[0]))][Fram.Language]);
#define PlayRandomMp3AndWait(a) PlayMp3AndWait(a[wTimerRandom%(sizeof(a)/sizeof(a[0]))]);
#define PlayRandomMp3(a) PlayMp3(a[random((sizeof(a)/sizeof(a[0])))])
#define AddRandomMp3(a) Mp3AddSong(a[random((sizeof(a)/sizeof(a[0])))])


void PlayMp3(uint16_t songNum) {
 
  char filePath[50];
  filePath[49] = 0;
  MP3_SetMute(1);
  vTaskDelay(2);
  if(songNum<1000)  snprintf ( filePath, 20, "mp3/%d.mp3", songNum );
  else snprintf ( filePath, 20, "mp3/%d.mp3", songNum );
  Serial.print("\n PlayMp3 filePath=");
  Serial.println(filePath);
  
  //printf("\n 1=%d", millis());
  
  if( xSemaphoreTake( xSemaphore, ( TickType_t ) 1000 ) == pdTRUE )
  {
    MP3_SetMute(1);
      bool result = audio.connecttoFS(SD_MMC, filePath);
      //printf("\n 4=%d result=%d", millis(), result);      
      xSemaphoreGive( xSemaphore );
      if(result==true) {
        IsMp3Playing = true;
        MP3_SetSby(0);
        MP3_SetMute(0);
        SpiLed.LED1 = 0; //On
      }
      else printf("\n ERROR PlayMp3 %d", songNum);
  }    
  vTaskDelay(2);
  
}

bool Mp3IsPlaying()
{
  static bool previousState = false;
  if(previousState != IsMp3Playing) {
    previousState = IsMp3Playing;
    printf("\n Mp3IsPlaying=%d", IsMp3Playing);
  }
  
  return (IsMp3Playing==true);
}

void AudioInit()
{
  #define I2S_DOUT      25
  #define I2S_BCLK      27
  #define I2S_LRC       26
  #define I2S_MCLK      0
  
  //audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
  //audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_PIN_NO_CHANGE, I2S_MCLK);
  audio.setVolume(Fram.Mp3Volume); // 0...21

  queue = xQueueCreate(5, sizeof(txBuffer)); 
  if (queue == 0)
  {
    printf("Failed to create queue= %p\n", queue);
  }
  else printf("\n xQueueCreated Mp3Volume=%d", Fram.Mp3Volume);

  
 // Semaphore cannot be used before a call to vSemaphoreCreateBinary ().
 // This is a macro so pass the variable in directly.
 vSemaphoreCreateBinary( xSemaphore );

  if( xSemaphore != NULL )
  {
    if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
    {
      xSemaphoreGive( xSemaphore );
    }
  }

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "TaskAudio", /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    (tskIDLE_PRIORITY + 2),           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */  



  PlayRandomMp3(wcPlayWelcome);

}

void Mp3AddSong(uint16_t trackNum) {
  printf("\n Mp3AddSong(%d)", trackNum);
  Mp3Queue.push(trackNum);  
}

void CzekajDoKoncaMp3LubKlawiszStart(void)
{
  for(uint32_t d=0;d<100000;d++) {
    if (false==Mp3IsPlaying()) break;
    if (KEY_START1 || KEY_START2) break;
    UpdateCredit();
  }
}

void Mp3Handle(void)
{
  //printf("\n Mp3Handle = %d idleCnt=%d", Mp3IsPlaying(), idleCnt);
  if (false==Mp3IsPlaying()) {
    if (Mp3Queue.count() > 0) {
      PlayMp3(Mp3Queue.pop());
    }
    MP3_SetSby(1);
    MP3_SetMute(1);
    SpiLed.LED1 = 1; //Off
  }

  if(audio.isRunning()) {
      MP3_SetSby(0);
      MP3_SetMute(0);
      SpiLed.LED1 = 0; //On
  }
}

extern bool QuickStartBoxer;
extern bool QuickStartKopacz;

void PlayMp3AndWait(uint16_t trackNum)
{
  printf("\n PlayMp3AndWait");

  PlayMp3(trackNum);
  while(Mp3IsPlaying()) {
    //printf(".");
    delay(10); //musi tu byc bo glodzimy watek obslugi mp3
    if(QuickStartBoxer==true || QuickStartKopacz==true) break;
    else if(KEY_START1) {
      QuickStartBoxer = true;
      break;
    }
    else if(KEY_START2) {
      QuickStartKopacz = true;
      break;
    }
    UpdateCredit();
  }
  printf("\n End PlayMp3AndWait");
}
