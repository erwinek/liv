#define _LANGUAGE_H_


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Kodyfikacja dŸwiêków														  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
100..199 - Prezentacje
200..249 - Stopnie punktów muzyka
250..299 - Stopnie punktów komunikaty
 		   lub
200..299 - Stopnie punktów muzyka z komunikatami
300..307 - Moneta,
308..309 - Plus 1 Kredyt,
310..329 - Powitanie
330..349 - Game Start
350..359 - Game Over
360..369 - Liczenie
370..379 - Rekord, fanfary
390..399 - Rozne key (Man, Woman)

401..450  - stopnie punktow kopacz

950..999 - Setup
		951 - klik miedzy programami
		952 - klik 
		953 - kasowanie
		954 - zapis
		955 - wejœcie setup


100 - komunikat dodawany do innych prezentacji 101..199 
300 - wrzut monety (mieszany np. z dŸwiêkiem wrzuæ monetê)	
330 - np. powodzenia plus standardowe dŸwiêki startu	
Itd.

Je¿eli sa wersje z kilkoma wersja i jezykowymi w jednym ISD to posiadaj¹ dodatkowe komunikaty w obszarze powy¿ej 10000
np: je¿eli wczesniej bral udzial komunikat nr 231 to w innej wersji jezykowej bedzie to 1232 itd 
(oczywiscie wiekszosc komunikatów pozostaje niezaleznie od wersji jezykowej)
ENGLISH 	1000
POLAND		2000
RUSSIAN 	3000
GERMAN 		4000
SPANISH 	5000
ITALIAN 	6000
CZECH  		7000
UKRAINE		8000
FRENCH		9000
GREECE	    10000
HUNGARY	    11000
	        XX000

natomiast jezeli sa dzwieki które pojawiaja sie wiecej niz w jednej wersji jezykowej to sa ponizej 1000
*/

typedef enum Language {
	ENGLISH = 0,
	POLAND = 1,
	RUSSIAN = 2,
	GERMAN = 3,
	SPANISH = 4,
	ITALIAN = 5,
	CZECH = 6,
	UKRAINE = 7,
	FRENCH = 8,
	GREECE = 9,
	HUNGARY = 10,
	MAX_LANGUAGE = 11
} Language_t;


static const uint16_t wcPlayPrezentacja0[][MAX_LANGUAGE]=
{
//		English	Poland	Russian	German	Spanish	Italian Czech		
			100,					100,
};
/*
static const uint16_t wcPlayPrezentacja[]	={101,103,104};

static const uint16_t wcPlayStopien01[]		={200};
static const uint16_t wcPlayStopien02[]		={230};
static const uint16_t wcPlayStopien03[]		={260};
static const uint16_t wcPlayStopien04[]		={280};

static const uint16_t wcPlayStopien1[]		={201,202,212,216,217,219,223};
static const uint16_t wcPlayStopien2[]		={232,233,236,237,238,239,242};
static const uint16_t wcPlayStopien3[]		={261,262,263,264,265,271,272};
static const uint16_t wcPlayStopien4[]		={281,282,283,284,285,289,291,293};

static const uint16_t wcPlayStopien1Kopacz[]={401,402};
static const uint16_t wcPlayStopien2Kopacz[]={410,411};
static const uint16_t wcPlayStopien3Kopacz[]={421,422,423};
static const uint16_t wcPlayStopien4Kopacz[]={431,432};
*/

static const uint16_t wcPlayNagroda1Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1401,  2401, 1401, 1401, 5401, 0,  0,  0,  0,  0,  0
};	
static const uint16_t wcPlayNagroda2Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1402,  2402, 1402, 1402, 5402, 0,  0,  0,  0,  0,  0
};	
static const uint16_t wcPlayNagroda3Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1403,  2403, 1403, 1403, 5403, 0,  0,  0,  0,  0,  0
};	
static const uint16_t wcPlayNagroda4Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1404,  2404, 1404, 1404, 5404, 0,  0,  0,  0,  0,  0,
};	

static const uint16_t wcPlayPrawieNagrodaLanguage[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
      1501, 2501, 1501, 1501, 5501, 0,  0,  0,  0,  0,  0,
      1502, 2502, 1502, 1502, 5502, 0,  0,  0,  0,  0,  0,
      1503, 2503, 1503, 1503, 5503, 0,  0,  0,  0,  0,  0

			
};

static const uint16_t wcPlayJeszczeRazLanguage[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
      1510, 2510, 1510, 1510, 5510, 0,  0,  0,  0,  0,  0,
      1511, 2511, 1511, 1511, 5511, 0,  0,  0,  0,  0,  0
};

