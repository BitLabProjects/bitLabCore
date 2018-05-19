#include "presepio.h"
#include "os\os.h"

#include "platform/mbed_stats.h"

#include "storyboard\storyboard_loader.h"

#define USE_JSON false
#define USE_ORIGINAL_TIMELINE false

Presepio::Presepio() : sd(PC_12, PC_11, PC_10, PD_2, "sd"),
                       pc(USBTX, USBRX),
                       storyboard(),
                       relay_board(),
                       triac_board(),
                       storyboardPlayer(&storyboard, &relay_board, &triac_board)
{
}

void Presepio::init()
{
  pc.baud(115200);

  currTime = 0;
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

    Timeline* t = NULL;
    //           time,#out,value,duration
    // analog output_1
    t = storyboard.addTimeline(1, 3);
    t->add(0, 0, 10000);
    t->add(10000, 100, 44000);
    t->add(54000, 0, 65000);
    // analog output_2
    t = storyboard.addTimeline(2, 4);
    t->add(0, 0, 11000);
    t->add(11000, 80, 58000);
    t->add(69000, 80, 110000);
    t->add(179000, 0, 45000);
    // analog output_3
    t = storyboard.addTimeline(3, 4);
    t->add(0, 0, 40000);
    t->add(40000, 60, 50000);
    t->add(90000, 60, 80000);
    t->add(170000, 0, 50000);
    // analog output_4
    t = storyboard.addTimeline(4, 3);
    t->add(0, 0, 160000);
    t->add(160000, 60, 20000);
    t->add(180000, 0, 50000);
    // analog output_5
    t = storyboard.addTimeline(5, 4);
    t->add(0, 0, 280000);
    t->add(280000, 40, 20000);
    t->add(300000, 40, 30000);
    t->add(330000, 0, 20000);
    // analog output_6
    t = storyboard.addTimeline(6, 4);
    t->add(0, 0, 185000);
    t->add(185000, 25, 50000);
    t->add(235000, 25, 50000);
    t->add(285000, 0, 15000);
    // analog output_7
    t = storyboard.addTimeline(7, 4);
    t->add(0, 0, 245000);
    t->add(245000, 40, 15000);
    t->add(260000, 40, 20000);
    t->add(280000, 0, 12000);
    // analog output_8
    t = storyboard.addTimeline(8, 4);
    t->add(0, 0, 250000);
    t->add(250000, 45, 50000);
    t->add(300000, 45, 40000);
    t->add(340000, 0, 25000);

    // digital output_1
    t = storyboard.addTimeline(9, 0);
    // digital output_2
    t = storyboard.addTimeline(10, 4);
    t->add(3000, 100, 0);
    t->add(100000, 0, 0);
    t->add(180000, 100, 0);
    t->add(240000, 0, 0);
    // digital output_3
    t = storyboard.addTimeline(11, 0);
    // digital output_4
    t = storyboard.addTimeline(12, 4);
    t->add(28000, 100, 0);
    t->add(90000, 0, 0);
    t->add(180000, 100, 0);
    t->add(220000, 0, 0);
    // digital output_5
    t = storyboard.addTimeline(13, 4);
    t->add(25000, 100, 0);
    t->add(65000, 0, 0);
    t->add(175000, 100, 0);
    t->add(230000, 0, 0);
    // digital output_6
    t = storyboard.addTimeline(14, 2);
    t->add(60000, 100, 0);
    t->add(225000, 0, 0);
    // digital output_7
    t = storyboard.addTimeline(15, 4);
    t->add(25000, 100, 0);
    t->add(80000, 0, 0);
    t->add(180000, 100, 0);
    t->add(210000, 0, 0);
    // digital output_8
    t = storyboard.addTimeline(16, 2);
    t->add(55000, 100, 0);
    t->add(220000, 0, 0);
    // digital output_9
    t = storyboard.addTimeline(17, 2);
    t->add(65000, 100, 0);
    t->add(185000, 0, 0);
    // digital output_10
    t = storyboard.addTimeline(18, 4);
    t->add(10000, 100, 0);
    t->add(95000, 0, 0);
    t->add(155000, 100, 0);
    t->add(255000, 0, 0);
    // digital output_11
    t = storyboard.addTimeline(19, 2);
    t->add(200000, 100, 0);
    t->add(240000, 0, 0);
    // digital output_12
    t = storyboard.addTimeline(20, 4);
    t->add(15000, 100, 0);
    t->add(60000, 0, 0);
    t->add(180000, 100, 0);
    t->add(290000, 0, 0);
    // digital output_13
    t = storyboard.addTimeline(21, 2);
    t->add(65000, 100, 0);
    t->add(225000, 0, 0);
    // digital output_14
    t = storyboard.addTimeline(22, 2);
    t->add(60000, 100, 0);
    t->add(315000, 0, 0);
    // digital output_15
    t = storyboard.addTimeline(23, 2);
    t->add(30000, 100, 0);
    t->add(210000, 0, 0);
    // digital output_16
    t = storyboard.addTimeline(24, 2);
    t->add(25000, 100, 0);
    t->add(210000, 0, 0);
    // digital output_17
    t = storyboard.addTimeline(25, 0);
    // digital output_18
    t = storyboard.addTimeline(26, 2);
    t->add(205000, 100, 0);
    t->add(305000, 0, 0);
    // digital output_19
    t = storyboard.addTimeline(27, 2);
    t->add(200000, 100, 0);
    t->add(340000, 0, 0);
    // digital output_20
    t = storyboard.addTimeline(28, 2);
    t->add(190000, 100, 0);
    t->add(320000, 0, 0);
    // digital output_21
    t = storyboard.addTimeline(29, 2);
    t->add(195000, 100, 0);
    t->add(295000, 0, 0);
    // digital output_22
    t = storyboard.addTimeline(30, 0);
    // digital output_23
    t = storyboard.addTimeline(31, 2);
    t->add(50000, 100, 0);
    t->add(295000, 0, 0);
    // digital output_24
    t = storyboard.addTimeline(32, 0);
    // digital output_25
    t = storyboard.addTimeline(33, 0);
    // digital output_26
    t = storyboard.addTimeline(34, 4);
    t->add(10000, 100, 0);
    t->add(60000, 0, 0);
    t->add(150000, 100, 0);
    t->add(200000, 0, 0);
    // digital output_27
    t = storyboard.addTimeline(35, 8);
    t->add(18000, 100, 0);
    t->add(19000, 0, 0);
    t->add(20000, 100, 0);
    t->add(21000, 0, 0);
    t->add(158000, 100, 0);
    t->add(160000, 0, 0);
    t->add(161000, 100, 0);
    t->add(162000, 0, 0);
    // digital output_28
    t = storyboard.addTimeline(36, 0);
    // digital output_29
    t = storyboard.addTimeline(37, 2);
    t->add(250000, 100, 0);
    t->add(265000, 0, 0);
    // digital output_30
    t = storyboard.addTimeline(38, 2);
    t->add(10000, 100, 0);
    t->add(13000, 0, 0);
    // digital output_31
    t = storyboard.addTimeline(39, 2);
    t->add(14000, 100, 0);
    t->add(18000, 0, 0);
    // digital output_32
    t = storyboard.addTimeline(40, 2);
    t->add(287000, 100, 0);
    t->add(302000, 0, 0);
  }
  else
  {
    Timeline* t = NULL;

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
    t = storyboard.addTimeline(1, 11);
    t->add(0, 0, 1000);
    t->add(1000, 20, 0);
    t->add(1000, 0, 1000);
    t->add(2000, 40, 0);
    t->add(2000, 0, 1000);
    t->add(3000, 60, 0);
    t->add(3000, 0, 1000);
    t->add(4000, 80, 0);
    t->add(4000, 0, 1000);
    t->add(5000, 100, 0);
    t->add(5000, 0, 1000);
    t = storyboard.addTimeline(2, 0);
    t = storyboard.addTimeline(3, 0);
    t = storyboard.addTimeline(4, 0);
    t = storyboard.addTimeline(5, 0);
    t = storyboard.addTimeline(6, 0);
    t = storyboard.addTimeline(7, 0);
    t = storyboard.addTimeline(8, 0);

    for (uint8_t i = 9; i <= 40; i++)
    {
      t = storyboard.addTimeline(i, 2);
      t->add(250 * (i - 9), 100, 0);
      t->add(250 * (i - 8), 0, 0);
    }
  }

  storyboardPlayer.init();
}

