/**
 * Interfaces and helper classes for class-based WLED effects.
 *
 * @file This file provides a compatibility layer for persistent effect data (between frames).
 *
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#pragma once

#include <type_traits>

#include "FX.h"

//--------------------------------------------------------------------------------------------------

/** Persistent effect data.
 * Effect implementations shall use this instead of \c SEGENV
 * @note This helper class emulates the same allocation functionality as the Segment class provides.
 * Nevertheless, try to avoid using that feature. Prefer implementing your own class-based effect
 * with member variables, so that allocation isn't needed at all.
 *
 * Dveleoper's note: This class re-implements the allocation stuff from the Segment class instead
 * of forwarding the method calls.
 * That decision was intentional: Although it is bad for code size, this simplifies possible future
 * refactorings of the Segment class, so that the allocation stuff can be eliminated there.
 */
class Segenv
{
public:
  // no general copy & move - this class is intended to be passed as reference to other functions
  Segenv(Segenv &&) = delete;
  Segenv &operator=(const Segenv &) = delete;
  Segenv &operator=(Segenv &&) = delete;

  /** Helper for storing persistent effect data with different datatypes.
   * @note Only one member of the union can be used at the same time!
   * For example, either float_0 \e or int16_0 and int16_1 \e or uint8_0 ... uint8_3 \e or bit_0 ... bit_n
   */
  union Buffer
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
  };
  static_assert(sizeof(Buffer) == 4, "WTF compiler are you using?!?");

  /** Call counter (starts with 0 and is incremented by one after every frame).
   * @note Effect implementations shall use this instead of \c SEGENV.call
   * The value of this counter cannot be manipulated by the effects.
   * If your effect has the need to start all over again, call \c reset() instead.
   */
  const uint32_t &call = _call;

  // all members are initialized with 0
  union
  {
    /** Alternative to step/aux0/aux1 with much more options apout the desired datatype.
     * Do never use both at the same time!
     */
    Buffer buffer[4]; // yes, we spent additional 8 bytes to have two more entries :-)

    struct
    {
      /** Custom "step" variable.
       * @note Effect implementations shall use this instead of \c SEGENV.step
       */
      uint32_t step;

      /** Custom variable.
       * @note Effect implementations shall use this instead of \c SEGENV.aux0
       */
      uint16_t aux0;

      /** Custom variable.
       * @note Effect implementations shall use this instead of \c SEGENV.aux1
       */
      uint16_t aux1;
    };
  };

  /** EXPERIMENTAL
   * ...
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @tparam FX_DATA Type of custom effect data.
   * @param dataPtr Pointer to effect data (which will be redirected).
   * @retval \c true Success; \a dataPtr is now pointing to a valid instance of \a FX_DATA
   * @retval \c false Allocation failed; do \e not use \a dataPtr
   * @note Try to avoid using this method. Prefer implementing your own class-based effect with
   * member variables, so that allocation isn't needed at all.
   */
  template <typename FX_DATA>
  bool getFxData(FX_DATA *&dataPtr)
  {
    static_assert(std::is_default_constructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must have a default constructor");
    // https://en.cppreference.com/w/cpp/language/destructor.html#Trivial_destructor
    static_assert(std::is_trivially_destructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must not have a custom destructor");
    // https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable.html
    // https://en.cppreference.com/w/cpp/language/classes.html#Trivially_copyable_class
    static_assert(std::is_trivially_copyable<FX_DATA>::value,
                  "Incompatible FX_DATA: must be trivially copyable");

    const size_t size = sizeof(FX_DATA);
    // memory already allocated? --> just cast the pointer
    if (_dataSize >= size)
    {
      dataPtr = static_cast<FX_DATA *>(_data);
    }
    // must allocate memory
    else
    {
      if (!allocateData(size))
      {
        dataPtr = nullptr;
        return false;
      }

      dataPtr = new (_data) FX_DATA;
      if (dataPtr != _data)
      {
        // DEBUG_PRINTF_P(PSTR("Segenv %p: Alignment failure [%p/%p]\n"), this, dataPtr, _data);
        onSegenvAllocFailed();
        return false;
      }
    }

    return true;
  }

  /** EXPERIMENTAL
   * ...
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @tparam FX_DATA Type of custom effect data.
   * @param dataPtr Pointer to effect data array (which will be redirected).
   * @param arrayLength Number of elements in the array.
   * @retval \c true Success; \a dataPtr is now pointing to a valid array of \a FX_DATA
   * @retval \c false Allocation failed; do \e not use \a dataPtr
   * @note Try to avoid using this method. Prefer implementing your own class-based effect with
   * member variables, so that allocation isn't needed at all.
   */
  template <typename FX_DATA>
  bool getFxDataArray(FX_DATA *&dataPtr, uint32_t arrayLength)
  {
    static_assert(std::is_default_constructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must have a default constructor");
    // https://en.cppreference.com/w/cpp/language/destructor.html#Trivial_destructor
    static_assert(std::is_trivially_destructible<FX_DATA>::value,
                  "Incompatible FX_DATA: must not have a custom destructor");
    // https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable.html
    // https://en.cppreference.com/w/cpp/language/classes.html#Trivially_copyable_class
    static_assert(std::is_trivially_copyable<FX_DATA>::value,
                  "Incompatible FX_DATA: must be trivially copyable");

    const size_t size = sizeof(FX_DATA) * arrayLength;
    // memory already allocated? --> just cast the pointer
    if (_dataSize >= size)
    {
      dataPtr = static_cast<FX_DATA *>(_data);
    }
    // must allocate memory
    else
    {
      if (!allocateData(size))
      {
        dataPtr = nullptr;
        return false;
      }

      dataPtr = new (_data) FX_DATA[arrayLength];
      if (dataPtr != _data)
      {
        // DEBUG_PRINTF_P(PSTR("Segenv %p: Alignment failure [%p/%p]\n"), this, dataPtr, _data);
        onSegenvAllocFailed();
        return false;
      }
    }

    return true;
  }

  /** Allocate raw effect data buffer (and set all bytes to 0).
   * The effect is marked as broken when the allocation failed, so its rendering function won't be
   * called anymore.
   * @retval \c true Success; data() will provide the allocated buffer.
   * @retval \c false Failed; the effect is broken now. The caller should return immediately.
   * @note Effect implementations shall use this instead of \c SEGENV.allocateData()
   * Nevertheless, try to avoid using this method. Prefer implementing your own class-based effect
   * with member variables, so that allocation isn't needed at all.
   */
  bool allocateData(size_t size);
  void deallocateData();

  /** Get pointer to the raw effect data.
   * Buffer must have been allocated before via \c allocateData()
   * @note Effect implementations shall use this instead of \c SEGENV.data
   * Tipp: Consider using \c getFxData() or \c getFxDataArray() instead of this method.
   */
  byte *data() { return static_cast<byte *>(_data); }

  /// Reset (and deallocate) all data - just as if it were the very first frame.
  void reset();

protected:
  Segenv() : buffer{} {};
  Segenv(const Segenv &other);
  ~Segenv() { deallocateData(); }

  /// Call this method \e after every rendered frame.
  void next() { ++_call; }

  /** This method is called when an allocation has failed.
   * Child class shall mark the effect as broken.
   */
  virtual void onSegenvAllocFailed() = 0;

private:
  static size_t _allDataSize;
  size_t _dataSize = 0;
  void *_data = nullptr;
  uint32_t _call = 0;
};

//--------------------------------------------------------------------------------------------------