static const uint16_t wcPlayStopien1Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1201,	2201,	3201,	4201,	5201,	6201,	7201,	8201,	9201,	10201,	0,
};	

static const uint16_t wcPlayStopien2Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1232,	2232,	3232,	4232,	5232,	6232,	7232,	8232,	9232,	10232,	0,
			1233,	2233,	3233,	4233,	5233,	6233,	7233,	8233,	9233,	10233,	0,
};	

static const uint16_t wcPlayStopien3Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1261,	2261,	3261,	4261,	5261,	6261,	7261,	8261,	9261,	10261,	0,
			1263,	2263,	3263,	4263,	5263,	6263,	7261,	8263,	9263,	10263,	0,
};	

static const uint16_t wcPlayStopien4Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1282,	2282,	3282,	4282,	5282,	6282,	7282,	8282,	9282,	10282,	0,
			1285,	2285,	3285,	4285,	5285,	6285,	7281,	8285,	9285,	10285,	0,
};	

static const uint16_t wcPlayMoneta[]		={300};
static const uint16_t wcPlayFanfaryRekord[]		={380,381,382};

static const uint16_t wcPlayWrzucMonete[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1302,	2301,	3302,	4302,	5302,	6302,	7302,	8302,	9302,	10302,	0,
			1302,	2302,	3302,	4302,	5302,	6302,	7302,	8302,	9302,	10302,	0,
};	

static const uint16_t wcPlayPlus1Kredyt[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1308,	2308,	3308,	4308,	5308,	6308,	7308,	8308,	9308,	10308,	0,
};	

static const uint16_t wcPlayPowitanie[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			99,		2311,	3311,	4311,	5311,	6311,	1311,	8311,	9311,	10311,	1311,
};	

static const uint16_t wcPlayGameStart0[]		={330};
static const uint16_t wcPlayGameStart0Kopacz[]		={331};

static const uint16_t wcPlayGameStart[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1331,	2331,	3331,	4331,	5331,	6331,	0,	8331,	9331,	10331,	0,
			1331,	2332,	3331,	4331,	5331,	6331,	0,	8331,	9331,	10331,	0,
};

static const uint16_t wcPlayGameStartMuzyka0[]	={0, 333, 335}; //wcPlayGameStartMuzyka0[bitTrybGry] 
static const uint16_t wcPlayGameStartMuzyka1[]	={0, 334, 336}; //BOXER = 1 KOPACZ = 2

static const uint16_t wcPlayGameOver0[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	
};

static const uint16_t wcGameOver0Language[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	1350,	
};

static const uint16_t wcPlayGameOver[]			={351,352,353};

static const uint16_t wcPlayLiczenie[]			={361,362};

static const uint16_t wcPlayRekord[]			={370};
static const uint16_t wcPlayFanfary[]			={0,371,372};

static const uint16_t wcPlayClickMan[]			={391};
static const uint16_t wcPlayClickWoman[]		={392};

static const uint16_t wcPlaySetupClickProgram[]	={0};
static const uint16_t wcPlaySetupClick[]		={0};
static const uint16_t wcPlaySetupClear[]		={953};
static const uint16_t wcPlaySetupSave[]			={954};
static const uint16_t wcPlaySetupWejscieProgram[]	={955};

static const uint16_t Mp3SelectNumberOfPlayers[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1100,	1100,	1100,	1100,	1100,	1100,	1100,	1100,	1100,	1100,	1100	
};


static const uint16_t Mp3OnePlayer[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1101,	1101,	1101,	1101,	1101,	1101,	1101,	1101,	1101,	1101,	1101	
};

static const uint16_t Mp3TwoPlayers[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1102,	1102,	1102,	1102,	1102,	1102,	1102,	1102,	1102,	1102,	1102	
};

static const uint16_t Mp3ThreePlayers[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1103,	1103,	1103,	1103,	1103,	1103,	1103,	1103,	1103,	1103,	1103
};

static const uint16_t Mp3PlayerOne[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1105,	1105,	1105,	1105,	1105,	1105,	1105,	1105,	1105,	1105,	1105	
};

static const uint16_t Mp3PlayerTwo[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1106,	1106,	1106,	1106,	1106,	1106,	1106,	1106,	1106,	1106,	1106	
};

static const uint16_t Mp3PlayerThree[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1107,	1107,	1107,	1107,	1107,	1107,	1107,	1107,	1107,	1107,	1107	
};

static const uint16_t Mp3TheWinnerIs[][MAX_LANGUAGE]=
{
/* 			English	Poland	Russian	German	Spanish	Italian Czech	Ukraine	French	Grecce	Hungary	*/
			1110,	1110,	1110,	1110,	1110,	1110,	1110,	1110,	1110,	1110,	1110	
};
