#include "presepio.h"
#include "os\os.h"

#include "platform/mbed_stats.h"

#include "storyboard\storyboard_loader.h"

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

  const char *jsonContent = "{  \"timelines\": [    {      \"name\": \"timeline1\",      \"outputId\": 1,      \"outputType\": 0,      \"entries\": [        {          \"time\": 0,          \"value\": 100,          \"duration\": 500        },        {          \"time\": 500,          \"value\": 20,          \"duration\": 250        },        {          \"time\": 750,          \"value\": 50,          \"duration\": 100        }      ]    },    {      \"name\": \"timeline2\",      \"outputId\": 2,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    },    {      \"name\": \"timeline3\",      \"outputId\": 3,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    },    {      \"name\": \"timeline4\",      \"outputId\": 4,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    },    {      \"name\": \"timeline5\",      \"outputId\": 5,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    },    {      \"name\": \"timeline6\",      \"outputId\": 6,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    },    {      \"name\": \"timeline7\",      \"outputId\": 7,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    },    {      \"name\": \"timeline8\",      \"outputId\": 8,      \"outputType\": 1,      \"entries\": [        {          \"time\": 0,          \"value\": 1,          \"duration\": 125        },        {          \"time\": 125,          \"value\": 0,          \"duration\": 1000        },        {          \"time\": 1125,          \"value\": 1,          \"duration\": 250        }      ]    }  ]}";
  StoryboardLoader loader(&storyboard, jsonContent);
  loader.load();

  storyboardPlayer.init();
}

const int BUFFER_SIZE = 48;
char readBuffer[BUFFER_SIZE] = {0};

void Presepio::mainLoop()
{
  bool printDebug = false;

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
