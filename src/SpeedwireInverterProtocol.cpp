#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireInverterProtocol.hpp>


SpeedwireInverterProtocol::SpeedwireInverterProtocol(const void* const udp_packet, const unsigned long udp_packet_len) {
    udp = (uint8_t*)udp_packet;
    size = udp_packet_len;
    packet_id = 0x8001;
}

SpeedwireInverterProtocol::SpeedwireInverterProtocol(SpeedwireHeader& prot) {
    udp = prot.getPacketPointer() + prot.getPayloadOffset();
    size = prot.getPacketSize() - prot.getPayloadOffset();
    packet_id = 0x8001;
}

SpeedwireInverterProtocol::~SpeedwireInverterProtocol(void) {
    udp = NULL;
    size = 0;
}


// get destination susy id
uint16_t SpeedwireInverterProtocol::getDstSusyID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_dst_susy_id_offset);
}

// get destination serial number
uint32_t SpeedwireInverterProtocol::getDstSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_dst_serial_number_offset);
}

// get destination control word
uint16_t SpeedwireInverterProtocol::getDstControl(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_dst_control_offset);
}

// get source susy id
uint16_t SpeedwireInverterProtocol::getSrcSusyID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_src_susy_id_offset);
}

// get source serial number
uint32_t SpeedwireInverterProtocol::getSrcSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_src_serial_number_offset);
}

// get source control word
uint16_t SpeedwireInverterProtocol::getSrcControl(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_src_control_offset);
}

// get error code
uint16_t SpeedwireInverterProtocol::getErrorCode(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_error_code_offset);
}

// get fragment id
uint16_t SpeedwireInverterProtocol::getFragmentID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_fragment_id_offset);
}

// get packet id
uint16_t SpeedwireInverterProtocol::getPacketID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_packet_id_offset);
}

// get command id0
uint32_t SpeedwireInverterProtocol::getCommandID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_command_id_offset);
}

// get first register id
uint32_t SpeedwireInverterProtocol::getFirstRegisterID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_first_register_id_offset);
}

// get last register id
uint32_t SpeedwireInverterProtocol::getLastRegisterID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_last_register_id_offset);
}


// get 32-bit of data 
uint32_t SpeedwireInverterProtocol::getDataUint32(unsigned long byte_offset) const {   // offset 0 is the first byte after last register index
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_data_offset + byte_offset);
}

// get 64-bit of data
uint64_t SpeedwireInverterProtocol::getDataUint64(unsigned long byte_offset) const {
    return SpeedwireByteEncoding::getUint64LittleEndian(udp + sma_data_offset + byte_offset);
}

// get an array of 8-bit data
void SpeedwireInverterProtocol::getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const {
    if (buff != NULL) {
        memcpy(buff, udp + sma_data_offset + byte_offset, buff_size);
    }
}


// set destination susy id
void SpeedwireInverterProtocol::setDstSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_dst_susy_id_offset, value);
}

// set destination serial number
void SpeedwireInverterProtocol::setDstSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_dst_serial_number_offset, value);
}

// set destination control word
void SpeedwireInverterProtocol::setDstControl(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_dst_control_offset, value);
}

// set source susy id
void SpeedwireInverterProtocol::setSrcSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_src_susy_id_offset, value);
}

// set source serial number
void SpeedwireInverterProtocol::setSrcSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_src_serial_number_offset, value);
}

// set source control word
void SpeedwireInverterProtocol::setSrcControl(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_src_control_offset, value);
}

// set error code
void SpeedwireInverterProtocol::setErrorCode(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_error_code_offset, value);
}

// set fragment id
void SpeedwireInverterProtocol::setFragmentID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_fragment_id_offset, value);
}

// set packet id
void SpeedwireInverterProtocol::setPacketID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_packet_id_offset, value);
}

// set command id
void SpeedwireInverterProtocol::setCommandID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_command_id_offset, value);
}

// set first register id
void SpeedwireInverterProtocol::setFirstRegisterID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_first_register_id_offset, value);
}

// set last register id
void SpeedwireInverterProtocol::setLastRegisterID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_last_register_id_offset, value);
}

// set 32-bit of data
void SpeedwireInverterProtocol::setDataUint32(const unsigned long byte_offset, const uint32_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_data_offset + byte_offset, value);
}

// set 64-bit of data
void SpeedwireInverterProtocol::setDataUint64(const unsigned long byte_offset, const uint64_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint64LittleEndian(udp + sma_data_offset + byte_offset, value);
}

// set an array of 8-bit data
void SpeedwireInverterProtocol::setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length) {
    memcpy(udp + sma_data_offset + byte_offset, value, value_length);
}

// set trailer long word at data offset
void SpeedwireInverterProtocol::setTrailer(const unsigned long offset) {
    setDataUint32(offset, 0x00000000);
}
