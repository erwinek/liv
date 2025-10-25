#include <FastLED.h>

#define NUM_LEDS1       12 //daszek
#define NUM_LEDS1_MONSTER 25 //daszek monster
#define NUM_LEDS2       12 + 16 + 8//czacha i bramka
#define NUM_LEDS2_COMBAT  14 + 11 + 11 //czacha i LEWY + PRAWY BOK
#define NUM_LEDS2_COMBAT_KIDS  14 + 9 + 9 //czacha i LEWY + PRAWY BOK
#define NUM_LEDS2_MONSTER  14 + 16 + 12 + 16 //czacha, bramka 16, mlot 12 + 16 mlot scianka
#define NUM_LEDS2_DOUBLE_HIT  4 + 12 + 4 + 20 //4-front, 12 czacha, 4 front, 12 bramka
#define NUM_LEDS3       33 + 41 + 33 + 41 
#define NUM_LEDS3_KIDS  33 + 25 + 33 + 25 //kids - mniejsza ramka LED RGB
#define NUM_LEDS3_COMBAT  70 + 70 //COMBAT - 2 paski z przodu po 70 LED
#define NUM_LEDS3_FIST  106 + 106 + 106 + 106 //FIST - 4 paski z przodu po 106 LED
#define NUM_LEDS3_COMBAT_KIDS  63 + 63 //COMBAT - 2 paski z przodu po 70 LED
#define NUM_LEDS3_MONSTER  87 + 87 //Monster - 2 paski za mlotem po 87 LED


uint16_t num_leds3 = NUM_LEDS3_FIST;
uint16_t num_leds2 = NUM_LEDS2;
uint16_t num_leds1 = NUM_LEDS1;
uint8_t num_led_czacha = 12;

volatile uint32_t dummy0 = 0x55aa55aa;
CRGB leds1[NUM_LEDS1_MONSTER+10];
volatile uint32_t dummy1 = 0x55aa55aa;
CRGB leds2[NUM_LEDS2_MONSTER+10];
volatile uint32_t dummy2 = 0x55aa55aa;
CRGB leds3[NUM_LEDS3_FIST+10];
volatile uint32_t dummy3 = 0x55aa55aa;

void StroboScope(CRGB *leds, int num_leds, CRGB color);
void StroboScope2(CRGB *leds, int num_leds, CRGB color);
void fadeInOutThree(CRGB *leds, int num_leds);
void fadeInOutWhite(CRGB *leds, int num_leds);
void dot_beat(CRGB *leds, int NUM_LEDS);
void NightRiderRGB(CRGB *leds, int NUM_LEDS);

uint32_t LedRgbCnt = 0;
uint8_t LiczbaLedCzacha = 12;
uint8_t LiczbaLedBramka = 12;
uint8_t LiczbaLedDaszek = 12;
uint8_t LiczbaLedBoki = 0;
uint8_t LiczbaLedMlot = 0;
uint8_t LiczbaLedMlotScianka = 0;
uint8_t LiczbaLedFrontLeft = 0;
uint8_t LiczbaLedFrontRight = 0;

#define LED_DEBUG_INTERVAL 100  // ms between updates

void SendLed2StateToSerial() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < LED_DEBUG_INTERVAL) return;
    lastUpdate = millis();
    
    // Wysyłamy dane w formacie: "LED2:<r1>,<g1>,<b1>;<r2>,<g2>,<b2>;...;\n"
    Serial.print("LED2:");
    for(int i = 0; i < num_leds2; i++) {
        Serial.printf("%d,%d,%d;", 
            leds2[i].r,
            leds2[i].g, 
            leds2[i].b
        );
    }
    Serial.println();
}

TaskHandle_t TaskLedHandle = NULL;
void TaskLed( void * pvParameters ) {
  for(;;) {
    vTaskDelay(20);
    FastLED.show();
    //SendLed2StateToSerial();  
    assert(dummy0 == 0x55aa55aa);
    assert(dummy1 == 0x55aa55aa);
    assert(dummy2 == 0x55aa55aa);
    assert(dummy3 == 0x55aa55aa);
  }
}

