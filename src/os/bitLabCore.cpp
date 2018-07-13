#include "bitLabCore.h"

bitLabCore::bitLabCore() {

}

void bitLabCore::run() {
  init();
  while (true) {
    mainLoop();
  }
}