#ifndef _STORYBOARDPLAYER_H_
#define _STORYBOARDPLAYER_H_

#include "Storyboard.h"
//#include "..\bitLabCore\src\os\CoreModule.h"

struct PlayBufferEntry
{
  int32_t output;
  TimelineEntry entry;
};

enum PlayStatus
{
  Stopped,
  Paused,
  Playing
};

class StoryboardPlayer /*: public CoreModule*/ {
public:
  StoryboardPlayer(Storyboard* storyboard, Callback<void(int, int, millisec, millisec)> onSetOutput);

  // --- CoreModule ---
  void init();
  void mainLoop();
  void tick(millisec64 timeDelta);
  // ------------------

  void play();
  void pause();
  void stop();

  millisec64 getStoryboardTime() { return storyboardTime; }

private:
  Storyboard* storyboard;
  Callback<void(int, int, millisec, millisec)> onSetOutput;

  //Circular buffer for storyboard playing
  PlayStatus playStatus;
  int playBufferCount;
  PlayBufferEntry* playBuffer;
  volatile int playBufferHead;
  volatile int playBufferTail;
  millisec playBufferHeadTime;
  millisec playBufferTailTime;
  millisec playBufferMaxTime;
  volatile millisec64 storyboardTime;

  void fillPlayBuffer();
  void executePlayBuffer();
};

#endif