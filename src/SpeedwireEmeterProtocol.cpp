
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireEmeterProtocol.hpp>


const unsigned long SpeedwireEmeterProtocol::sma_susy_id_offset = 0;
const unsigned long SpeedwireEmeterProtocol::sma_susy_id_size   = 2;
const unsigned long SpeedwireEmeterProtocol::sma_serial_number_offset = SpeedwireEmeterProtocol::sma_susy_id_size;
const unsigned long SpeedwireEmeterProtocol::sma_serial_number_size   = 4;
const unsigned long SpeedwireEmeterProtocol::sma_time_offset = SpeedwireEmeterProtocol::sma_serial_number_offset + SpeedwireEmeterProtocol::sma_serial_number_size;
const unsigned long SpeedwireEmeterProtocol::sma_time_size   = 4;
const uint8_t SpeedwireEmeterProtocol::sma_firmware_version_channel = 144;


SpeedwireEmeterProtocol::SpeedwireEmeterProtocol(const void *const udp_packet, const unsigned long udp_packet_len) {
    udp = (uint8_t *)udp_packet;
    size = udp_packet_len;
}

SpeedwireEmeterProtocol::SpeedwireEmeterProtocol(SpeedwireHeader& prot) {
    udp = prot.getPacketPointer() + prot.getPayloadOffset();
    size = prot.getPacketSize() - prot.getPayloadOffset();
}

SpeedwireEmeterProtocol::~SpeedwireEmeterProtocol(void) {
    udp = NULL;
    size = 0;
}

// get susy id
uint16_t SpeedwireEmeterProtocol::getSusyID(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_susy_id_offset);
}

// get serial number
uint32_t SpeedwireEmeterProtocol::getSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_serial_number_offset);
}

// get ticker
uint32_t SpeedwireEmeterProtocol::getTime(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_time_offset);
}

// set susy id
void SpeedwireEmeterProtocol::setSusyID(uint16_t susy_id) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_susy_id_offset, susy_id);
}

// set serial number
void SpeedwireEmeterProtocol::setSerialNumber(uint32_t serial_number) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_serial_number_offset, serial_number);
}

// set ticker
void SpeedwireEmeterProtocol::setTime(uint32_t time) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_time_offset, time);
}

// get pointer to first obis element in udp packet
void *const SpeedwireEmeterProtocol::getFirstObisElement(void) const {
    uint8_t *first_element = udp + sma_time_offset + sma_time_size;
    if ((std::uintptr_t)(first_element - udp) > size) {
        return NULL;
    }
    return first_element;
}

// get pointer to next obis element starting from the given element
void* const SpeedwireEmeterProtocol::getNextObisElement(const void* const current_element) const {
    uint8_t* const next_element = ((uint8_t* const)current_element) + getObisLength(current_element);
    // check if the next element including the 4-byte obis head is inside the udp packet
    if ((std::uintptr_t)(next_element + 4 - udp) > size) {
        return NULL;
    }
    // check if the entire next element is inside the udp packet
    if ((std::uintptr_t)(next_element + getObisLength(next_element) - udp) > size) {
        return NULL;
    }
    return next_element;
}

// set given obis element right at location of the given current element
void *const SpeedwireEmeterProtocol::setObisElement(void *const current_element, const void* const obis) {
    const unsigned long obis_length = getObisLength(obis);
    uint8_t *const next_element = ((uint8_t *const)current_element) + obis_length;
    // check if the obis element fits inside the udp packet
    if ((std::uintptr_t)(next_element - udp) > size) {
        return NULL;
    }
    memcpy(current_element, obis, obis_length);
    return next_element;
}


// methods to get obis information with current_element pointing to the first byte of the obis field
uint8_t SpeedwireEmeterProtocol::getObisChannel(const void *const current_element) {
    return ((uint8_t*)current_element)[0];
}

uint8_t SpeedwireEmeterProtocol::getObisIndex(const void *const current_element) {
    return ((uint8_t*)current_element)[1];
}

