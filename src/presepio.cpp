#include "presepio.h"

const int TICKS_PER_SECOND = 50 * 100;

Presepio::Presepio():
  led_heartbeat(LED1),
  pc(USBTX, USBRX) {

}

void Presepio::init() {
  tick_received = false;
  tick_count = 0;
  ticker.attach(callback(this, &Presepio::tick), 1.0 / TICKS_PER_SECOND);

  pc.baud(115200);
  pc.printf("===== Presepe =====\n");
  pc.printf(" version: 1.0      \n");
  pc.printf("===================\n");
}

void Presepio::loop() {
  if (tick_received) {
    tick_received = false;

    tick_count += 1;
    if (tick_count == TICKS_PER_SECOND) {
      tick_count = 0;
      led_heartbeat = !led_heartbeat;
    }
  }
}

void Presepio::tick() {
  tick_received = true;
}
