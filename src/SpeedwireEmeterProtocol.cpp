
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireEmeterProtocol.hpp>
using namespace libspeedwire;


//SpeedwireEmeterProtocol::SpeedwireEmeterProtocol(const void *const udp_packet, const unsigned long udp_packet_len) {
//    udp = (uint8_t *)udp_packet;
//    size = udp_packet_len;
//}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireHeader instance that encapsulate the SMA header and the pointers to the entire udp packet.
 */
SpeedwireEmeterProtocol::SpeedwireEmeterProtocol(const SpeedwireHeader& header) :
    SpeedwireEmeterProtocol(SpeedwireData2Packet(header)) {
}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireData2Packet instance that encapsulate the data2 tag header.
 */
SpeedwireEmeterProtocol::SpeedwireEmeterProtocol(const SpeedwireData2Packet& data2_packet) {
    //uint16_t tag_length  = data2_packet.getTagLength();     // 2 bytes
    //uint16_t tag_id      = data2_packet.getTagId();         // 2 bytes
    //uint16_t protocol_id = data2_packet.getProtocolID();    // 2 bytes
    //if (protocol_id == SpeedwireData2Packet::sma_extended_emeter_protocol_id) {
    //    uint8_t  long_words  = data2_packet.getLongWords(); // 1 byte
    //    uint8_t  control     = data2_packet.getControl();   // 1 byte
    //    unsigned long payload_offset = data2_packet.getPayloadOffset(); // returns 2 + 2 + 2 + 1 + 1 = 8
    //}
    //else {
    //    unsigned long payload_offset = data2_packet.getPayloadOffset(); // returns 2 + 2 + 2 = 6
    //}
    unsigned long payload_offset = data2_packet.getPayloadOffset();
    udp = data2_packet.getPacketPointer() + payload_offset; // points to first byte after protocol_id or control byte
    size = data2_packet.getTotalLength() - payload_offset;
}

/** Destructor. */
SpeedwireEmeterProtocol::~SpeedwireEmeterProtocol(void) {
    udp = NULL;
    size = 0;
}

/** Get susy id. */
uint16_t SpeedwireEmeterProtocol::getSusyID(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_susy_id_offset);
}

/** Get serial number. */
uint32_t SpeedwireEmeterProtocol::getSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_serial_number_offset);
}

/** Get timestamp. */
uint32_t SpeedwireEmeterProtocol::getTime(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_time_offset);
}

/** Set susy id. */
void SpeedwireEmeterProtocol::setSusyID(uint16_t susy_id) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_susy_id_offset, susy_id);
}

/** Set serial number. */
void SpeedwireEmeterProtocol::setSerialNumber(uint32_t serial_number) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_serial_number_offset, serial_number);
}

/** Set timestamp. */
void SpeedwireEmeterProtocol::setTime(uint32_t time) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_time_offset, time);
}

/** Get pointer to first obis element in udp packet. */
const void* SpeedwireEmeterProtocol::getFirstObisElement(void) const {
    uint8_t* first_element = udp + sma_first_obis_offset; // sma_time_offset + sma_time_size;
    if ((std::uintptr_t)(first_element - udp) > size) {
        return NULL;
    }
    return first_element;
}

