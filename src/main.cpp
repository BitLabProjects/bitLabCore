#include "presepio.h"


int main() {
  Presepio presepio;
  presepio.init();
  while(1) {
    presepio.mainLoop();
  }  
}
