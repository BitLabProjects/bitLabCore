#include "presepio.h"

const int RISE_PER_SECOND = 50;
const int TICKS_PER_SECOND = 50 * 100;
const int TICKS_PER_TENTHOFASECOND = TICKS_PER_SECOND / 10;

const int TIMELINE_DURATION = 50;

int analog_timeline[8][8] = {{0,100, 10,0,   20,0,   30,100},
                             {0,0,   10,100, 20,0,   30,100},
                             {0,0,   10,0,   20,100, 30,0  },
                             {1600, 0, 200, 60, 500, 0, 0, 0},
                             {2800, 0, 200, 40, 300, 40, 200, 0},
                             {1850, 0, 500, 25, 500, 25, 150, 0},
                             {2450, 0, 150, 40, 200, 40, 120, 0},
                             {2500, 0, 500, 45, 400, 45, 250, 0}};

Presepio::Presepio() : led_heartbeat(LED2),
                       analog0(D9),
                       analog1(D10),
                       analog2(D2),
                       analog3(D3),
                       analog4(D4),
                       analog5(D5),
                       analog6(D6),
                       analog7(D7),
                       pc(USBTX, USBRX),
                       main_crossover(D8)
{
}

void Presepio::init()
{
  curr_time = 0;
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

//Change leds light based on timeline
void Presepio::dimming()
{
  //
  static int tick_per_rise_count = 0;
  static int dim_percent[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  if (tick_received)
  {
    tick_count += 1;
    tick_received = false;
    tick_per_rise_count += 1;

    if (tick_count == TICKS_PER_TENTHOFASECOND)
    {
      tick_count = 0;
      curr_time += 1;
    }

    //restart timeline
    if (curr_time > TIMELINE_DURATION)
      curr_time = 0;

    //Update percent for each out
    int out, time_value;
    for (out = 0; out < 8; out++)
    {
      for (time_value = 0; time_value < 4; time_value++)
      {
        int t = analog_timeline[out][time_value * 2];
        int v = analog_timeline[out][time_value * 2 + 1];
        if (curr_time == t)
        {
          dim_percent[out] = v;
          //pc.printf("time: %d, out: %d, value: %d \n", curr_time, out, v);
        }
      }
    }

    if (rise_received)
    {
      tick_per_rise_count = 0;
      rise_received = false;
      rise_count += 1;
    }

    for (out = 0; out < 8; out++)
    {
      int valueToSet;
      if (tick_per_rise_count < dim_percent[out])
        valueToSet = 1;
      else
        valueToSet = 0;

      switch (out)
      {
      case 0:
        analog0 = valueToSet;
        break;
      case 1:
        analog1 = valueToSet;
        break;
      case 2:
        analog2 = valueToSet;
        break;
      case 3:
        analog3 = valueToSet;
        break;
      case 4:
        analog4 = valueToSet;
        break;
      case 5:
        analog5 = valueToSet;
        break;
      case 6:
        analog6 = valueToSet;
        break;
      case 7:
        analog7 = valueToSet;
        break;
      }
    }

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