void LedInit(void)
{

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
  else if (Fram.BoxerModel==FIST) {
    num_leds2 = NUM_LEDS2_COMBAT;
    num_leds3 = NUM_LEDS3_FIST;
    LiczbaLedCzacha = 14;
    LiczbaLedBoki = 11;
  }
  else if (Fram.BoxerModel==MONSTER) {
    num_leds3 = NUM_LEDS3_MONSTER;
    num_leds2 = NUM_LEDS2_MONSTER;
    num_leds1 = NUM_LEDS1_MONSTER;
    LiczbaLedCzacha = 14;
    LiczbaLedDaszek = 25;
    LiczbaLedBoki = 0;
    LiczbaLedMlot = 12;
    LiczbaLedMlotScianka = 16;
  }
  else if (Fram.BoxerModel==DOUBLE_HIT) {
    num_leds3 = 27 + 45 + 27; //wkolo bramki
    num_leds2 = NUM_LEDS2_DOUBLE_HIT;
    num_leds1 = NUM_LEDS1;
    LiczbaLedCzacha = 12;
    LiczbaLedDaszek = 12;
    LiczbaLedBoki = 0;
    LiczbaLedFrontLeft = 4;
    LiczbaLedFrontRight = 4;
    LiczbaLedBramka = 5+10+5;
  }
  else if (Fram.BoxerModel==DOUBLE_HIT_GIFT_KIDS_ESPANIOL) {
    LiczbaLedCzacha = 12;
    LiczbaLedDaszek = 12;
    LiczbaLedBramka = 16;
    LiczbaLedBoki = 0;
    num_leds1 = LiczbaLedDaszek;
    num_leds2 = LiczbaLedBramka + LiczbaLedCzacha;
  }
  printf("\n LedInit num_leds1=%d num_leds2=%d num_leds3=%d LiczbaLedCzacha=%d LiczbaLedBoki=%d", num_leds1, num_leds2, num_leds3, LiczbaLedCzacha, LiczbaLedBoki);
  if (Fram.BoxerModel==FIST) FastLED.setBrightness(70);
	
    //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    TaskLed,   /* Task function. */
                    "TaskLedH", /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    (tskIDLE_PRIORITY + 3),           /* priority of the task */
                    &TaskLedHandle,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */  
}


