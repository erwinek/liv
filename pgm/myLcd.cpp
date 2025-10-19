#include <LiquidCrystal_I2C.h>
#include "myLcd.hpp"

LiquidCrystal_I2C lcd(0x24,16,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
MyLcd myLcd;

void MyLcd::init(void)
{
    lcd.init();                      // initialize the lcd 
}

void MyLcd::setCursor(uint8_t x, uint8_t y)
{
    m_x = x;
    m_y = y;
}

void MyLcd::print(const char* text)
{
    int n = 0;
    if(m_y==0) n = snprintf(&currentTxt1[m_x], LINE_LEN - m_x , text);
    else if(m_y==1) n = snprintf(&currentTxt2[m_x], LINE_LEN - m_x , text);
    m_x += n;
}

void MyLcd::print(const uint32_t digit)
{
    int n = 0;
    if(m_y==0) n = snprintf(&currentTxt1[m_x], LINE_LEN - m_x , "%d", digit);
    else if(m_y==1) n = snprintf(&currentTxt2[m_x], LINE_LEN - m_x , "%d", digit);
    m_x += n;
}

void MyLcd::clear(void)
{
    for (int x = 0; x < 16; x++)  {
        currentTxt1[x] = ' ';
    }
    for (int x = 0; x < 16; x++)  {
        currentTxt2[x] = ' ';
    }
}

void MyLcd::handle(void) {
    static char Line1[16];
    static char Line2[16];

    if(strncmp(currentTxt1, Line1, 16)) {
        strncpy(Line1, currentTxt1, 16);
        lcd.setCursor(0, 0);
        lcd.print(Line1);
        printf("\n Line1=%s", Line1);
    }
    if(strncmp(currentTxt2, Line2, 16)) {
        strncpy(Line2, currentTxt2, 16);
        lcd.setCursor(0, 1);
        lcd.print(Line2);
        printf("\n Line2=%s", Line2);
    }
}


