#include "counter.hpp"
#include "I2C_eeprom.h"
#include "FRAM.h"
#include "pcf.h"

void Counter::AddCounts(float pulseCnt) {
  printf("\n Counter::AddCounts m_NewCounts=%f", m_NewCounts);
  m_NewCounts += pulseCnt;
  printf("\n m_NewCounts=%f", m_NewCounts);
}

Counter MechCount;

void Counter::Loop(void) {
  static bool halfPuls = false;
  static uint64_t timeout = millis();

  if(millis() - timeout > 200) {
    timeout = millis();
    if(halfPuls==true) {
      halfPuls = false;
      SET_COUNTER;
    }
    else if (m_NewCounts >= 1) {
      Fram.QuarterCoinCnt = m_NewCounts;
      m_NewCounts--;
      halfPuls = true;
      CLR_COUNTER;
    }
  }
}
