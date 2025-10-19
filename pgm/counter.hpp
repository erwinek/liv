#ifndef COUNTER_H
#define COUNTER_H
//obsluga licznika mech
#include <stdint.h>



class Counter {

public:
  Counter() {
    m_NewCounts = 0;
  }
  void AddCounts(float pulseCnt);
  void Loop(void);

private:
  float m_NewCounts;

};

extern Counter MechCount;

#endif