#ifndef _UTILS_H_
#define _UTILS_H_

#include "os\types.h"

class Utils {
public:

  static inline bool bitIsSet(uint8_t value, uint8_t bit) {
    return (value & (1<<bit)) != 0;
  }
  static inline void nop() {
    __asm volatile (
      "NOP\n\t"
    );
  }
  static inline float clamp01(float value) {
    return max(0, min(value, 1));
  }
  static inline int32_t clamp(int32_t value, int32_t minValue, int32_t maxValue) {
    return max(minValue, min(value, maxValue));
  }
  static inline int32_t min(int32_t a, int32_t b) {
    return a < b ? a : b;
  }
  static inline uint8_t min(uint8_t a, uint8_t b) {
    return a < b ? a : b;
  }
  static inline float min(float a, float b) {
    return a < b ? a : b;
  }
  static inline int32_t max(int32_t a, int32_t b) {
    return a > b ? a : b;
  }
  static inline uint8_t max(uint8_t a, uint8_t b) {
    return a > b ? a : b;
  }
  static inline float max(float a, float b) {
    return a > b ? a : b;
  }
  static inline float abs(int a) {
    return a < 0 ? -a : a;
  }
  static inline float absDiff(int a, int b) {
    return abs(a - b);
  }

  static uint32_t crc32(uint8_t value, uint32_t init);
  static uint32_t crc32(int32_t value, uint32_t init);
  static uint32_t crc32(uint32_t value, uint32_t init);
  static uint32_t crc32(const uint8_t *buf, uint32_t len, uint32_t init);

  static inline void memCopy4(void* dst, void* src) {
    *((uint8_t*)dst+0) = *((uint8_t*)src+0);
    *((uint8_t*)dst+1) = *((uint8_t*)src+1);
    *((uint8_t*)dst+2) = *((uint8_t*)src+2);
    *((uint8_t*)dst+3) = *((uint8_t*)src+3);
  }

  static void strprint(char** dst, uint32_t& size, uint32_t value, uint32_t base = 10);
  static void strprint(char** dst, uint32_t& size, const char* src)
};

#endif