#include "os.h"

#include "platform/mbed_stats.h"

int Os::getUsedHeap() {
  mbed_stats_heap_t heap;
  mbed_stats_heap_get(&heap);
  return heap.current_size;
}

volatile millisec64 mCurrTime;
void Os::setCurrTime(millisec64 value) {
  mCurrTime = value;
}

millisec64 Os::currTime() {
  return mCurrTime;
}