#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireEncryptionProtocol.hpp>
using namespace libspeedwire;


//SpeedwireEncryptionProtocol::SpeedwireEncryptionProtocol(const void* const udp_packet, const unsigned long udp_packet_len) {
//    udp = (uint8_t*)udp_packet;
//    size = udp_packet_len;
//}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireHeader instance that encapsulate the SMA header and the pointers to the entire udp packet.
 */
SpeedwireEncryptionProtocol::SpeedwireEncryptionProtocol(const SpeedwireHeader& header) :
    SpeedwireEncryptionProtocol(SpeedwireData2Packet(header)) {
}

/**
 *  Constructor.
 *  @param header Reference to the SpeedwireData2Packet instance that encapsulate the data2 tag header.
 */
SpeedwireEncryptionProtocol::SpeedwireEncryptionProtocol(const SpeedwireData2Packet& data2_packet) {
    //uint16_t tag_length  = data2_packet.getTagLength();           // 2 bytes
    //uint16_t tag_id      = data2_packet.getTagId();               // 2 bytes
    //uint16_t protocol_id = data2_packet.getProtocolID();          // 2 bytes
    unsigned long payload_offset = data2_packet.getPayloadOffset(); // returns 2 + 2 + 2 = 6
    udp = data2_packet.getPacketPointer() + payload_offset;         // udp points to the byte after the control byte
    size = data2_packet.getTotalLength() - payload_offset;
}


/** Destructor. */
SpeedwireEncryptionProtocol::~SpeedwireEncryptionProtocol(void) {
    udp = NULL;
    size = 0;
}


/** Get packet type. */
uint8_t SpeedwireEncryptionProtocol::getPacketType(void) const {
    return SpeedwireByteEncoding::getUint8(udp + sma_packet_type_offset);
}

/** Get destination susy id. */
uint16_t SpeedwireEncryptionProtocol::getDstSusyID(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_dst_susy_id_offset);
}

/** Get destination serial number. */
uint32_t SpeedwireEncryptionProtocol::getDstSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_dst_serial_number_offset);
}

/** Get source susy id. */
uint16_t SpeedwireEncryptionProtocol::getSrcSusyID(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_src_susy_id_offset);
}

/** Get source serial number. */
uint32_t SpeedwireEncryptionProtocol::getSrcSerialNumber(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_src_serial_number_offset);
}

/** Get 32-bit of data from the given offset in the data area. */
uint32_t SpeedwireEncryptionProtocol::getDataUint32(unsigned long byte_offset) const {   // offset 0 is the first byte after last register index
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_data_offset + byte_offset);
}

/** Get 64-bit of data from the given offset in the data area. */
uint64_t SpeedwireEncryptionProtocol::getDataUint64(unsigned long byte_offset) const {
    return SpeedwireByteEncoding::getUint64BigEndian(udp + sma_data_offset + byte_offset);
}

/** Get an array of 8-bit data from the given offset in the data area. */
void SpeedwireEncryptionProtocol::getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const {
    if (buff != NULL) {
        memcpy(buff, udp + sma_data_offset + byte_offset, buff_size);
    }
}

/** Print all raw data elements given in this inverter packet */
std::string SpeedwireEncryptionProtocol::toString(void) const {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
        "PacketType %d  SrcSusyID %u (0x%04x)  SrcSerialNumber %lu (0x%06lx)  DstSusyID %u (0x%04x)  DstSerialNumber %lu (0x%06lx)",
        (int)getPacketType(),
        getSrcSusyID(), getSrcSusyID(), getSrcSerialNumber(), getSrcSerialNumber(), 
        getDstSusyID(), getDstSusyID(), getDstSerialNumber(), getDstSerialNumber());
    std::string result(buffer);
    return result;
}


/** Set packet type. */
void SpeedwireEncryptionProtocol::setPacketType(const uint8_t value) {
    SpeedwireByteEncoding::setUint8(udp + sma_packet_type_offset, value);
}

/** Set destination susy id. */
void SpeedwireEncryptionProtocol::setDstSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_dst_susy_id_offset, value);
}

/** Set destination serial number. */
void SpeedwireEncryptionProtocol::setDstSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_dst_serial_number_offset, value);
}

/** Set source susy id */
void SpeedwireEncryptionProtocol::setSrcSusyID(const uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_src_susy_id_offset, value);
}

/** Set source serial number. */
void SpeedwireEncryptionProtocol::setSrcSerialNumber(const uint32_t value) {
    SpeedwireByteEncoding::setUint32LittleEndian(udp + sma_src_serial_number_offset, value);
}

/** Set 32-bit of data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint32(const unsigned long byte_offset, const uint32_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set 64-bit of data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint64(const unsigned long byte_offset, const uint64_t value) {  // offset 0 is the first byte after last register index
    SpeedwireByteEncoding::setUint64BigEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set an array of 8-bit data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length) {
    memcpy(udp + sma_data_offset + byte_offset, value, value_length);
}
