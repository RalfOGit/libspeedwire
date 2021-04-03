#ifndef __SPEEDWIREEMETER_HPP__
#define __SPEEDWIREEMETER_HPP__

#include <cstdint>
#include <stdio.h>
#include <string>
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
    uint16_t    getSusyID(void) const;
    uint32_t    getSerialNumber(void) const;
    uint32_t    getTime(void) const;
    void        setSusyID(const uint16_t susy);
    void        setSerialNumber(const uint32_t serial);
    void        setTime(const uint32_t time);
    void *const getFirstObisElement(void) const;
    void *const getNextObisElement(const void *const current_element) const;
    void* const setObisElement(void* const current_element, const void* const obis);

    // methods to get obis information with current_element pointing to the first byte of the given obis field
    static uint8_t getObisChannel(const void *const current_element);
    static uint8_t getObisIndex(const void *const current_element);
    static uint8_t getObisType(const void *const current_element);
    static uint8_t getObisTariff(const void *const current_element);
    static uint32_t getObisValue4(const void *const current_element);
    static uint64_t getObisValue8(const void *const current_element);
    static unsigned long getObisLength(const void *const current_element);

    // methods to set obis information with current_element pointing to the first byte of the given obis field
    static void setObisChannel(const void* current_element, const uint8_t channel);
    static void setObisIndex(const void* current_element, const uint8_t index);
    static void setObisType(const void* current_element, const uint8_t type);
    static void setObisTariff(const void* current_element, const uint8_t tariff);
    static void setObisValue4(const void* current_element, const uint32_t value);
    static void setObisValue8(const void* current_element, const uint64_t value);

    // print methods with current_element pointing to the first byte of the given obis field
    static std::string toHeaderString(const void* const current_element);
    static std::string toValueString(const void* const current_element, const bool hex);
    static void printObisElement(const void *const current_element, FILE *file);
};

#endif