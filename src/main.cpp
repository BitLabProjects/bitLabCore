#include "presepio.h"

Presepio presepio;

int main() {
  presepio.init();
  while(1) {
    presepio.loop();
  }
}

// Serial pc(USBTX, USBRX);
// DigitalIn mybutton(USER_BUTTON);
// DigitalOut myled(LED1);

// int main()
// {
//   pc.baud(115200);
//   pc.printf("===== Presepe =====\n");
//   pc.printf(" version: 1.0      \n");
//   pc.printf("===================\n");

//   while (1)
//   {
//     if (mybutton == 0)
//     {                 // Button is pressed
//       myled = !myled; // Toggle the LED state
//       pc.printf("Toggle!\n");
//       wait(0.2); // 200 ms
//     }
//   }
// }