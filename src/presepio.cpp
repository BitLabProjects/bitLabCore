#include "presepio.h"

const int RISE_PER_SECOND = 50;
const int TICKS_PER_SECOND = 50 * 100;

Presepio::Presepio() : led_heartbeat(LED1),
                       led_dimming(D6),
                       pc(USBTX, USBRX),
                       main_crossover(D7)
{
}

void Presepio::init()
{
  tick_received = false;
  tick_count = 0;
  ticker.attach(callback(this, &Presepio::tick), 1.0 / TICKS_PER_SECOND);
  main_crossover.rise(callback(this, &Presepio::main_crossover_rise));

  pc.baud(115200);
  pc.printf("===== Presepe =====\n");
  pc.printf(" version: 1.0      \n");
  pc.printf("===================\n");
}

void Presepio::loop()
{
  //Use tick_received for internal clock
  if (rise_received)
  {
    rise_received = false;

    tick_count += 1;
    if (tick_count == TICKS_PER_SECOND)
    {
      tick_count = 0;
      led_heartbeat = !led_heartbeat;
    }
  }
}

//Change led light
void Presepio::dimming()
{
  //percent of high PWM
  static int dim_percent = 0;
  //
  static bool dim_ascending = true;
  //
  static int tick_per_rise_count = 0;

  if (tick_received)
  {
    tick_received = false;
    tick_per_rise_count += 1;

    if (rise_received)
    {
      tick_per_rise_count = 0;
      rise_received = false;
      rise_count += 1;
      //setting dimming percent
      if (dim_ascending)
      {
        if (dim_percent < 100)
          dim_percent += 1;
        else
          dim_ascending = false;
      }
      else
      {
        if (dim_percent > 0)
          dim_percent -= 1;
        else
          dim_ascending = true;
      }
    }

    if (tick_per_rise_count < dim_percent)
      led_dimming = 1;
    else
      led_dimming = 0;

    if (rise_count == RISE_PER_SECOND)
    {
      rise_count = 0;
      led_heartbeat = !led_heartbeat;
    }
  }
}

void Presepio::tick()
{
  tick_received = true;
}

void Presepio::main_crossover_rise()
{
  rise_received = true;
}