const int BUFFER_SIZE = 48;
char readBuffer[BUFFER_SIZE] = {0};

void Presepio::mainLoop()
{
  bool printDebug = true;

  storyboardPlayer.mainLoop();

  if (pc.readable())
  {
    pc.gets(readBuffer, BUFFER_SIZE - 1);
    //pc.printf("Received <%s> with %i chars\n", readBuffer, strlen(readBuffer));
    if (strcmp(readBuffer, "play\n") == 0) {
      storyboardPlayer.play();
      pc.printf("Playing...\n");
    } else if (strcmp(readBuffer, "pause\n") == 0) {
      storyboardPlayer.pause();
      pc.printf("Paused\n");
    } else if (strcmp(readBuffer, "stop\n") == 0) {
      storyboardPlayer.stop();
      pc.printf("Stopped\n");
    }
  }

  if (tick_received)
  {
    tick_received = false;

    if (printDebug)
    {
      if (currTime % 500 == 0)
      {
        //int playBufferSize = (playBufferHead >= playBufferTail ? playBufferHead - playBufferTail : playBufferCount - (playBufferTail - playBufferHead));
        //Os::debug("Debug info: time=%lli ms, play=%d, storyboardTime=%lli ms, playBufferMaxTime=%i ms, playBufferSize=%i\n", currTime, play, storyboardTime, playBufferMaxTime, playBufferSize);
        triac_board.debugPrintOutputs();
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

  millisec64 prevCurrTime = currTime;
  currTime = 1000 * tick_count / TICKS_PER_SECOND;

  //Send only the delta, so storyboardPlayer can keep its time with just a sum
  storyboardPlayer.tick(currTime - prevCurrTime);
}
