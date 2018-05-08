#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"
#include "config.h"
#include "timeline.h"
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

  Timeline timeline;
  RelayBoard relay_board;
  TriacBoard triac_board;

  Ticker ticker;
  volatile bool tick_received;
  volatile int tick_count;
  int curr_time; //Tenths of second in current timeline

  void tick();
};

#endif