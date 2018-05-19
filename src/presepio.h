#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"
#include "config.h"
#include "storyboard\storyboard.h"
#include "boards\triac_board.h"
#include "boards\relay_board.h"
#include "os\SDFileSystem.h"

struct PlayBufferEntry
{
  int32_t output;
  TimelineEntry entry;
};

class Presepio
{
public:
  Presepio();

  void init();
  void playTimeline();

private:
  SDFileSystem sd;
  Serial pc;

  Storyboard storyboard;
  RelayBoard relay_board;
  bool lastInput50HzIsStable;
  TriacBoard triac_board;

  //Circular buffer for storyboard playing
  bool play;
  int playBufferCount;
  PlayBufferEntry* playBuffer;
  volatile int playBufferHead;
  volatile int playBufferTail;
  millisec playBufferHeadTime;
  millisec playBufferTailTime;
  millisec playBufferMaxTime;
  void fillPlayBuffer();

  Ticker ticker;
  volatile bool tick_received;
  volatile millisec64 tick_count;
  volatile millisec64 currTime; //Time in milliseconds
  volatile millisec64 storyboardTime; 

  void tick();
  void executePlayBuffer();
  void applyTimelineEntry(uint8_t output, const TimelineEntry* entry);
};

#endif