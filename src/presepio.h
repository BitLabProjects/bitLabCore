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


class Presepio: public bitLabCore
{
public:
  Presepio();

private:
  void init();
  void mainLoop();
  void tick(millisec64 timeDelta);

  Storyboard storyboard;
  RelayBoard relay_board;
  TriacBoard triac_board;
  StoryboardPlayer storyboardPlayer;
};

#endif