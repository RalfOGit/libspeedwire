#ifndef __SPEEDWIREEMETER_HPP__
#define __SPEEDWIREEMETER_HPP__

#include <cstdint>
#include <stdio.h>
#include <SpeedwireHeader.hpp>


class SpeedwireEmeterProtocol {

protected:
    static const unsigned long sma_susy_id_offset;
    static const unsigned long sma_susy_id_size;
    static const unsigned long sma_serial_number_offset;
    static const unsigned long sma_serial_number_size;
    static const unsigned long sma_time_offset;
    static const unsigned long sma_time_size;
    static const uint8_t       sma_firmware_version_channel;

    uint8_t *udp;
    unsigned long size;

public:
    SpeedwireEmeterProtocol(const void* const udp_packet, const unsigned long udp_packet_size);
    SpeedwireEmeterProtocol(SpeedwireHeader &protocol);
    ~SpeedwireEmeterProtocol(void);

    // accessor methods
    uint16_t    getSusyID(void);
    uint32_t    getSerialNumber(void);
    uint32_t    getTime(void);
    void *const getFirstObisElement(void);
    void *const getNextObisElement(const void *const current_element);

    // methods to get obis information with udp_ptr pointing to the first byte of the given obis field
    static uint8_t getObisChannel(const void *const current_element);
    static uint8_t getObisIndex(const void *const current_element);
    static uint8_t getObisType(const void *const current_element);
    static uint8_t getObisTariff(const void *const current_element);
    static uint32_t getObisValue4(const void *const current_element);
    static uint64_t getObisValue8(const void *const current_element);
    static unsigned long getObisLength(const void *const current_element);
    static void printObisElement(const void *const current_element, FILE *file);
};

#endif