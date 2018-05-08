#include "triac_board.h"
#include "utils.h"

TriacBoard::TriacBoard() : led_heartbeat(LED2),
                           outputs({(D2), (D3), (D4), (D5), (D6), (D7), (D8), (D9)}),
                           main_crossover(D10)
{
  tick_per_rise_count = 0;

  for (int i = 0; i < ANALOGOUT_COUNT; i++) {
    states[i].reset(); 
  }

  if (SIMULATE_VAC)
    zerocross_sim.attach(callback(this, &TriacBoard::main_crossover_rise), 1.0 / RISE_PER_SECOND);
  else
    main_crossover.rise(callback(this, &TriacBoard::main_crossover_rise));
}

void TriacBoard::setOutput(int idx, int value, millisec startTime, millisec duration)
{
  __disable_irq();
  states[idx].set(value, startTime, duration);
  __enable_irq();
}

void TriacBoard::updateOutputs(millisec time)
{
  tick_per_rise_count += 1;
  // set/reset each out based on percent
  for (int out = 0; out < ANALOGOUT_COUNT; out++)
  {
    states[out].update(time);

    int valueToSet;
    int low_ticks = TICKS_PER_RISE * ((100.0 - (states[out].value)) / 100.0);

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

    outputs[out] = valueToSet;
  }
}

void TriacBoard::debugPrintOutputs(Serial& serial) {
  for (int out = 0; out < ANALOGOUT_COUNT; out++)
  {
    serial.printf("#%i=%3i,", out, states[out].value);
  }
  serial.printf("\r\n");
}

void TriacBoard::main_crossover_rise()
{
  tick_per_rise_count = 0;
  rise_count += 1;
  if (rise_count == RISE_PER_SECOND)
  {
    rise_count = 0;
    led_heartbeat = !led_heartbeat;
  }
}