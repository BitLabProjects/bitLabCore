#include "presepio.h"
#include "os\os.h"

#include "platform/mbed_stats.h"

#include "storyboard\storyboard_loader.h"

#define USE_JSON true
#define USE_ORIGINAL_TIMELINE false

// for t=0 and t=TIMELINE_DURATION percent values must be equal
int analog_timeline[ANALOGOUT_COUNT][TIME_PERCENT_ITEMS] = {{0, 0, 5, 10, 10, 0, 15, 20, 20, 0, 25, 30, 30, 0, 35, 40, 40, 0, -1, 0},
                                                            {0, 40, 5, 0, 10, 30, 15, 0, 20, 20, 25, 0, 30, 10, 35, 0, 40, 40, -1, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 50, 0}};

Presepio::Presepio() : sd(PC_12, PC_11, PC_10, PD_2, "sd"),
                       pc(USBTX, USBRX),
                       storyboard(),
                       relay_board(),
                       triac_board()
{
}

void Presepio::init()
{
  pc.baud(115200);

  curr_time = 0;
  tick_received = false;
  tick_count = 0;
  ticker.attach(callback(this, &Presepio::tick), 1.0 / TICKS_PER_SECOND);

  pc.printf("===== Presepe =====\n");
  pc.printf("   version: 0.1a   \n");
  pc.printf("===================\n");
  pc.printf("Used heap: %i bytes\n", Os::getUsedHeap());

  pc.printf("Mounting SD........");
  int sdErrCode = sd.disk_initialize();
  if (sdErrCode == 0)
    pc.printf("[OK]\n");
  else
    pc.printf("[ERR]\n");


  if (USE_JSON)
  {
    const char *jsonContent = "{\"timelinesCount\":2,\"timelines\":[{\"name\":\"timeline0\",\"outputId\":1,\"outputType\":0,\"entriesCount\":3,\"entries\":[{\"time\":0,\"value\":100,\"duration\":500},{\"time\":500,\"value\":20,\"duration\":250},{\"time\":750,\"value\":50,\"duration\":100}]},{\"name\":\"timeline1\",\"outputId\":2,\"outputType\":1,\"entriesCount\":3,\"entries\":[{\"time\":0,\"value\":1,\"duration\":125},{\"time\":125,\"value\":0,\"duration\":1000},{\"time\":1125,\"value\":1,\"duration\":250}]}]}";
    StoryboardLoader loader(&storyboard, jsonContent);
    loader.load();
  }
  else if (USE_ORIGINAL_TIMELINE)
  {
    storyboard.create(40, 4000);

    //           time,#out,value,duration
    // analog output_1
    storyboard.addTimeline(3);
    storyboard.addEntry(0, 1, 0, 10000);
    storyboard.addEntry(10000, 1, 100, 44000);
    storyboard.addEntry(54000, 1, 0, 65000);
    // analog output_2
    storyboard.addTimeline(4);
    storyboard.addEntry(0, 2, 0, 11000);
    storyboard.addEntry(11000, 2, 80, 58000);
    storyboard.addEntry(69000, 2, 80, 110000);
    storyboard.addEntry(179000, 2, 0, 45000);
    // analog output_3
    storyboard.addTimeline(4);
    storyboard.addEntry(0, 3, 0, 40000);
    storyboard.addEntry(40000, 3, 60, 50000);
    storyboard.addEntry(90000, 3, 60, 80000);
    storyboard.addEntry(170000, 3, 0, 50000);
    // analog output_4
    storyboard.addTimeline(3);
    storyboard.addEntry(0, 4, 0, 160000);
    storyboard.addEntry(160000, 4, 60, 20000);
    storyboard.addEntry(180000, 4, 0, 50000);
    // analog output_5
    storyboard.addTimeline(4);
    storyboard.addEntry(0, 5, 0, 280000);
    storyboard.addEntry(280000, 5, 40, 20000);
    storyboard.addEntry(300000, 5, 40, 30000);
    storyboard.addEntry(330000, 5, 0, 20000);
    // analog output_6
    storyboard.addTimeline(4);
    storyboard.addEntry(0, 6, 0, 185000);
    storyboard.addEntry(185000, 6, 25, 50000);
    storyboard.addEntry(235000, 6, 25, 50000);
    storyboard.addEntry(285000, 6, 0, 15000);
    // analog output_7
    storyboard.addTimeline(4);
    storyboard.addEntry(0, 7, 0, 245000);
    storyboard.addEntry(245000, 7, 40, 15000);
    storyboard.addEntry(260000, 7, 40, 20000);
    storyboard.addEntry(280000, 7, 0, 12000);
    // analog output_8
    storyboard.addTimeline(4);
    storyboard.addEntry(0, 8, 0, 250000);
    storyboard.addEntry(250000, 8, 45, 50000);
    storyboard.addEntry(300000, 8, 45, 40000);
    storyboard.addEntry(340000, 8, 0, 25000);

    // digital output_1
    storyboard.addTimeline(0);
    // digital output_2
    storyboard.addTimeline(4);
    storyboard.addEntry(3000, 10, 100, 0);
    storyboard.addEntry(100000, 10, 0, 0);
    storyboard.addEntry(180000, 10, 100, 0);
    storyboard.addEntry(240000, 10, 0, 0);
    // digital output_3
    storyboard.addTimeline(0);
    // digital output_4
    storyboard.addTimeline(4);
    storyboard.addEntry(28000, 12, 100, 0);
    storyboard.addEntry(90000, 12, 0, 0);
    storyboard.addEntry(180000, 12, 100, 0);
    storyboard.addEntry(220000, 12, 0, 0);
    // digital output_5
    storyboard.addTimeline(4);
    storyboard.addEntry(25000, 13, 100, 0);
    storyboard.addEntry(65000, 13, 0, 0);
    storyboard.addEntry(175000, 13, 100, 0);
    storyboard.addEntry(230000, 13, 0, 0);
    // digital output_6
    storyboard.addTimeline(2);
    storyboard.addEntry(60000, 14, 100, 0);
    storyboard.addEntry(225000, 14, 0, 0);
    // digital output_7
    storyboard.addTimeline(4);
    storyboard.addEntry(25000, 15, 100, 0);
    storyboard.addEntry(80000, 15, 0, 0);
    storyboard.addEntry(180000, 15, 100, 0);
    storyboard.addEntry(210000, 15, 0, 0);
    // digital output_8
    storyboard.addTimeline(2);
    storyboard.addEntry(55000, 16, 100, 0);
    storyboard.addEntry(220000, 16, 0, 0);
    // digital output_9
    storyboard.addTimeline(2);
    storyboard.addEntry(65000, 17, 100, 0);
    storyboard.addEntry(185000, 17, 0, 0);
    // digital output_10
    storyboard.addTimeline(4);
    storyboard.addEntry(10000, 18, 100, 0);
    storyboard.addEntry(95000, 18, 0, 0);
    storyboard.addEntry(155000, 18, 100, 0);
    storyboard.addEntry(255000, 18, 0, 0);
    // digital output_11
    storyboard.addTimeline(2);
    storyboard.addEntry(200000, 19, 100, 0);
    storyboard.addEntry(240000, 19, 0, 0);
    // digital output_12
    storyboard.addTimeline(4);
    storyboard.addEntry(15000, 20, 100, 0);
    storyboard.addEntry(60000, 20, 0, 0);
    storyboard.addEntry(180000, 20, 100, 0);
    storyboard.addEntry(290000, 20, 0, 0);
    // digital output_13
    storyboard.addTimeline(2);
    storyboard.addEntry(65000, 21, 100, 0);
    storyboard.addEntry(225000, 21, 0, 0);
    // digital output_14
    storyboard.addTimeline(2);
    storyboard.addEntry(60000, 22, 100, 0);
    storyboard.addEntry(315000, 22, 0, 0);
    // digital output_15
    storyboard.addTimeline(2);
    storyboard.addEntry(30000, 23, 100, 0);
    storyboard.addEntry(210000, 23, 0, 0);
    // digital output_16
    storyboard.addTimeline(2);
    storyboard.addEntry(25000, 24, 100, 0);
    storyboard.addEntry(210000, 24, 0, 0);
    // digital output_17
    storyboard.addTimeline(0);
    // digital output_18
    storyboard.addTimeline(2);
    storyboard.addEntry(205000, 26, 100, 0);
    storyboard.addEntry(305000, 26, 0, 0);
    // digital output_19
    storyboard.addTimeline(2);
    storyboard.addEntry(200000, 27, 100, 0);
    storyboard.addEntry(340000, 27, 0, 0);
    // digital output_20
    storyboard.addTimeline(2);
    storyboard.addEntry(190000, 28, 100, 0);
    storyboard.addEntry(320000, 28, 0, 0);
    // digital output_21
    storyboard.addTimeline(2);
    storyboard.addEntry(195000, 29, 100, 0);
    storyboard.addEntry(295000, 29, 0, 0);
    // digital output_22
    storyboard.addTimeline(0);
    // digital output_23
    storyboard.addTimeline(2);
    storyboard.addEntry(50000, 31, 100, 0);
    storyboard.addEntry(295000, 31, 0, 0);
    // digital output_24
    storyboard.addTimeline(0);
    // digital output_25
    storyboard.addTimeline(0);
    // digital output_26
    storyboard.addTimeline(4);
    storyboard.addEntry(10000, 34, 100, 0);
    storyboard.addEntry(60000, 34, 0, 0);
    storyboard.addEntry(150000, 34, 100, 0);
    storyboard.addEntry(200000, 34, 0, 0);
    // digital output_27
    storyboard.addTimeline(8);
    storyboard.addEntry(18000, 35, 100, 0);
    storyboard.addEntry(19000, 35, 0, 0);
    storyboard.addEntry(20000, 35, 100, 0);
    storyboard.addEntry(21000, 35, 0, 0);
    storyboard.addEntry(158000, 35, 100, 0);
    storyboard.addEntry(160000, 35, 0, 0);
    storyboard.addEntry(161000, 35, 100, 0);
    storyboard.addEntry(162000, 35, 0, 0);
    // digital output_28
    storyboard.addTimeline(0);
    // digital output_29
    storyboard.addTimeline(2);
    storyboard.addEntry(250000, 37, 100, 0);
    storyboard.addEntry(265000, 37, 0, 0);
    // digital output_30
    storyboard.addTimeline(2);
    storyboard.addEntry(10000, 38, 100, 0);
    storyboard.addEntry(13000, 38, 0, 0);
    // digital output_31
    storyboard.addTimeline(2);
    storyboard.addEntry(14000, 39, 100, 0);
    storyboard.addEntry(18000, 39, 0, 0);
    // digital output_32
    storyboard.addTimeline(2);
    storyboard.addEntry(287000, 40, 100, 0);
    storyboard.addEntry(302000, 40, 0, 0);
  }
  else
  {
    storyboard.create(40, 6000);
    // storyboard.addTimeline(8);
    // storyboard.addEntry(0, 1, 20, 500);
    // storyboard.addEntry(500, 1, 20, 500);
    // storyboard.addEntry(1000, 1, 40, 500);
    // storyboard.addEntry(1500, 1, 40, 500);
    // storyboard.addEntry(2000, 1, 40, 500);
    // storyboard.addEntry(2500, 1, 20, 500);
    // storyboard.addEntry(3000, 1, 20, 500);
    // storyboard.addEntry(3500, 1, 0, 500);
    storyboard.addTimeline(11);
    storyboard.addEntry(0, 1, 0, 1000);
    storyboard.addEntry(1000, 1, 20, 0);
    storyboard.addEntry(1000, 1, 0, 1000);
    storyboard.addEntry(2000, 1, 40, 0);
    storyboard.addEntry(2000, 1, 0, 1000);
    storyboard.addEntry(3000, 1, 60, 0);
    storyboard.addEntry(3000, 1, 0, 1000);
    storyboard.addEntry(4000, 1, 80, 0);
    storyboard.addEntry(4000, 1, 0, 1000);
    storyboard.addEntry(5000, 1, 100, 0);
    storyboard.addEntry(5000, 1, 0, 1000);
    storyboard.addTimeline(0);
    storyboard.addTimeline(0);
    storyboard.addTimeline(0);
    storyboard.addTimeline(0);
    storyboard.addTimeline(0);
    storyboard.addTimeline(0);
    storyboard.addTimeline(0);

    for (uint8_t i = 9; i <= 40; i++)
    {
      storyboard.addTimeline(2);
      storyboard.addEntry(250 * (i - 9), i, 100, 0);
      storyboard.addEntry(250 * (i - 8), i, 0, 0);
    }
  }
}

