#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"
#include "config.h"
#include "storyboard.h"
#include "triac_board.h"
#include "relay_board.h"

class Presepio
{
public:
  Presepio();

  void init();
  void playTimeline();

private:
  Serial pc;

  Storyboard storyboard;
  RelayBoard relay_board;
  bool lastInput50HzIsStable;
  TriacBoard triac_board;

  Ticker ticker;
  volatile bool tick_received;
  volatile int tick_count;
  int curr_time; //Tenths of second in current timeline

  void tick();
};

#endif