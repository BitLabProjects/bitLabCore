#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"
#include "config.h"
#include "storyboard\storyboard.h"
#include "storyboard\storyboard_player.h"
#include "boards\triac_board.h"
#include "boards\relay_board.h"
#include "os\bitLabCore.h"
#include "os\SDFileSystem.h"

class Presepio: public bitLabCore
{
public:
  Presepio();

private:
  void init();
  void mainLoop();

  SDFileSystem sd;
  Serial pc;

  Storyboard storyboard;
  RelayBoard relay_board;
  TriacBoard triac_board;
  StoryboardPlayer storyboardPlayer;
  
  bool lastInput50HzIsStable;

  Ticker ticker;
  volatile bool tick_received;
  volatile millisec64 tick_count;
  volatile millisec64 currTime; //Time in milliseconds

  void tick();
};

#endif