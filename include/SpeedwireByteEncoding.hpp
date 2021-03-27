#ifndef __SPEEDWIREBYTEENCODINGL_H__
#define __SPEEDWIREBYTEENCODINGL_H__

#include <cstdint>


class SpeedwireByteEncoding {

public:

    static uint8_t getUint8(const void* const udp_ptr);
    static void setUint8(void* udp_ptr, const uint8_t value);

    // methods to get and set field value from and to big endian format, i.e. standard network byte order
    static uint16_t getUint16BigEndian(const void* const udp_ptr);
    static uint32_t getUint32BigEndian(const void* const udp_ptr);
    static uint64_t getUint64BigEndian(const void* const udp_ptr);
    static void     setUint16BigEndian(void* udp_ptr, const uint16_t value);
    static void     setUint32BigEndian(void* udp_ptr, const uint32_t value);
    static void     setUint64BigEndian(void* udp_ptr, const uint64_t value);

    // methods to get and set field value from and to little endian format
    static uint16_t getUint16LittleEndian(const void* const udp_ptr);
    static uint32_t getUint32LittleEndian(const void* const udp_ptr);
    static uint64_t getUint64LittleEndian(const void* const udp_ptr);
    static void     setUint16LittleEndian(void* udp_ptr, const uint16_t value);
    static void     setUint32LittleEndian(void* udp_ptr, const uint32_t value);
    static void     setUint64LittleEndian(void* udp_ptr, const uint64_t value);
};

#endif
