#ifndef _STORYBOARDPLAYER_H_
#define _STORYBOARDPLAYER_H_

#include "Storyboard.h"
#include "..\boards\triac_board.h"
#include "..\boards\relay_board.h"

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
  StoryboardPlayer(Storyboard *storyboard, RelayBoard *relay_board, TriacBoard *triac_board);

  void init();
  void mainLoop();
  void tick(millisec64 timeDelta);

  void play();
  void pause();
  void stop();

private:
  Storyboard *storyboard;
  RelayBoard *relay_board;
  TriacBoard *triac_board;

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
  void applyTimelineEntry(uint8_t output, const TimelineEntry* entry);
};

#endif