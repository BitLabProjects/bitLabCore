#include "presepio.h"

// for t=0 and t=TIMELINE_DURATION percent values must be equal
int analog_timeline[ANALOGOUT_COUNT][TIME_PERCENT_ITEMS] = {{0, 0, 5, 10, 10, 0, 15, 20, 20, 0, 25, 30, 30, 0, 35, 40, 40, 0, -1, 0},
                                                            {0, 40, 5, 0, 10, 30, 15, 0, 20, 20, 25, 0, 30, 10, 35, 0, 40, 40, -1, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0}};

Presepio::Presepio() : pc(USBTX, USBRX),
                       timeline(),
                       relay_board(),
                       triac_board()
{
}

void Presepio::init()
{
  curr_time = 0;
  tick_received = false;
  tick_count = 0;
  ticker.attach(callback(this, &Presepio::tick), 1.0 / TICKS_PER_SECOND);

  pc.baud(115200);
  pc.printf("===== Presepe =====\n");
  pc.printf("   version: 0.1a   \n");
  pc.printf("===================\n");

  timeline.create(TIMELINE_ENTRIES);
  timeline.add(0, 1, 100, 2500);
  timeline.add(0, 9, 100, 0);
  timeline.add(5000, 1, 0, 2500);
  timeline.add(5000, 9, 0, 0);
  timeline.add(10000, TimelineEntry::OutputForTimelineEnd, 0, 0);
}

void Presepio::playTimeline()
{
  bool printEvents = true;
  bool printDebug = false;

  if (tick_received)
  {
    tick_received = false;

    //Calculate the current time in milliseconds
    int currTime = 1000 * tick_count / TICKS_PER_SECOND;

    const TimelineEntry *currEntry = timeline.getCurrent();
    //TODO loop for multiple simultaneous (or near) entries
    if (currEntry->time <= currTime)
    {
      //Apply
      if (currEntry->isTimelineEnd())
      {
        if (printEvents)
        {
          pc.printf("%5i ms: timeline finished\r\n", currEntry->time);
        }
        timeline.moveFirst();
        tick_count = 0; //Reset time when timeline ended
      }
      else
      {
        uint8_t output = currEntry->output;
        if (output >= 1 && output <= 8)
        {
          int triacIdx = output - 1;
          triac_board.setOutput(triacIdx, currEntry->value, currEntry->time, currEntry->duration);
          if (printEvents)
          {
            pc.printf("%5i ms: dimmer #%i to %i%% in %i ms\r\n", currEntry->time, triacIdx + 1, currEntry->value, currEntry->duration);
          }
        }
        else if (output >= 9 && output <= 40)
        {
          int relayIdx = output - 9;
          relay_board.setOutput(relayIdx, currEntry->value);
          if (printEvents)
          {
            pc.printf("%5i ms: relay  #%i to %s\r\n", currEntry->time, relayIdx + 1, currEntry->value == 0 ? "OFF" : "ON");
          }
        }
        timeline.moveNext();
      }
    }

    triac_board.updateOutputs(currTime);
    relay_board.updateOutputs();

    if (printDebug)
    {
      //Every 100 ms print outputs for debug
      if (currTime % 100 == 0)
      {
        triac_board.debugPrintOutputs(pc);
      }
    }
  }
}

void Presepio::tick()
{
  tick_received = true;
  tick_count += 1;
}