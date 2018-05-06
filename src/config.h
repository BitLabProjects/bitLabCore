#ifndef _CONFIG_H_
#define _CONFIG_H_

// this file contains tickers, clocks and timeline settings

// internal clock ticks per second
#define TICKS_PER_SECOND (100 * 100)
#define TICKS_PER_TENTHOFASECOND (TICKS_PER_SECOND / 10)

// switch to simulate 50Hz signal
#define SIMULATE_VAC true
// zero-crossings per second (signal ~ 50 Hz)
#define RISE_PER_SECOND 100

// ticks between two zero-crossings
#define TICKS_PER_RISE (TICKS_PER_SECOND / RISE_PER_SECOND)
// ticks needed to activate TRIAC till the next crossover
#define GATE_TICKS (TICKS_PER_RISE * 1 / 100)

// 1/10sec
#define TIMELINE_DURATION 40
#define ANALOGOUT_COUNT 8
#define TIMELINE_ENTRIES 10
#define TIME_PERCENT_ITEMS (TIMELINE_ENTRIES * 2)

#endif