// Efekt 3: Juggle - kilka kolorowych kropek
void effect_juggle(CRGB *leds, int num_leds) {
  fadeToBlackBy(leds, num_leds, 20);
  byte dothue = 0;
  for(int i = 0; i < 8; i++) {
    int pos = beatsin16(i + 7, 0, num_leds - 1);
    leds[pos] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

static uint8_t gHue = 0; // rotating "base color" used by many of the effects

// Efekt 6: Theater Chase - przesuwające się grupy
void effect_theater_chase(CRGB *leds, int num_leds, uint8_t gHue) {
  static int effectCounter=0;
  
  
  int offset = (effectCounter / 3) % 3;
  for(int i = 0; i < num_leds; i++) {
    if((i + offset) % 3 == 0) {
      leds[i] = CHSV(gHue, 255, 255);
    } else {
      leds[i] = CRGB::Black;
    }
  }

  effectCounter++;
}

void circleLedStr2(void) {

  //printf("\n circleLedStr2 LiczbaLedBoki=%d LiczbaLedBramka=%d LiczbaLedCzacha=%d", LiczbaLedBoki, LiczbaLedBramka, LiczbaLedCzacha);

  if(Fram.BoxerModel==MONSTER) {
    static uint8_t starthue = 0;
    LiczbaLedMlot = 12;
    fill_rainbow(&leds2[LiczbaLedCzacha+LiczbaLedBramka], LiczbaLedMlot, starthue, 30);
    starthue += 7;
  }

  if(LiczbaLedBoki==0) {
    
    static int cntBramka = 12;
    CRGB colourArray[] = {CRGB::Blue, CRGB::Red, CRGB::Green};
    static int colourCnt = 0;
    colourCnt++;
    uint16_t ledStartCzacha = LiczbaLedFrontLeft;
    static int cnt = ledStartCzacha ;

    if (cnt < (LiczbaLedCzacha+ledStartCzacha)) cnt++;
    else {
      for(int i = 0; i<LiczbaLedCzacha;i++) leds2[i+LiczbaLedFrontLeft] = CRGB::Black;
      cnt = ledStartCzacha;
    }

    if (cntBramka < (LiczbaLedCzacha+LiczbaLedBramka-1)) cntBramka++;
    else {
      uint16_t ledStartBramka = LiczbaLedFrontLeft + LiczbaLedCzacha + LiczbaLedFrontRight;
      for(int i = ledStartBramka; i < (ledStartBramka + LiczbaLedBramka);i++) leds2[i] = CRGB::Black;
      cntBramka = ledStartBramka;
    }

   if(Fram.BoxerMat==1) {
      leds2[cnt] = CRGB::White;
      leds2[cntBramka] = CRGB::White;
    }
    else {
      effect_theater_chase(&leds2[LiczbaLedFrontLeft], LiczbaLedCzacha, gHue);
      effect_theater_chase(&leds2[LiczbaLedFrontLeft+LiczbaLedCzacha+LiczbaLedFrontRight], LiczbaLedBramka, gHue+128);
      gHue += 5;

      for(int i = 0; i<4; i++) { //clear front
        leds2[i] = 0;
        leds2[i+LiczbaLedFrontLeft+LiczbaLedCzacha] = 0;
      }
      for(int i = cnt%2; i<4; i+=2) { //purple migacz na front
        leds2[i] = CRGB::Purple;
        leds2[i+LiczbaLedFrontLeft+LiczbaLedCzacha] = CRGB::Purple;
      }
    }
  }
}

void NightRiderGreen(CRGB *leds, int NUM_LEDS)
{
  //printf("\n NightRiderGreen");
  fill_solid( leds, NUM_LEDS, CRGB::Black);
  
  uint8_t center = beatsin8(40, 3, (NUM_LEDS-4));
  leds[center - 3] = CRGB(0,100,0);
  leds[center - 2] = CRGB(0,150,0);
  leds[center - 1] = CRGB(0,200,0);
  leds[center]     = CRGB(0,255,0);
  leds[center + 1] = CRGB(0,200,0);
  leds[center + 2] = CRGB(0,150,0);
  leds[center + 3] = CRGB(0,100,0);

  center = beatsin8(40, 3, (NUM_LEDS-4), 0, 90);
  leds[center - 3] = CRGB(0,100,0);
  leds[center - 2] = CRGB(0,150,0);
  leds[center - 1] = CRGB(0,200,0);
  leds[center]     = CRGB(0,255,0);
  leds[center + 1] = CRGB(0,200,0);
  leds[center + 2] = CRGB(0,150,0);
  leds[center + 3] = CRGB(0,100,0);

  center = beatsin8(40, 3, (NUM_LEDS-4), 0, 180);
  leds[center - 3] = CRGB(0,100,0);
  leds[center - 2] = CRGB(0,150,0);
  leds[center - 1] = CRGB(0,200,0);
  leds[center]     = CRGB(0,255,0);
  leds[center + 1] = CRGB(0,200,0);
  leds[center + 2] = CRGB(0,150,0);
  leds[center + 3] = CRGB(0,100,0);
}


void LedGameStart(void) {
  //printf("\n LedGameStart");
  static uint16_t startPosition = 0;
  static const uint16_t LEN = 6;
  static uint8_t starthue = 0;
  uint8_t PLUM_SRODEK = 41+33;
  
  static uint8_t colCnt = 0;
  if(Fram.BoxerModel == KIDS) PLUM_SRODEK = 33 + 25;
  if(Fram.BoxerModel == COMBAT) {
    LiczbaLedBramka = 11;
    PLUM_SRODEK = 69;
    LiczbaLedCzacha = 14;    
  } 
  else if(Fram.BoxerModel == COMBAT_KIDS) {
    LiczbaLedBramka = 9;
    LiczbaLedCzacha = 14;
    PLUM_SRODEK = 62;
  } 
  else if((Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS ) || (Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS_ESPANIOL )) {
    LiczbaLedBramka = 16;
    LiczbaLedCzacha = 12;
    PLUM_SRODEK = 62;
  } 
  else if(Fram.BoxerModel == FIST) {
    LiczbaLedBramka = 16;
    LiczbaLedCzacha = 12;
    PLUM_SRODEK = 106;
  }
  else if(Fram.BoxerModel == MONSTER) {
    LiczbaLedBramka = 16;
    LiczbaLedCzacha = 14;
    PLUM_SRODEK = 87;
  } 
  else if(Fram.BoxerModel == DOUBLE_HIT) {
    LiczbaLedBramka = 20;
    LiczbaLedCzacha = 12;
    PLUM_SRODEK = 87;    
  }  
  
  static int licznikPlum = PLUM_SRODEK;
  
  static bool kierunek = true;

  CRGB startColour = CRGB(0,1,0); 
  CRGB endColour = CRGB(0,255,0);

  CRGB startColourRun[] = {CRGB::Green, CRGB::Red, CRGB::Blue};

  //biegajace swiatelko na pasku RGB
  if(startPosition < (num_leds3-LEN-1)) startPosition+=2;
  else startPosition = 0;

  fill_solid(leds1, num_leds1, CRGB(0,0,0));
  fill_solid(leds2, LiczbaLedCzacha, CRGB(0,0,0));
  fill_rainbow(&leds2[LiczbaLedCzacha+LiczbaLedFrontLeft+LiczbaLedFrontRight], LiczbaLedBramka, starthue, 30);
  if(LiczbaLedFrontLeft>0) fill_rainbow(&leds2[0], LiczbaLedFrontLeft, starthue, 30);
  if(LiczbaLedFrontRight>0) fill_rainbow(&leds2[LiczbaLedCzacha+LiczbaLedFrontLeft], LiczbaLedFrontRight, starthue, 30);
  fill_solid(leds3, num_leds3, CRGB(0,0,0));
  starthue+=10;

  if (Fram.BoxerModel == FIST) fadeLightBy(leds3, num_leds3, 50);

  //strzalka na START
  static uint8_t licznik6 = 0;
  static uint8_t licznik12 = 0;
  static uint64_t timestamp = millis();
  if (millis()-timestamp > 50) {
    timestamp = millis();
    if (licznik6 < (LiczbaLedCzacha/2)) licznik6++;
    else licznik6 = 0;

    //bramka
    if(kierunek==true) {
      if (licznik12 < LiczbaLedBramka) licznik12++;
      else {
        kierunek = false;
        colCnt++;
      }
    }
    else {
      if (licznik12 > 0) licznik12--;
      else {
        kierunek = true;
        colCnt++;
      }
    }
  }
  //czacha
  if(Fram.BoxerMat==1) {
    leds2[licznik6] = CRGB::White;
    leds2[(LiczbaLedCzacha-1)-licznik6] = CRGB::White;
    leds2[LiczbaLedCzacha + licznik12] = CRGB::Black; 
    leds2[LiczbaLedCzacha + LiczbaLedBramka + licznik12] = CRGB::White;
    //fadeInOutWhite na daszku
    fadeInOutWhite(leds1, num_leds1);

    //Plum Kwadrat //LED RGB w kolo ekranu to pasek leds3
    if(licznikPlum < (PLUM_SRODEK-2)) licznikPlum++;
    else licznikPlum = 0;

    leds3[PLUM_SRODEK+licznikPlum] = CRGB::White;
    leds3[PLUM_SRODEK+licznikPlum+1] =CRGB::White;
    leds3[PLUM_SRODEK+licznikPlum+2] = CRGB::White;
    leds3[PLUM_SRODEK-licznikPlum] = CRGB::White;
    leds3[PLUM_SRODEK-licznikPlum-1] = CRGB::White;
    leds3[PLUM_SRODEK-licznikPlum-2] = CRGB::White;
  }
  else {
    fill_solid(&leds2[LiczbaLedFrontLeft], LiczbaLedCzacha, CRGB(0,0,0));
    leds2[LiczbaLedFrontLeft + licznik6] = CRGB::Green;
    leds2[LiczbaLedFrontLeft + (LiczbaLedCzacha-1)-licznik6] = CRGB::Green;

    leds2[LiczbaLedFrontLeft + LiczbaLedCzacha + LiczbaLedFrontRight + licznik12] = CRGB::Black; 
    leds2[LiczbaLedFrontLeft + LiczbaLedCzacha + LiczbaLedBramka + licznik12] = startColourRun[(colCnt+1)%3];
    //tecza na daszku
    fill_rainbow( leds1, num_leds1, starthue, 30);
    starthue+=3;

    //Plum Kwadrat //LED RGB w kolo ekranu to pasek leds3
    if(licznikPlum < (PLUM_SRODEK-2)) licznikPlum++;
    else licznikPlum = 0;
    
    if (Fram.BoxerModel==FIST) {
      dot_beat(&leds3[106], 106);
      dot_beat(&leds3[106*2], 106);
      NightRiderRGB(&leds3[0], 106);
      NightRiderRGB(&leds3[106*3], 106);
      if (Fram.BoxerModel == FIST) fadeLightBy(leds3, num_leds3, 70);
    }
    else if (Fram.BoxerModel==MONSTER) {
      static uint8_t gHue = 128;
      dot_beat(&leds3[0], 87);
      dot_beat(&leds3[87], 87);
      effect_theater_chase(&leds2[LiczbaLedBramka+LiczbaLedCzacha+LiczbaLedMlot], LiczbaLedMlotScianka, gHue);
      gHue += 5;
    }
    else if (Fram.BoxerModel==DOUBLE_HIT) {
      effect_juggle(leds3, num_leds3);
    }
    
    else {
      leds3[PLUM_SRODEK+licznikPlum] = CRGB::Red;
      leds3[PLUM_SRODEK+licznikPlum+1] = CRGB::Red;
      leds3[PLUM_SRODEK+licznikPlum+2] = CRGB::Red;
      leds3[PLUM_SRODEK-licznikPlum] = CRGB::Red;
      leds3[PLUM_SRODEK-licznikPlum-1] = CRGB::Red;
      leds3[PLUM_SRODEK-licznikPlum-2] = CRGB::Red;
    }
  }
}

void whiteSnake(CRGB *leds, int num_leds) {
  static uint64_t timestamp = millis();
  if(millis() - timestamp > 30) {
    timestamp = millis();
    static uint8_t pos = 0;
    static bool direction = true;

    if(direction) {
      leds[pos] = CRGB::White;
      pos++;
      if(pos >= num_leds) {
        direction = false;
        pos = num_leds - 1;
      }
    }
    else {
      leds[pos] = CRGB::Black;
      pos--;
      if(pos == 0) {
        direction = true;
        pos = 0;
      }
    }
  }

}

void fadeInOut(CRGB *leds, int num_leds, uint8_t col)
{
  //printf("\n fadeInOut");
  static uint8_t n = 0;
  static bool direction = 0;
  uint8_t color[3] = {0,0,0};
  const uint8_t MAX_VAL = 200;

  if(!direction)
  {
    if(n < MAX_VAL) {
      color[col] = n;
      fill_solid(leds, num_leds, CRGB(color[0], color[1], color[2]));
      n += 5;
    }
    else {
      direction = 1;
    }
  }
  else
  {
    if(n > 0) {
      color[col] = n;
      fill_solid(leds, num_leds, CRGB(color[0], color[1], color[2]));
      n -= 5;
    }
    else {
      direction = 0;
    }
  }
}

void LedNightRiderWhite(CRGB *leds, int num_leds) {
  static int pos = 0;        // Aktualna pozycja "oka"
  static int direction = 1;  // Kierunek ruchu: 1 = w prawo, -1 = w lewo
  // Wygaszanie wszystkich LEDów (tworzy efekt smug)
  fadeToBlackBy(leds, num_leds, 20);

  // Zapalenie aktualnej pozycji na czerwono
  leds[pos] = CRGB::White;

  // Przesunięcie pozycji
  pos += direction;

  // Odbicie od końców
  if (pos == num_leds - 1 || pos == 0) {
    direction *= -1;
  }
}

void LedPercent(uint16_t value)
{
  static uint8_t starthue = 0;
  fill_solid( leds3, num_leds3, CRGB(0,0,0));    

  uint8_t MAX_PEAK = 41;
  
  uint16_t peak = (value * MAX_PEAK) / 100;
  
  if ((Fram.BoxerModel == HIT) || (Fram.BoxerModel == GIFT) || (Fram.BoxerModel == GIFT_ESPANIOL)) {
    peak = (value * MAX_PEAK) / 100;
    fill_rainbow( &leds3[33], peak, starthue, 20);
    fill_rainbow( &leds3[33+41+33+(41-peak)], peak, starthue, 20);
    fadeInOutThree(leds3, 33);
    fadeInOutThree(&leds3[33+41], 33);
  }
  else if (Fram.BoxerModel == KIDS) {
  	MAX_PEAK = 25;
  	peak = (value * MAX_PEAK) / 100;
    fill_rainbow( &leds3[33], peak, starthue, 20);
    fill_rainbow( &leds3[33+25+33+(25-peak)], peak, starthue, 20);
    fadeInOutThree(leds3, 33);
    fadeInOutThree(&leds3[33+25], 33);
  }
  else if (Fram.BoxerModel == COMBAT) {
  	MAX_PEAK = 70;   //Combat
  	peak = (value * MAX_PEAK) / 100;
    fill_rainbow( &leds3[70], peak, 160, 3);
    fill_rainbow( &leds3[(70-peak)], peak, 160, 3);
  }
  else if (Fram.BoxerModel == COMBAT_KIDS) {
  	MAX_PEAK = 63;   //Combat
  	peak = (value * MAX_PEAK) / 100;
    fill_rainbow( &leds3[63], peak, 160, 3);
    fill_rainbow( &leds3[(63-peak)], peak, 160, 3);
  }   
  else if (Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS) {
  	MAX_PEAK = 25;
  	peak = (value * MAX_PEAK) / 100;
    fill_rainbow( &leds3[33], peak, starthue, 20);
    fill_rainbow( &leds3[33+25+33+(25-peak)], peak, starthue, 20);
    fadeInOutThree(leds3, 33);
    fadeInOutThree(&leds3[33+25], 33);
  }
  else if (Fram.BoxerModel == MONSTER) {
  	MAX_PEAK = NUM_LEDS3_MONSTER/2;   //Combat
  	peak = (value * MAX_PEAK) / 150;
    fill_solid( leds3, peak, CRGB(128,128,128));
    fill_solid( &leds3[NUM_LEDS3_MONSTER-peak], peak, CRGB(128,128,128));
  }
  else if (Fram.BoxerModel == FIST) {
  	MAX_PEAK = 106;
  	peak = (value * MAX_PEAK) / 100;
    
    fill_rainbow( &leds3[106], peak, starthue, 20);
    fill_rainbow( &leds3[((106*3)-peak)], peak, starthue, 20);
    fadeLightBy(leds3, num_leds3, 50);
    
    static bool onOff = true;
    // Turn on every other LED
    for (int i = 0; i < 106; i++)
    {
      if (i % 2 == onOff) leds3[i] = CHSV(starthue, 255, 255);
      else leds3[i] = CRGB::Black;
    }
    onOff = !onOff;
    for (int i = 106*3; i < 106*4; i++)
    {
      if (i % 2 != onOff) leds3[i] = CHSV(starthue, 255, 255);
      else leds3[i] = CRGB::Black;
    }
  }
  else if (Fram.BoxerModel == DOUBLE_HIT) {
  	MAX_PEAK = 27;
  	peak = (value * MAX_PEAK) / 100;

    fill_rainbow( &leds3[0], peak, starthue, 20);
    fill_rainbow( &leds3[((27+45+27)-peak)], peak, starthue, 20);

    static bool onOff = true;
    // Turn on every other LED
    for (int i = 27; i < (27+45); i++)
    {
      if (i % 2 == onOff) leds3[i] = CHSV(starthue, 255, 255);
      else leds3[i] = CRGB::Black;
    }
    onOff = !onOff;
    for (int i = 106*3; i < 106*4; i++)
    {
      if (i % 2 != onOff) leds3[i] = CHSV(starthue, 255, 255);
      else leds3[i] = CRGB::Black;
    }
  }

  starthue+=5;  
}

void LedPercent2(uint16_t value)
{
  
  static bool stroboskop = false;
  static uint64_t timestamp = millis();
  static uint8_t starthue = 0;
  uint8_t MAX_PEAK = 11; //Combat
  if (Fram.BoxerModel == COMBAT) MAX_PEAK = 11;
  else if (Fram.BoxerModel == COMBAT_KIDS) MAX_PEAK = 9;
  
  fill_solid( leds2, num_leds2, CRGB(0,0,0));    

  if(Fram.BoxerModel == COMBAT_KIDS) {
    fill_rainbow( leds2, LiczbaLedCzacha, starthue, 20);
    starthue += 5;
    if(millis() - timestamp > 100) {
      timestamp = millis();
      static uint8_t cntBlack = 0;
      cntBlack++;
      leds2[cntBlack%(LiczbaLedCzacha/2)] = 0;
      leds2[LiczbaLedCzacha-1-(cntBlack%(LiczbaLedCzacha/2))] = 0;
    }
  }
  else {
    if(millis() - timestamp > 100) {
      timestamp = millis();
      stroboskop = !stroboskop;
      if (stroboskop) fill_solid( leds2, LiczbaLedCzacha, CRGB::White); 
      else fill_solid( leds2, LiczbaLedCzacha, CRGB::Black); 
    }
  }
  
  uint16_t peak = 1 + ((value * MAX_PEAK) / 100);

  if (Fram.BoxerModel==COMBAT_KIDS) {
    LiczbaLedCzacha = 14;
    LiczbaLedBoki = 9;
  }

  if(LiczbaLedBoki>0) {  

    CRGB colArray[11] = {CRGB::Blue, CRGB::BlueViolet, CRGB::Brown, CRGB::Chartreuse, CRGB::DeepPink,
                      CRGB::Chocolate, CRGB::DarkGoldenrod, CRGB::Crimson, CRGB::DarkMagenta, CRGB::Fuchsia};
 	  peak = (value * MAX_PEAK) / 100;
    
    fill_solid( &leds2[LiczbaLedCzacha+LiczbaLedBoki], peak, colArray[peak]);    
    fill_solid( &leds2[(LiczbaLedCzacha+LiczbaLedBoki-peak)], peak, colArray[peak]);    
  }
  else 
  starthue+=5;  

}


void LedRunThree(CRGB *leds, int NUM_LEDS) {

  static uint16_t cnt = 0;
  static bool dirRight = true;
  uint64_t rnd = millis();
  static uint64_t timestamp = millis();
  static uint8_t r = 0;
  static uint8_t g = 85;
  static uint8_t b = 170;
  r++; g++; b++;
  
  if ((rnd - timestamp) > 200) {
    CRGB color = CRGB(r, g, b);
    timestamp = rnd;

    color = CRGB(r, g, b);

    fill_solid(leds, NUM_LEDS, CRGB(0,0,0));

    if(dirRight==true) {
      leds[cnt] = color;
      leds[NUM_LEDS-1-cnt] = color;
      if(cnt < (NUM_LEDS/2)) cnt++;
      else {
        dirRight = false;
        
      }
    }
    else {
      leds[cnt] = color;
      leds[NUM_LEDS-1-cnt] = color;
      if(cnt > 0) cnt--;
      else dirRight = true;
    }
  }
}

void NightRiderRGB(CRGB *leds, int NUM_LEDS) {
  
  //printf("\n NightRiderRGB");
  fill_solid( leds, NUM_LEDS, CRGB::Black);
  
  uint8_t center = beatsin8(20, 3, (NUM_LEDS-4));
  leds[center - 3] += CRGB(100,0,0);
  leds[center - 2] += CRGB(150,0,0);
  leds[center - 1] += CRGB(200,0,0);
  leds[center]     += CRGB(255,0,0);
  leds[center + 1] += CRGB(200,0,0);
  leds[center + 2] += CRGB(150,0,0);
  leds[center + 3] += CRGB(100,0,0);


  center = beatsin8(20, 3, (NUM_LEDS-4), 0, 90);  
  leds[center - 3] += CRGB(0,100,0);
  leds[center - 2] += CRGB(0,150,0);
  leds[center - 1] += CRGB(0,200,0);
  leds[center]     += CRGB(0,255,0);
  leds[center + 1] += CRGB(0,200,0);
  leds[center + 2] += CRGB(0,150,0);
  leds[center + 3] += CRGB(0,100,0);

  center = beatsin8(20, 3, (NUM_LEDS-4), 0, 180);  
  leds[center - 3] += CRGB(0,0,100);
  leds[center - 2] += CRGB(0,0,150);
  leds[center - 1] += CRGB(0,0,200);
  leds[center]     += CRGB(0,0,255);
  leds[center + 1] += CRGB(0,0,200);
  leds[center + 2] += CRGB(0,0,150);
  leds[center + 3] += CRGB(0,0,100);

}

void StroboScope1on3(CRGB *leds, int num_leds, CRGB color)
{
    static uint64_t timestamp = millis();
    static bool OnOff = false;
    if(millis() - timestamp > 200) {
        timestamp = millis();
        OnOff = !OnOff;
        if(OnOff) for (int i=0;i<num_leds;i+=3) leds[i] = color; 
        else fill_solid(leds, num_leds, CRGB(0, 0, 0)); 
    }
}


void StroboScope(CRGB *leds, int num_leds, CRGB color)
{
    static uint64_t timestamp = millis();
    static bool OnOff = false;
    if(millis() - timestamp > 200) {
        timestamp = millis();
        OnOff = !OnOff;
        if(OnOff) fill_solid(leds, num_leds, color); 
        else fill_solid(leds, num_leds, CRGB(0, 0, 0)); 
    }
}

void StroboScope2(CRGB *leds, int num_leds, CRGB color)
{
    static uint64_t timestamp = millis();
    static bool OnOff = false;
    if(millis() - timestamp > 200) {
        timestamp = millis();
        OnOff = !OnOff;
        if(OnOff) fill_solid(leds, num_leds, color); 
        else fill_solid(leds, num_leds, CRGB(0, 0, 0)); 
    }
}

void StroboScope3Player(CRGB *leds, int num_leds, CRGB color)
{
    static uint64_t timestamp = millis();
    static bool OnOff = false;
    if(millis() - timestamp > 200) {
        timestamp = millis();
        OnOff = !OnOff;
        if(OnOff) for (int i=0;i<num_leds; i+=3) leds[i] = color; 
        else fill_solid(leds, num_leds, CRGB(0, 0, 0)); 
    }
}
void fadeInOutThree(CRGB *leds, int num_leds)
{
  //printf("\n fadeInOutThree");
  static uint8_t n = 0;
  static bool direction = 0;
  uint8_t color[3] = {0,0,0};
  static uint8_t col = 0;
  const uint8_t MAX_VAL = 200;

  if(!direction)
  {
    if(n < MAX_VAL) {
      color[col] = n;
      fill_solid(leds, num_leds, CRGB(color[0], color[1], color[2]));
      n += 5;
    }
    else {
      direction = 1;
    }
  }
  else
  {
    if(n > 0) {
      color[col] = n;
      fill_solid(leds, num_leds, CRGB(color[0], color[1], color[2]));
      n -= 5;
    }
    else {
      direction = 0;
      if(col<2) col++;
      else col = 0;
    }
  }  
}

void fadeInOutWhite(CRGB *leds, int num_leds)
{
  //printf("\n fadeInOutThree");
  static uint8_t n = 0;
  static bool direction = 0;
  const uint8_t MAX_VAL = 200;

  if(!direction)
  {
    if(n < MAX_VAL) {
      fill_solid(leds, num_leds, CRGB(n, n, n));
      n += 5;
    }
    else {
      direction = 1;
    }
  }
  else
  {
    if(n > 0) {
      fill_solid(leds, num_leds, CRGB(n, n, n));
      n -= 5;
    }
    else {
      direction = 0;
    }
  }  
}


// Define variables used by the sequences.
int   thisdelay =   10;                                       // A delay value for the sequence(s)
uint8_t   count =   0;                                        // Count up to 255 and then reverts to 0
uint8_t fadeval = 224;                                        // Trail behind the LED's. Lower => faster fade.
uint8_t bpm = 30;
void dot_beat(CRGB *leds, int NUM_LEDS) {

  uint8_t inner = beatsin8(bpm, NUM_LEDS/4, NUM_LEDS/4*3);    // Move 1/4 to 3/4
  uint8_t outer = beatsin8(bpm, 0, NUM_LEDS-1);               // Move entire length
  uint8_t middle = beatsin8(bpm, NUM_LEDS/3, NUM_LEDS/3*2);   // Move 1/3 to 2/3
  
  if (Fram.BoxerModel==MONSTER) {
    if(middle<NUM_LEDS) leds[middle] = CRGB::White;
    if(inner<NUM_LEDS) leds[inner] = CRGB::White;
    if(outer<NUM_LEDS) leds[outer] = CRGB::White;
  }
  else {
    if(middle<NUM_LEDS) leds[middle] = CRGB::Purple;
    if(inner<NUM_LEDS) leds[inner] = CRGB::Blue;
    if(outer<NUM_LEDS) leds[outer] = CRGB::Aqua;
  }

  nscale8(leds,NUM_LEDS,fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);

} // dot_beat()

void dot_beat16(CRGB *leds, int NUM_LEDS) {

  uint16_t inner = beatsin16(bpm, NUM_LEDS/4, NUM_LEDS/4*3);    // Move 1/4 to 3/4
  uint16_t outer = beatsin16(bpm, 0, NUM_LEDS-1);               // Move entire length
  uint16_t middle = beatsin16(bpm, NUM_LEDS/3, NUM_LEDS/3*2);   // Move 1/3 to 2/3
  

  if(middle<NUM_LEDS) leds[middle] = CRGB::Purple;
  if(inner<NUM_LEDS) leds[inner] = CRGB::Blue;
  if(outer<NUM_LEDS) leds[outer] = CRGB::Aqua;

  nscale8(leds,NUM_LEDS,fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);

}
