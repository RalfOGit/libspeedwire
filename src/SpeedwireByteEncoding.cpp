#ifdef _WIN32
#include <Winsock2.h>       // for ntohs(), ntohl()
#else
#include <netinet/in.h>     // for ntohs(), ntohl()
#endif
#include <memory.h>         // for memcpy()
#include <SpeedwireByteEncoding.hpp>


// get a uint8_t value from the given packet
uint8_t SpeedwireByteEncoding::getUint8(const void* const udp_ptr) {
    return *(const uint8_t* const)udp_ptr;
}

// set a uint8_t value to the given packet
void SpeedwireByteEncoding::setUint8(void* udp_ptr, const uint8_t value) {
    *(uint8_t*)udp_ptr = value;
}


// get a uint16_t value from the given packet; convert from network to host byte order, i.e. from big endian
uint16_t SpeedwireByteEncoding::getUint16BigEndian(const void* const udp_ptr) {
    uint16_t value_in_nbo;
    memcpy(&value_in_nbo, udp_ptr, sizeof(value_in_nbo));
    uint16_t value = ntohs(value_in_nbo);
    return value;
}

// get a uint32_t value from the given packet; convert from network to host byte order, i.e. from big endian
uint32_t SpeedwireByteEncoding::getUint32BigEndian(const void* const udp_ptr) {
    uint32_t value_in_nbo;
    memcpy(&value_in_nbo, udp_ptr, sizeof(value_in_nbo));
    uint32_t value = ntohl(value_in_nbo);
    return value;
}

// get a uint64_t value from the given packet; convert from network to host byte order, i.e. from big endian
uint64_t SpeedwireByteEncoding::getUint64BigEndian(const void* const udp_ptr) {
    uint64_t hi_value = getUint32BigEndian(udp_ptr);
    uint64_t lo_value = getUint32BigEndian(((uint8_t*)udp_ptr) + sizeof(uint32_t));
    uint64_t value = (hi_value << (sizeof(uint32_t) * 8)) | lo_value;
    return value;
}

// set a uint16_t value to the given packet; convert from host to network byte order, i.e. to big endian
void SpeedwireByteEncoding::setUint16BigEndian(void* udp_ptr, const uint16_t value) {
    uint16_t value_in_nbo = htons(value);
    memcpy(udp_ptr, &value_in_nbo, sizeof(value_in_nbo));
}

// set a uint32_t value to the given packet; convert from host to network byte order, i.e. to big endian
void SpeedwireByteEncoding::setUint32BigEndian(void* udp_ptr, const uint32_t value) {
    uint32_t value_in_nbo = htonl(value);
    memcpy(udp_ptr, &value_in_nbo, sizeof(value_in_nbo));
}

// set a uint64_t value to the given packet; convert from host to network byte order, i.e. to big endian
void SpeedwireByteEncoding::setUint64BigEndian(void* udp_ptr, const uint64_t value) {
    uint64_t mask32bit = 0x00000000ffffffff;
    uint32_t hi_value = (uint32_t)((value >> (sizeof(uint32_t) * 8)) & mask32bit);
    uint32_t lo_value = (uint32_t)(value & mask32bit);
    setUint32BigEndian(udp_ptr, hi_value);
    setUint32BigEndian(((uint8_t*)udp_ptr) + sizeof(uint32_t), lo_value);
}



// get a uint16_t value from the given packet; convert to little endian
uint16_t SpeedwireByteEncoding::getUint16LittleEndian(const void* const udp_ptr) {
    uint16_t value_in_le = ((unsigned char*)udp_ptr)[0];
    value_in_le |= ((uint16_t)((unsigned char*)udp_ptr)[1]) << 8;
    return value_in_le;
}

// get a uint32_t value from the given packet; convert to little endian
uint32_t SpeedwireByteEncoding::getUint32LittleEndian(const void* const udp_ptr) {
    uint32_t value_in_le = ((unsigned char*)udp_ptr)[0];
    value_in_le |= ((uint32_t)((unsigned char*)udp_ptr)[1]) << 8;
    value_in_le |= ((uint32_t)((unsigned char*)udp_ptr)[2]) << 16;
    value_in_le |= ((uint32_t)((unsigned char*)udp_ptr)[3]) << 24;
    return value_in_le;
}

// get a uint64_t value from the given packet; convert to little endian
uint64_t SpeedwireByteEncoding::getUint64LittleEndian(const void* const udp_ptr) {
    uint64_t value_in_le = ((unsigned char*)udp_ptr)[0];
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[1]) << 8;
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[2]) << 16;
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[3]) << 24;
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[4]) << 32;
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[5]) << 40;
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[6]) << 48;
    value_in_le |= ((uint64_t)((unsigned char*)udp_ptr)[7]) << 56;
    return value_in_le;
}


// set a uint16_t value to the given packet; convert to little endian
void SpeedwireByteEncoding::setUint16LittleEndian(void* udp_ptr, const uint16_t value) {
    ((unsigned char*)udp_ptr)[0] = value & 0xff;
    ((unsigned char*)udp_ptr)[1] = (value >> 8) & 0xff;
}

// set a uint32_t value to the given packet; convert to little endian
void SpeedwireByteEncoding::setUint32LittleEndian(void* udp_ptr, const uint32_t value) {
    ((unsigned char*)udp_ptr)[0] = value & 0xff;
    ((unsigned char*)udp_ptr)[1] = (value >> 8) & 0xff;
    ((unsigned char*)udp_ptr)[2] = (value >> 16) & 0xff;
    ((unsigned char*)udp_ptr)[3] = (value >> 24) & 0xff;
}

// set a uint64_t value to the given packet; convert to little endian
void SpeedwireByteEncoding::setUint64LittleEndian(void* udp_ptr, const uint64_t value) {
    ((unsigned char*)udp_ptr)[0] = value & 0xff;
    ((unsigned char*)udp_ptr)[1] = (value >> 8) & 0xff;
    ((unsigned char*)udp_ptr)[2] = (value >> 16) & 0xff;
    ((unsigned char*)udp_ptr)[3] = (value >> 24) & 0xff;
    ((unsigned char*)udp_ptr)[4] = (value >> 32) & 0xff;
    ((unsigned char*)udp_ptr)[5] = (value >> 40) & 0xff;
    ((unsigned char*)udp_ptr)[6] = (value >> 48) & 0xff;
    ((unsigned char*)udp_ptr)[7] = (value >> 56) & 0xff;
}