void Presepio::playTimeline()
{
  bool printEvents = false;
  bool printDebug = false;

  if (tick_received)
  {
    tick_received = false;

    //Calculate the current time in milliseconds
    int currTime = 1000 * tick_count / TICKS_PER_SECOND;

    if (storyboard.isFinished(currTime))
    {
      if (printEvents)
      {
        pc.printf("%5i ms: storyboard finished\r\n", currTime);
      }
      storyboard.reset();
      tick_count = 0; //Reset time when storyboard ended
    }
    else
    {
      for (uint8_t out = 1; out <= storyboard.timelinesCount; out++)
      {
        if (storyboard.isTimelineFinished(out))
        {
          // this timeline has already been applied, go on
          continue;
        }

        const TimelineEntry *currEntry = storyboard.getCurrent(out);
        if (currEntry->time <= currTime)
        {
          //Apply
          if (out >= 1 && out <= 8)
          {
            int triacIdx = out - 1;
            triac_board.setOutput(triacIdx, currEntry->value, currEntry->time, currEntry->duration);
            if (printEvents)
            {
              pc.printf("%5i ms: dimmer #%i to %i%% in %i ms\r\n", currEntry->time, triacIdx + 1, currEntry->value, currEntry->duration);
            }
          }
          else if (out >= 9 && out <= 40)
          {
            int relayIdx = out - 9;
            relay_board.setOutput(relayIdx, currEntry->value);
            if (printEvents)
            {
              pc.printf("%5i ms: relay  #%i to %s\r\n", currEntry->time, relayIdx + 1, currEntry->value == 0 ? "OFF" : "ON");
            }
          }
          // advance timeline cursor to next entry
          storyboard.advanceTimeline(out);
        }
      }
    }

    triac_board.onTick(currTime);
    relay_board.onTick();

    if (printDebug)
    {
      //Every 100 ms print outputs for debug
      if (currTime % 100 == 0)
      {
        triac_board.debugPrintOutputs(pc);
      }
    }

    if (lastInput50HzIsStable != triac_board.getInput50HzIsStable())
    {
      lastInput50HzIsStable = triac_board.getInput50HzIsStable();
      pc.printf("sys: Input 50Hz %s, measured: %.1f Hz\r\n",
                lastInput50HzIsStable ? "detected" : "lost",
                triac_board.getMeasured50HzFrequency());
    }
  }
}

void Presepio::tick()
{
  tick_received = true;
  tick_count += 1;
}