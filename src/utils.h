#ifndef _UTILS_H_
#define _UTILS_H_

class Utils {
public:

  static inline bool bitIsSet(uint8_t value, uint8_t bit) {
    return (value & (1<<bit)) != 0;
  }
  static inline void nop() {
    __ASM volatile (
      "NOP\n\t"
    );
  }
  static inline float min(float a, float b) {
    return a < b ? a : b;
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
};

#endif