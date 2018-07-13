#include "bitLabCore.h"

#include "platform/mbed_stats.h"

bitLabCore::bitLabCore():
  pc(USBTX, USBRX),
  sd(PC_12, PC_11, PC_10, PD_2, "sd"),
  coreTicker(10 * 1000, callback(this, &bitLabCore::tick)) {
}

void bitLabCore::run() {
  init();
  while (true) {
    mainLoop();
  }
}

void bitLabCore::init() {
  pc.baud(115200);

  pc.printf("===== bitLabCore =====\n");
  pc.printf("    version:  0.1a    \n");
  pc.printf("======================\n");
  pc.printf("Used heap: %i bytes\n", getUsedHeap());

  pc.printf("Mounting SD........");
  int sdErrCode = sd.disk_initialize();
  if (sdErrCode == 0)
    pc.printf("[OK]\n");
  else
    pc.printf("[ERR]\n");
}

int bitLabCore::getUsedHeap() {
  mbed_stats_heap_t heap;
  mbed_stats_heap_get(&heap);
  return heap.current_size;
}