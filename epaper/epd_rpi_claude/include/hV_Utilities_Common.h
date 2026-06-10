///
/// @file hV_Utilities_Common.h
/// @brief Common utility functions – Raspberry Pi port
///

#ifndef hV_UTILITIES_COMMON_RELEASE
#define hV_UTILITIES_COMMON_RELEASE 812

#include "hV_HAL_Peripherals.h"
#include "hV_Configuration.h"
#include <algorithm>

// min / max (portable)
template<typename T> T hV_min(T a, T b) { return (a < b) ? a : b; }
template<typename T> T hV_max(T a, T b) { return (a > b) ? a : b; }

// Re-map Arduino min/max macros used in the code
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

void       delay_ms(uint32_t ms);
STRING_TYPE formatString(const char* format, ...);
STRING_TYPE trimString(STRING_TYPE text);
int32_t    cos32x100(int32_t degreesX100);
int32_t    sin32x100(int32_t degreesX100);
STRING_TYPE utf2iso(STRING_TYPE s);
uint16_t   checkRange(uint16_t value, uint16_t valueMin, uint16_t valueMax);
void       setMinMax(uint16_t value, uint16_t& valueMin, uint16_t& valueMax);
uint32_t   roundUp(uint32_t value, uint16_t modulo);

#endif // hV_UTILITIES_COMMON_RELEASE
