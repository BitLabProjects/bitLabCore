#include "presepio.h"

// switch to simulate 50Hz signal
const bool SIMULATE_VAC = true;

// zero-crossings per second (signal ~ 50 Hz)
const int RISE_PER_SECOND = 100;
// internal clock ticks per second
const int TICKS_PER_SECOND = 50 * 100;
const int TICKS_PER_TENTHOFASECOND = TICKS_PER_SECOND / 10;
// ticks between two zero-crossings
const int TICKS_PER_RISE = TICKS_PER_SECOND / RISE_PER_SECOND;

// 1/10sec
const int TIMELINE_DURATION = 32;

const int ANALOGOUT_COUNT = 8;
const int TIMELINE_ENTRIES = 4;
const int TIME_PERCENT_PAIRS = TIMELINE_ENTRIES * 2;

int analog_timeline[TIME_PERCENT_PAIRS][ANALOGOUT_COUNT] = {{0, 10, 8,  20, 16, 30, 24, 0},
                                                            {1, 10, 9,  20, 17, 30, 25, 0},
                                                            {2, 10, 10, 20, 18, 30, 26, 0},
                                                            {3, 10, 11, 20, 19, 30, 27, 0},
                                                            {4, 10, 12, 20, 20, 30, 28, 0},
                                                            {5, 10, 13, 20, 21, 30, 29, 0},
                                                            {6, 10, 14, 20, 22, 30, 30, 0},
                                                            {7, 10, 15, 20, 23, 30, 31, 0}};

Presepio::Presepio() : led_heartbeat(LED2),
                       analog0(D2),
                       analog1(D3),
                       analog2(D4),
                       analog3(D5),
                       analog4(D6),
                       analog5(D7),
                       analog6(D8),
                       analog7(D9),
                       analogOutMap({analog0, analog1, analog2, analog3, analog4, analog5, analog6, analog7}),
                       pc(USBTX, USBRX),
                       main_crossover(D10)
{
}

void Presepio::init()
{
  curr_time = 0;
  tick_received = false;
  tick_count = 0;
  ticker.attach(callback(this, &Presepio::tick), 1.0 / TICKS_PER_SECOND);

  if (SIMULATE_VAC)
    zerocross_sim.attach(callback(this, &Presepio::main_crossover_rise), 1.0 / RISE_PER_SECOND);
  else
    main_crossover.rise(callback(this, &Presepio::main_crossover_rise));

  pc.baud(115200);
  pc.printf("===== Presepe =====\n");
  pc.printf(" version: 1.0      \n");
  pc.printf("===================\n");

  pc.printf("TICKS_PER_RISE: %d\n", TICKS_PER_RISE);
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
  static int dim_percent[ANALOGOUT_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};

  if (tick_received)
  {
    tick_count += 1;
    tick_received = false;
    tick_per_rise_count += 1;

    if (tick_count > TICKS_PER_TENTHOFASECOND)
    {
      tick_count = 0;
      curr_time += 1;
      //restart timeline
      if (curr_time > TIMELINE_DURATION)
        curr_time = 0;
    }

    if (rise_received)
    {
      tick_per_rise_count = 0;
      rise_received = false;
      rise_count += 1;
      if (rise_count > RISE_PER_SECOND)
      {
        rise_count = 0;
        led_heartbeat = !led_heartbeat;
      }
    }

    //Update percent for each out
    int out, time_value;
    for (out = 0; out < ANALOGOUT_COUNT; out++)
    {
      for (time_value = 0; time_value < TIMELINE_ENTRIES; time_value++)
      {
        int t = analog_timeline[out][time_value * 2];
        int v = analog_timeline[out][time_value * 2 + 1];
        if (curr_time == t)
        {
          dim_percent[out] = v;
        }
      }
    }

    // set/reset each out based on percent
    for (out = 0; out < ANALOGOUT_COUNT; out++)
    {
      int valueToSet;
      int low_ticks = TICKS_PER_RISE * ((100.0 - (dim_percent[out])) / 100.0);

      // @_TODO manage min time for TRIAC gate activation
      // if ((tick_per_rise_count < low_ticks) || (tick_per_rise_count > (low_ticks + GATE_TICKS) ))
      if (tick_per_rise_count > low_ticks)
        valueToSet = 1;
      else
        valueToSet = 0;

      analogOutMap[out] = valueToSet;
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