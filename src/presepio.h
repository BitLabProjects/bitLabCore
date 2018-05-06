#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"
#include "triac_board.h"
#include "relay_board.h"

class Presepio
{
public:
  Presepio();

  void init();
  void loop();
  void dimming();

private:
  Serial pc;

  RelayBoard relay_board;
  TriacBoard triac_board;

  Ticker ticker;
  bool tick_received;
  int tick_count;
  int curr_time; //Tenths of second in current timeline

  void tick();
};

#endif