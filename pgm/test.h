extern volatile uint8_t PcfCoin; 
uint16_t CntFilesInDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return 0;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return 0;
    }

    uint16_t numFiles = 0;
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            
        } else {
            numFiles++;
        }
        file = root.openNextFile();
    }
    return numFiles;
}
uint16_t numMp3Files = 0;
void Test(void)
{
    static int Value = 111;
    static uint64_t TestTimer = millis();
    static uint64_t ElektroTimer = millis();
    static bool oneTime = true;


    if(oneTime) {
      oneTime = false;
      numMp3Files = CntFilesInDir(SD_MMC, "/mp3", 0);
    }
    
    if (millis() - TestTimer > 600)
    {
        TestTimer = millis();

        static bool blink = 0;
        blink = !blink;
        if(blink) Zar.All = 0x55555555;
        else Zar.All = 0xaaaaaaaa;
        
        
        Value += 111;
        if (Value>1000) Value = 111;
        DisplayWynik(Value);

        DisplayPlayer(1, Value);
        DisplayPlayer(2, Value);
        DisplayPlayer(3, Value);
 

        CRGB colours[] = {CRGB(0, 0, 0), CRGB(50, 0, 0), CRGB(0, 50, 0), CRGB(0, 0, 50)};
        static int i = 0;
        i++;
        fill_solid(&leds3[0], 106, CRGB::Red);
        fill_solid(&leds3[106], 106, CRGB::Green);
        fill_solid(&leds3[106*2], 106, CRGB::Blue);
        fill_solid(&leds3[106*3], 106, CRGB::Red);
        fill_solid(leds2, num_leds2, colours[(i+1) % 4]);
        fill_solid(leds1, num_leds1, colours[(i+2) % 4]);

        if (false==Mp3IsPlaying()) PlayMp3(108);

        printf("\n PcfCoin=%X", PcfCoin);
        uint8_t notCoin = ~PcfCoin;
        printf("\n notCoin=%X", PcfCoin);

    }

    if(KEY_DOWN) {
        printf("\n KEY_DOWN");
        Zar.Z18_MOTOR3 = 1;
        UpdateSPI();  
        delay(1000);
        Zar.Z18_MOTOR3 = 0;
        UpdateSPI();  
    }

    if(KEY_UP) {
      TicketOut = 1;     
    }

    if(KEY_START1) {
        CLR_GRUSZKA;
        Zar.Z16_MOTOR1 = 1;
        UpdateSPI();  
        delay(200);
        SET_GRUSZKA;
        delay(1000);
        Zar.Z16_MOTOR1 = 0;
        UpdateSPI();  
    }

    if(KEY_START2) {
        CLR_GRUSZKA2;
        Zar.Z17_MOTOR2 = 1;
        UpdateSPI();  
        delay(200);
        SET_GRUSZKA2;
        delay(1000);
        Zar.Z17_MOTOR2 = 0;
        UpdateSPI();  
    }
    uint8_t sensorVal = 0;
    if(0==digitalRead(SENS0_PIN)) sensorVal += 1;
    if(0==digitalRead(SENS1_PIN)) sensorVal += 2;
    if(0==digitalRead(SENS2_PIN)) sensorVal += 10;
    if(0==digitalRead(SENS3_PIN)) sensorVal += 20;
    if(0==digitalRead(SENS4_PIN)) sensorVal += 40;
    if(~PcfGP&(0x01<<2)) sensorVal += 100;
    else sensorVal += 200;

    DisplayRekord(sensorVal);

    uint8_t notCoint = (((~PcfCoin) & 0x0F) | ((((~PcfCoin) & 0xF0) >> 4)*10) );
    DisplayCredit(notCoint);
}