/** Get pointer to next obis element starting from the given element. */
const void* SpeedwireEmeterProtocol::getNextObisElement(const void* const current_element) const {
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

/** Set given obis element right at location of the given current element. */
void* SpeedwireEmeterProtocol::setObisElement(void *const current_element, const void* const obis) {
    const unsigned long obis_length = getObisLength(obis);
    uint8_t *const next_element = ((uint8_t *const)current_element) + obis_length;
    // check if the obis element fits inside the udp packet
    if ((std::uintptr_t)(next_element - udp) > size) {
        return NULL;
    }
    memcpy(current_element, obis, obis_length);
    return next_element;
}


// methods to get obis information with current_element pointing to the first byte of the obis field. */
/** Get obis channel field from the given obis element. */
uint8_t SpeedwireEmeterProtocol::getObisChannel(const void *const current_element) {
    return ((uint8_t*)current_element)[0];
}

/** Get obis index field from the given obis element. */
uint8_t SpeedwireEmeterProtocol::getObisIndex(const void *const current_element) {
    return ((uint8_t*)current_element)[1];
}

/** Get obis type field from the given obis element. */
uint8_t SpeedwireEmeterProtocol::getObisType(const void *const current_element) {
    return ((uint8_t*)current_element)[2];
}

/** Get obis tariff field from the given obis element. */
uint8_t SpeedwireEmeterProtocol::getObisTariff(const void *const current_element) {
    return ((uint8_t*)current_element)[3];
}

/** Get obis uint32_t data from the given obis element. */
uint32_t SpeedwireEmeterProtocol::getObisValue4(const void *const current_element) {
    return SpeedwireByteEncoding::getUint32BigEndian(((uint8_t*)current_element)+4);
}

/** Get obis uint64_t data from the given obis element. */
uint64_t SpeedwireEmeterProtocol::getObisValue8(const void *const current_element) {
    return SpeedwireByteEncoding::getUint64BigEndian(((uint8_t*)current_element)+4);
}

/** Get length of the given obis element. */
unsigned long SpeedwireEmeterProtocol::getObisLength(const void *const current_element) {
    unsigned long type = getObisType(current_element);
    if (getObisChannel(current_element) == sma_firmware_version_channel) {       // the software version has a type of 0, although it has a 4 byte payload
        return 8;
    }
    return 4 + type;
}


// methods to set obis information with current_element pointing to the first byte of the given obis field
/** Set obis channel field in the given obis element. */
void SpeedwireEmeterProtocol::setObisChannel(const void* current_element, const uint8_t channel) {
    ((uint8_t*)current_element)[0] = channel;
}

/** Set obis index field in the given obis element. */
void SpeedwireEmeterProtocol::setObisIndex(const void* current_element, const uint8_t index) {
    ((uint8_t*)current_element)[1] = index;
}

/** Set obis type field in the given obis element. */
void SpeedwireEmeterProtocol::setObisType(const void* current_element, const uint8_t type) {
    ((uint8_t*)current_element)[2] = type;
}

/** Set obis tariff field in the given obis element. */
void SpeedwireEmeterProtocol::setObisTariff(const void* current_element, const uint8_t tariff) {
    ((uint8_t*)current_element)[3] = tariff;
}

/** Set obis uint32_t data in the given obis element. */
void SpeedwireEmeterProtocol::setObisValue4(const void* current_element, const uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(((uint8_t*)current_element) + 4, value);
}

/** Set obis uint64_t data in the given obis element. */
void SpeedwireEmeterProtocol::setObisValue8(const void* current_element, const uint64_t value) {
    SpeedwireByteEncoding::setUint64BigEndian(((uint8_t*)current_element) + 4, value);
}


// print methods with current_element pointing to the first byte of the given obis field
/** Get a string representation of the given obis header fields, this is channel, index, type and tariff. */
std::string SpeedwireEmeterProtocol::toHeaderString(const void* const current_element) {
    char str[32];
    snprintf(str, sizeof(str), "%d.%d.%d.%d", getObisChannel(current_element), getObisIndex(current_element), getObisType(current_element), getObisTariff(current_element));
    return std::string(str);
}

/** Get a string representation of the given obis data value. */
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
            snprintf(str, sizeof(str), (hex == false ? "%u.%u.%u.%c" : "%02x.%02x.%02x.%02x"), array[3], array[2], array[1], array[0]);
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

/** Get a string representation of the given obis element including header and value. */
std::string SpeedwireEmeterProtocol::toString(const void* const current_element) {
    char str[128];
    snprintf(str, sizeof(str), "%s %s %s\n", toHeaderString(current_element).c_str(), toValueString(current_element, true).c_str(), toValueString(current_element, false).c_str());
    return std::string(str);
}
