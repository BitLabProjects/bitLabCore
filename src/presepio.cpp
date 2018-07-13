#include "presepio.h"
#include "os\os.h"

#include "platform/mbed_stats.h"

#include "storyboard\storyboard_loader.h"

Presepio::Presepio() : storyboard(),
                       relay_board(),
                       triac_board(),
                       storyboardPlayer(&storyboard, &relay_board, &triac_board)
{
}

void Presepio::init()
{
  this->bitLabCore::init();

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

  /*
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
  */
}

void Presepio::tick(millisec64 timeDelta)
{
  //Send only the delta, so storyboardPlayer can keep its time with just a sum
  storyboardPlayer.tick(timeDelta);
}
