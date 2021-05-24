#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireInverterProtocol.hpp>
using namespace libspeedwire;


//SpeedwireInverterProtocol::SpeedwireInverterProtocol(const void* const udp_packet, const unsigned long udp_packet_len) {
//    udp = (uint8_t*)udp_packet;
//    size = udp_packet_len;
//    packet_id = 0x8001;
//}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireHeader instance that encapsulate the SMA header and the pointers to the entire udp packet.
 */
SpeedwireInverterProtocol::SpeedwireInverterProtocol(const SpeedwireHeader& header) {
    udp = header.getPacketPointer() + header.getPayloadOffset();
    size = header.getPacketSize() - header.getPayloadOffset();
}

/** Destructor. */
SpeedwireInverterProtocol::~SpeedwireInverterProtocol(void) {
    udp = NULL;
    size = 0;
}


/** Get destination susy id. */
uint16_t SpeedwireInverterProtocol::getDstSusyID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_dst_susy_id_offset);
}

/** Get destination serial number. */
uint32_t SpeedwireInverterProtocol::getDstSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_dst_serial_number_offset);
}

/** Get destination control word. */
uint16_t SpeedwireInverterProtocol::getDstControl(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_dst_control_offset);
}

/** Get source susy id. */
uint16_t SpeedwireInverterProtocol::getSrcSusyID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_src_susy_id_offset);
}

/** Get source serial number. */
uint32_t SpeedwireInverterProtocol::getSrcSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_src_serial_number_offset);
}

/** Get source control word. */
uint16_t SpeedwireInverterProtocol::getSrcControl(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_src_control_offset);
}

/** Get error code. */
uint16_t SpeedwireInverterProtocol::getErrorCode(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_error_code_offset);
}

/** Fet fragment id. */
uint16_t SpeedwireInverterProtocol::getFragmentID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_fragment_id_offset);
}

/** Get packet id. */
uint16_t SpeedwireInverterProtocol::getPacketID(void) const {
    return SpeedwireByteEncoding::getUint16LittleEndian(udp + sma_packet_id_offset);
}

/** Get command id. */
uint32_t SpeedwireInverterProtocol::getCommandID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_command_id_offset);
}

/** Get first register id. */
uint32_t SpeedwireInverterProtocol::getFirstRegisterID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_first_register_id_offset);
}

/** Get last register id. */
uint32_t SpeedwireInverterProtocol::getLastRegisterID(void) const {
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_last_register_id_offset);
}


/** Get 32-bit of data from the given offset in the data area. */
uint32_t SpeedwireInverterProtocol::getDataUint32(unsigned long byte_offset) const {   // offset 0 is the first byte after last register index
    return SpeedwireByteEncoding::getUint32LittleEndian(udp + sma_data_offset + byte_offset);
}

/** Get 64-bit of data from the given offset in the data area. */
uint64_t SpeedwireInverterProtocol::getDataUint64(unsigned long byte_offset) const {
    return SpeedwireByteEncoding::getUint64LittleEndian(udp + sma_data_offset + byte_offset);
}

/** Get an array of 8-bit data from the given offset in the data area. */
void SpeedwireInverterProtocol::getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const {
    if (buff != NULL) {
        memcpy(buff, udp + sma_data_offset + byte_offset, buff_size);
    }
}


/** Set destination susy id. */
void SpeedwireInverterProtocol::setDstSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_dst_susy_id_offset, value);
}

/** Set destination serial number. */
void SpeedwireInverterProtocol::setDstSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_dst_serial_number_offset, value);
}

/** Sset destination control word. */
void SpeedwireInverterProtocol::setDstControl(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_dst_control_offset, value);
}

/** Set source susy id */
void SpeedwireInverterProtocol::setSrcSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_src_susy_id_offset, value);
}

/** Set source serial number. */
void SpeedwireInverterProtocol::setSrcSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_src_serial_number_offset, value);
}

/** Set source control word. */
void SpeedwireInverterProtocol::setSrcControl(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_src_control_offset, value);
}

/** Set error code. */
void SpeedwireInverterProtocol::setErrorCode(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_error_code_offset, value);
}

/** Set fragment id. */
void SpeedwireInverterProtocol::setFragmentID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_fragment_id_offset, value);
}

/** Set packet id. */
void SpeedwireInverterProtocol::setPacketID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16LittleEndian(udp + sma_packet_id_offset, value);
}

/** Set command id. */
void SpeedwireInverterProtocol::setCommandID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_command_id_offset, value);
}

/** Set first register id. */
void SpeedwireInverterProtocol::setFirstRegisterID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_first_register_id_offset, value);
}

// set last register id. */
void SpeedwireInverterProtocol::setLastRegisterID(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_last_register_id_offset, value);
}

/** Set 32-bit of data at the given offset in the data area. */
void SpeedwireInverterProtocol::setDataUint32(const unsigned long byte_offset, const uint32_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set 64-bit of data at the given offset in the data area. */
void SpeedwireInverterProtocol::setDataUint64(const unsigned long byte_offset, const uint64_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint64LittleEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set an array of 8-bit data at the given offset in the data area. */
void SpeedwireInverterProtocol::setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length) {
    memcpy(udp + sma_data_offset + byte_offset, value, value_length);
}

/** Set trailer long word at data offset. */
void SpeedwireInverterProtocol::setTrailer(const unsigned long offset) {
    setDataUint32(offset, 0x00000000);
}
