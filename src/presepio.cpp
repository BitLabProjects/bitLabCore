#include "presepio.h"

// switch to simulate 50Hz signal
const bool SIMULATE_VAC = false;

// zero-crossings per second (signal ~ 50 Hz)
const int RISE_PER_SECOND = 100;
// internal clock ticks per second
const int TICKS_PER_SECOND = 50 * 100;
const int TICKS_PER_TENTHOFASECOND = TICKS_PER_SECOND / 10;
// ticks between two zero-crossings
const int TICKS_PER_RISE = TICKS_PER_SECOND / RISE_PER_SECOND;

const int GATE_TICKS = TICKS_PER_RISE * 1 / 100;

// 1/10sec
const int TIMELINE_DURATION = 46;

const int ANALOGOUT_COUNT = 8;
const int TIMELINE_ENTRIES = 10;
const int TIME_PERCENT_PAIRS = TIMELINE_ENTRIES * 2;

int analog_timeline[ANALOGOUT_COUNT][TIME_PERCENT_PAIRS] = {{0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0},
                                                            {0, 10, 5, 20, 10, 30, 15, 40, 20, 50, 25, 60, 30, 70, 35, 80, 40, 90, 45, 0}};

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

  if (rise_received)
  {
    tick_per_rise_count = 0;
    rise_received = false;
    rise_count += 1;
    if (rise_count == RISE_PER_SECOND)
    {
      rise_count = 0;
      led_heartbeat = !led_heartbeat;
    }
  }

  if (tick_received)
  {
    tick_count += 1;
    tick_received = false;
    tick_per_rise_count += 1;

    if (tick_count == TICKS_PER_TENTHOFASECOND)
    {
      tick_count = 0;
      curr_time += 1;
      //restart timeline
      if (curr_time == TIMELINE_DURATION)
        curr_time = 0;
    }

    if (rise_received)
    {
      tick_per_rise_count = 0;
      rise_received = false;
      rise_count += 1;
      if (rise_count == RISE_PER_SECOND)
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

      if (SIMULATE_VAC)
      {
        if (tick_per_rise_count > low_ticks)
          valueToSet = 1;
        else
          valueToSet = 0;
      }
      else
      {
        // pulse for TRIAC activation
        if ((tick_per_rise_count < low_ticks) || (tick_per_rise_count > (low_ticks + GATE_TICKS)))
          valueToSet = 0;
        else
          valueToSet = 1;
      }

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