#include "bitLabCore.h"

#include "platform/mbed_stats.h"

bitLabCore::bitLabCore() : pc(USBTX, USBRX),
                           #ifdef UseSDCard
                           sdbd(PC_12, PC_11, PC_10, PD_2, 1*1000*1000, 42*1000*1000),
                           fs("sd"),
                           #endif
                           coreTicker(10 * 1000, callback(this, &bitLabCore::tick)),
                           modules()
{
}

void bitLabCore::addModule(CoreModule *module)
{
  modules.push_back(module);
}

void bitLabCore::run()
{
  for (size_t i = 0; i < modules.size(); i++)
  {
    modules[i]->init();
  }

  while (true)
  {
    // TODO Extract into a console interface module
    /*
    const int BUFFER_SIZE = 48;
    char readBuffer[BUFFER_SIZE] = {0};

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

    for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i]->mainLoop();
    }
  }
}

void bitLabCore::init()
{
  pc.baud(115200);

  pc.printf("===== bitLabCore =====\n");
  pc.printf("    version:  0.1a    \n");
  pc.printf("======================\n");
  pc.printf("Used heap: %i bytes\n", getUsedHeap());
  
  pc.printf("Starting core ticker\n");
  coreTicker.start();

  #ifdef UseSDCard
  pc.printf("Mounting SD........");
  int sdErrCode = fs.mount(&sdbd);
  if (sdErrCode == 0)
    pc.printf("[OK]\n");
  else
    pc.printf("[ERR]\n");
  #endif
}

void bitLabCore::tick(millisec64 timeDelta)
{
  for (size_t i = 0; i < modules.size(); i++)
  {
    modules[i]->tick(timeDelta);
  }
}

int bitLabCore::getUsedHeap()
{
  mbed_stats_heap_t heap;
  mbed_stats_heap_get(&heap);
  return heap.current_size;
}