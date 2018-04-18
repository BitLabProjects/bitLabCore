#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"

class Presepio {
public:
  Presepio();

  void init();
  void loop();
  void dimming();

private:
  DigitalOut led_heartbeat;

  DigitalOut analog0;
  DigitalOut analog1;
  DigitalOut analog2;
  DigitalOut analog3;
  DigitalOut analog4;
  DigitalOut analog5;
  DigitalOut analog6;
  DigitalOut analog7;
  DigitalOut analogOutMap[8];
  
  InterruptIn main_crossover;
  bool       rise_received;  

  Serial     pc;

  Ticker     ticker;
  Ticker     zerocross_sim;
  bool       tick_received;
  int        tick_count;
  int        rise_count;
  int        curr_time; //Tenths of second in current timeline

  void tick();
  void main_crossover_rise();    
};

#endif