uint8_t SpeedwireEmeterProtocol::getObisType(const void *const current_element) {
    return ((uint8_t*)current_element)[2];
}

uint8_t SpeedwireEmeterProtocol::getObisTariff(const void *const current_element) {
    return ((uint8_t*)current_element)[3];
}

uint32_t SpeedwireEmeterProtocol::getObisValue4(const void *const current_element) {
    return SpeedwireByteEncoding::getUint32BigEndian(((uint8_t*)current_element)+4);
}

uint64_t SpeedwireEmeterProtocol::getObisValue8(const void *const current_element) {
    return SpeedwireByteEncoding::getUint64BigEndian(((uint8_t*)current_element)+4);
}

unsigned long SpeedwireEmeterProtocol::getObisLength(const void *const current_element) {
    unsigned long type = getObisType(current_element);
    if (getObisChannel(current_element) == sma_firmware_version_channel) {       // the software version has a type of 0, although it has a 4 byte payload
        return 8;
    }
    return 4 + type;
}


// methods to set obis information with current_element pointing to the first byte of the given obis field
void SpeedwireEmeterProtocol::setObisChannel(const void* current_element, const uint8_t channel) {
    ((uint8_t*)current_element)[0] = channel;
}

void SpeedwireEmeterProtocol::setObisIndex(const void* current_element, const uint8_t index) {
    ((uint8_t*)current_element)[1] = index;
}

void SpeedwireEmeterProtocol::setObisType(const void* current_element, const uint8_t type) {
    ((uint8_t*)current_element)[2] = type;
}

void SpeedwireEmeterProtocol::setObisTariff(const void* current_element, const uint8_t tariff) {
    ((uint8_t*)current_element)[3] = tariff;
}

void SpeedwireEmeterProtocol::setObisValue4(const void* current_element, const uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(((uint8_t*)current_element) + 4, value);
}

void SpeedwireEmeterProtocol::setObisValue8(const void* current_element, const uint64_t value) {
    SpeedwireByteEncoding::setUint64BigEndian(((uint8_t*)current_element) + 4, value);
}


// print methods with current_element pointing to the first byte of the given obis field
std::string SpeedwireEmeterProtocol::toHeaderString(const void* const current_element) {
    char str[32];
    snprintf(str, sizeof(str), "%d.%d.%d.%d", getObisChannel(current_element), getObisIndex(current_element), getObisType(current_element), getObisTariff(current_element));
    return std::string(str);
}

std::string SpeedwireEmeterProtocol::toValueString(const void* const current_element, const bool hex) {
    char str[32];
    uint8_t type = getObisType(current_element);
    if (type == 4 || type == 7) {
        snprintf(str, sizeof(str), (hex == false ? "%lu" : "0x%08lx"), getObisValue4(current_element));
    }
    else if (type == 8) {
        snprintf(str, sizeof(str), (hex == false ? "%llu" : "0x%016llx"), getObisValue8(current_element));
    }
    else if (type == 0) {
        uint8_t channel = getObisChannel(current_element);
        if (channel == sma_firmware_version_channel) {
            uint32_t version = getObisValue4(current_element);
            uint8_t array[sizeof(uint32_t)];
            memcpy(array, &version, sizeof(array));
            snprintf(str, sizeof(str), (hex == false ? "%u.%u.%u.%u" : "%02x.%02x.%02x.%02x"), array[3], array[2], array[1], array[0]);
        }
        else if (channel == 0 && getObisIndex(current_element) == 0 && getObisTariff(current_element) == 0) {
            snprintf(str, sizeof(str), "end of data");
        }
        else {
            str[0] = '\0';
        }
    }
    else {
        snprintf(str, sizeof(str), "unknown data");
    }
    return std::string(str);
}

void SpeedwireEmeterProtocol::printObisElement(const void *const current_element, FILE *file) {
    fprintf(file, "%s %s %s\n", toHeaderString(current_element).c_str(), toValueString(current_element, true).c_str(), toValueString(current_element, false).c_str());
}
