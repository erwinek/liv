#define BEZ_NAGROD				16000
#define TOLERANCJA				20


//czekaj 10s, lub przelot przez ticket1, lub 7s i switch na silniku (key4)
void RunEngineUntilGift(uint8_t nrNagrody)
{
	int i;

	static uint32_t rnd = millis();

	rnd += millis();

	Zar.Z22_SKRZYNIA = 1;

	switch(nrNagrody) {
		case(1):
   			Zar.Z16_MOTOR1 = 1;
			UpdateSPI();
			PlayRandomLangMp3(wcPlayNagroda2Language);
		break;
		case(2):
   			Zar.Z17_MOTOR2 = 1;
			UpdateSPI();
			PlayRandomLangMp3(wcPlayNagroda3Language);
		break;
		case(3):
   			Zar.Z18_MOTOR3 = 1;
			UpdateSPI();
			PlayRandomLangMp3(wcPlayNagroda4Language);
		break;
	}

	for(i=0;i<15000;i++) { 
		delay(1);
        if ((PcfGP) & (0x01 << 2)) { //sensor Gift
            break;
        }
		static uint64_t strobo = millis();
		if (millis() - strobo > 100) {
			strobo = millis();
			static bool state = 1;
			state = !state;
			if (state) fill_solid( leds3, NUM_LEDS3, CRGB(100,100,100));    
			else fill_solid( leds3, NUM_LEDS3, CRGB(0,0,0));    
		}
		static uint32_t timestamp = millis();
		if ((millis() - timestamp) > 200) {
			timestamp = millis();
			static bool state = true;
			state = !state;
			Zar.Z22_SKRZYNIA = state;
			switch(nrNagrody) {
				case(1):
   					Zar.Z19_LED1 = state;
					if (!Mp3IsPlaying()) PlayRandomLangMp3(wcPlayNagroda2Language);
				break;
				case(2):
   					Zar.Z20_LED2 = state;
					if (!Mp3IsPlaying()) PlayRandomLangMp3(wcPlayNagroda3Language);
				break;
				case(3):
					Zar.Z21_LED3 = state;
					if (!Mp3IsPlaying()) PlayRandomLangMp3(wcPlayNagroda4Language);
				break;
			}
			UpdateSPI();
		}
    vTaskResume(TaskPcfHandle);
	}
       	
  Zar.Z16_MOTOR1 = 0;
  Zar.Z17_MOTOR2 = 0;
  Zar.Z18_MOTOR3 = 0;
	Zar.Z19_LED1 = 0;
	Zar.Z20_LED2 = 0;
	Zar.Z21_LED3 = 0;
	Zar.Z22_SKRZYNIA = 0;
	UpdateSPI();
}

uint32_t AlgorytmGift(uint32_t wynik)
{
	static uint8_t rnd = 10;
	printf("\n AlgorytmGift wynik input = %d", wynik);

	if (rnd < TOLERANCJA) rnd++;
	else rnd = 1;

  	if (Fram.BoxerModel == GIFT_ESPANIOL) {
    	if (wynik >= (333 - TOLERANCJA) && wynik <= (333 + TOLERANCJA))
    	{
        	if (Fram.GiftWinRatioMale > 0) {
        		printf(" Fram.GiftWinRatioMale = %d ", Fram.GiftWinRatioMale);
        		if (Fram.GiftCntMale <= 0) {
          			Fram.GiftCntMale = Fram.GiftWinRatioMale;
          			wynik = 333;
          			Fram.GiftCntOut555++;
        		}
        		else {
          			Fram.GiftCntMale--;
          			if (wynik == 333) wynik += rnd;
          			printf(" Fram.GiftCntMale = %d", Fram.GiftCntMale);
        		}
        	}
    	}
		if (wynik >= (444 - TOLERANCJA) && wynik <= (444 + TOLERANCJA))	{
	        if (Fram.GiftWinRatioMale > 0) {
        		printf(" Fram.GiftWinRatioMale = %d ", Fram.GiftWinRatioMale);
        		if (Fram.GiftCntMale <= 0) {
          			Fram.GiftCntMale = Fram.GiftWinRatioMale;
          			wynik = 444;
          			Fram.GiftCntOut555++;
        		}
        		else {
          			Fram.GiftCntMale--;
          			if (wynik == 444) wynik += rnd;
          			printf(" Fram.GiftCntMale = %d", Fram.GiftCntMale);
		        }
        	}  
    	}
    	
  	}
	if (Fram.BoxerModel == GIFT_ESPANIOL || Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS_ESPANIOL) {
	   	if (wynik >= (888 - TOLERANCJA) && wynik <= (888 + TOLERANCJA))  {
        	if (Fram.GiftWinRatioSrednie > 0) {
        		if (Fram.GiftCntDuze <= 0) {
          			Fram.GiftCntDuze = Fram.GiftWinRatioDuze;
          			wynik = 888;
          			Fram.GiftCntOut777++;
        		}
        		else {
          			Fram.GiftCntDuze--;
          			if (wynik == 888) wynik += rnd;
          			printf(" Fram.GiftCntDuze = %d", Fram.GiftCntDuze);
        		}   
        	}
    	}
	}


  	if ((Fram.BoxerModel == GIFT) || (Fram.BoxerModel == GIFT_ESPANIOL) || (Fram.BoxerModel == DOUBLE_HIT_GIFT_KIDS_ESPANIOL)) {

		if (wynik >= (555 - TOLERANCJA) && wynik <= (555 + TOLERANCJA))
		{
	  		if (Fram.GiftWinRatioMale > 0) {
				printf(" Fram.GiftWinRatioMale = %d ", Fram.GiftWinRatioMale);
				if (Fram.GiftCntMale <= 0) {
					Fram.GiftCntMale = Fram.GiftWinRatioMale;
					wynik = 555;
					Fram.GiftCntOut555++;
				}
				else {
					Fram.GiftCntMale--;
					if (wynik == 555) wynik += rnd;
					printf(" Fram.GiftCntMale = %d", Fram.GiftCntMale);
				}
	  		}
		}
  	}
	if (wynik >= (666 - TOLERANCJA) && wynik <= (666 + TOLERANCJA))
	{
		if (Fram.GiftWinRatioSrednie > 0) {
			if (Fram.GiftCntSrednie <= 0) {
				Fram.GiftCntSrednie = Fram.GiftWinRatioSrednie;
				wynik = 666;
				Fram.GiftCntOut666++;
			}
			else {
				Fram.GiftCntSrednie--;
				if (wynik == 666) wynik += rnd;
				printf(" Fram.GiftCntSrednie = %d", Fram.GiftCntSrednie);
			}
		}
	}
	if (wynik >= (777 - TOLERANCJA) && wynik <= (777 + TOLERANCJA))
	{
	  if (Fram.GiftWinRatioSrednie > 0) {
		if (Fram.GiftCntDuze <= 0) {
			Fram.GiftCntDuze = Fram.GiftWinRatioDuze;
			wynik = 777;
			Fram.GiftCntOut777++;
		}
		else {
			Fram.GiftCntDuze--;
			if (wynik == 777) wynik += rnd;
			printf(" Fram.GiftCntDuze = %d", Fram.GiftCntDuze);
		}		
	  }
	}

	printf("\n AlgorytmGift output = %d", wynik);

    return wynik;
}
