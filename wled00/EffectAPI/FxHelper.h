/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <bitset>
#include <type_traits>

//--------------------------------------------------------------------------------------------------

/// Helper class for effects that need periodic triggers for their algorithms.
class PeriodicTrigger
{
public:
  explicit PeriodicTrigger(uint32_t ms = 0) : _delta{ms} {}

  /** Check the trigger state.
   * @param now The current timestamp.
   * @retval \c true The trigger fired (happens only once per trigger time).
   * @retval \c false Trigger still pending; nothing to do.
   */
  bool check(uint32_t now);

  void set_ms(uint32_t ms) { _delta = ms; } // 0 = off
  void set_FPS(uint8_t fps = 50) { set_ms(fps ? 1000 / fps : 0); }

private:
  uint32_t _triggerTime = 0;
  uint32_t _delta;
};

//--------------------------------------------------------------------------------------------------

/** Segenv helper for storing persistent effect data with different datatypes.
 * @note Only one member of the union can be used at the same time!
 * For example, either float_0 \e or int16_0 and int16_1 \e or uint8_0 ... uint8_3 \e or bit_0 ... bit_n
 */
union BufferUnion
{
  float float_0;
  int32_t int32_0;
  uint32_t uint32_0;
  struct
  {
    int16_t int16_0;
    int16_t int16_1;
  };
  struct
  {
    uint16_t uint16_0;
    uint16_t uint16_1;
  };
  struct
  {
    int8_t int8_0;
    int8_t int8_1;
    int8_t int8_2;
    int8_t int8_3;
  };
  struct
  {
    uint8_t uint8_0;
    uint8_t uint8_1;
    uint8_t uint8_2;
    uint8_t uint8_3;
  };
  struct
  {
    unsigned bit_0 : 1;
    unsigned bit_1 : 1;
    unsigned bit_2 : 1;
    unsigned bit_3 : 1;
    unsigned bit_4 : 1;
    unsigned bit_5 : 1;
    unsigned bit_6 : 1;
    unsigned bit_7 : 1;
    // could be extended up to bit_31
  };
  std::bitset<32> bits;
};
static_assert(sizeof(BufferUnion) == 4, "incompatible compiler");
static_assert(std::is_trivially_copyable<BufferUnion>::value, "incompatible compiler");

//--------------------------------------------------------------------------------------------------
