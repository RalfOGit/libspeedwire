
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
uint16_t SpeedwireEmeterProtocol::getSusyID(void) {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_susy_id_offset);
}

// get serial number
uint32_t SpeedwireEmeterProtocol::getSerialNumber(void) {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_serial_number_offset);
}

// get ticker
uint32_t SpeedwireEmeterProtocol::getTime(void) {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_time_offset);
}

// get pointer to first obis element in udp packet
void *const SpeedwireEmeterProtocol::getFirstObisElement(void) {
    uint8_t *first_element = udp + sma_time_offset + sma_time_size;
    if ((std::uintptr_t)(first_element - udp) > size) {
        return NULL;
    }
    return first_element;
}

// get pointer to next obis element starting from the given element
void *const SpeedwireEmeterProtocol::getNextObisElement(const void *const current_element) {
    uint8_t *const next_element = ((uint8_t *const)current_element) + getObisLength(current_element);
    // check if the next element including the 4-byte obis head is inside the udp packet
    if ((std::uintptr_t)(next_element + 4 - udp) > size) {
        return NULL;
    }
    // check if the entire next element is inside the udp packet
    if ((std::uintptr_t)(next_element + getObisLength(next_element) - udp) > size ) {
        return NULL;
    }
    return next_element;
}


// methods to get obis information with udp_ptr pointing to the first byte of the obis field
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

void SpeedwireEmeterProtocol::printObisElement(const void *const current_element, FILE *file) {
    uint8_t type = getObisType(current_element);
    fprintf(file, "%d.%d.%d.%d ", getObisChannel(current_element), getObisIndex(current_element), type, getObisTariff(current_element));
    if (type == 4 || type == 7) {
        fprintf(file, "0x%08lx %lu\n", getObisValue4(current_element), getObisValue4(current_element));
    }
    else if (type == 8) {
        fprintf(file, "0x%016llx %llu\n", getObisValue8(current_element), getObisValue8(current_element));
    }
    else if (type == 0 && getObisChannel(current_element) == sma_firmware_version_channel) {
        uint32_t version = getObisValue4(current_element);
        uint8_t array[sizeof(uint32_t)];
        memcpy(array, &version, sizeof(array));
        fprintf(file, "%u.%u.%u.%u\n", array[3], array[2], array[1], array[0]);
    }
    else {
        fprintf(file, "\n");
    }
}
