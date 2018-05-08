#include "presepio.h"

Presepio presepio;

int main() {
  presepio.init();
  while(1) {
    presepio.playTimeline();
  }  
}
