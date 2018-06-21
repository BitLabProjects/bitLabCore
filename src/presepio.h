#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"
#include "config.h"
#include "storyboard\storyboard.h"
#include "storyboard\storyboard_player.h"
#include "boards\triac_board.h"
#include "boards\relay_board.h"
#include "FATFileSystem.h"
#include "os\SDBlockDevice.h"
#include "wav\wav_player.h"

class Presepio
{
public:
  Presepio();

  void init();
  void mainLoop();

private:
  SDBlockDevice sdbd;
  FATFileSystem fs;
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

  WavPlayer wavPlayer;

  void tick();
};

#endif