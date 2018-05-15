#include "os.h"

#include "platform/mbed_stats.h"

int Os::getUsedHeap() {
  mbed_stats_heap_t heap;
  mbed_stats_heap_get(&heap);
  return heap.current_size;
}