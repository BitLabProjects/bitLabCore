#ifndef _STORYBOARDPLAYER_H_
#define _STORYBOARDPLAYER_H_

#include "Storyboard.h"

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

class StoryboardPlayer {
public:
  StoryboardPlayer(Storyboard* storyboard, Callback<void(int, int, millisec, millisec)> onSetOutput);

  void fillPlayBuffer();
  void advance(millisec timeDelta);

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

  void executePlayBuffer();
};

#endif