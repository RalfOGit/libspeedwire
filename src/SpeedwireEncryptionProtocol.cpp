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

/** Get 8-bit of data from the given offset in the data area. */
uint8_t SpeedwireEncryptionProtocol::getDataUint8(unsigned long byte_offset) const {
    return SpeedwireByteEncoding::getUint8(udp + sma_data_offset + byte_offset);
}

/** Get 32-bit of data from the given offset in the data area. */
uint32_t SpeedwireEncryptionProtocol::getDataUint32(unsigned long byte_offset) const {
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

/** Get an array of 16x 8-bit data from the given offset in the data area. */
std::array<uint8_t, 16> SpeedwireEncryptionProtocol::getDataUint8Array16(const unsigned long byte_offset) const {
    std::array<uint8_t, 16> result;
    getDataUint8Array(byte_offset, result.data(), result.size());
    return result;
}

/** Get an array of 32x 8-bit data from the given offset in the data area. */
std::array<uint8_t, 32> SpeedwireEncryptionProtocol::getDataUint8Array32(const unsigned long byte_offset) const {
    std::array<uint8_t, 32> result;
    getDataUint8Array(byte_offset, result.data(), result.size());
    return result;
}


/** Get a string of at most 16 characters from the given offset in the data area. */
std::string SpeedwireEncryptionProtocol::getString16(const unsigned long byte_offset) const {
    std::array<uint8_t, 16> array16 = getDataUint8Array16(byte_offset);
    std::string result((char*)array16.data(), array16.size());
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

/** Set 8-bit of data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint8(const unsigned long byte_offset, const uint8_t value) {
    SpeedwireByteEncoding::setUint8(udp + sma_data_offset + byte_offset, value);
}

/** Set 32-bit of data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint32(const unsigned long byte_offset, const uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set 64-bit of data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint64(const unsigned long byte_offset, const uint64_t value) {
    SpeedwireByteEncoding::setUint64BigEndian(udp + sma_data_offset + byte_offset, value);
}

/** Set an array of 8-bit data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length) {
    memcpy(udp + sma_data_offset + byte_offset, value, value_length);
}

/** Set an array of 8-bit data at the given offset in the data area. */
void SpeedwireEncryptionProtocol::setDataUint8Array16(const unsigned long byte_offset, const std::array<uint8_t, 16>& value) {
    setDataUint8Array(byte_offset, value.data(), (unsigned long)value.size());
}

/** Set an array of 8-bit data at the given offset in the data area with the given string. */
void SpeedwireEncryptionProtocol::setString16(const unsigned long byte_offset, const std::string& value) {
    std::array<uint8_t, 16> array16;
    array16.fill(0);
    size_t n = (value.length() <= array16.size() ? value.length() : array16.size());
    memcpy(array16.data(), value.data(), n);
    setDataUint8Array16(byte_offset, array16);
}


/** Convert buffer content to hex string. */
std::string SpeedwireEncryptionProtocol::toHexString(uint8_t* buff, const size_t buff_size) {
    std::string result;
    for (size_t i = 0; i < buff_size; ++i) {
        uint8_t high_nibble = (buff[i] >> 4u) & 0x0f;
        uint8_t low_nibble  = buff[i] & 0x0f;
        result.append(1, "012345667890abcdef"[high_nibble]);
        result.append(1, "012345667890abcdef"[low_nibble]);
    }
    return result;
}


/** Print encryption packet */
std::string SpeedwireEncryptionProtocol::toString(void) const {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
        "PacketType %d  SrcSusyID %u  SrcSerialNumber %lu  DstSusyID %u  DstSerialNumber %lu ",
        (int)getPacketType(), getSrcSusyID(), getSrcSerialNumber(), getDstSusyID(), getDstSerialNumber());
    std::string result(buffer);
    switch (getPacketType()) {
    case 0x01: {
        std::array<uint8_t, 16> src_seed = getDataUint8Array16(0);
        std::string src_seed_str = toHexString(src_seed.data(), src_seed.size());
        snprintf(buffer, sizeof(buffer), "SrcSeed %s", src_seed_str.c_str());
        result.append(buffer);
        break;
    }
    case 0x02:
        std::array<uint8_t, 16> src_seed = getDataUint8Array16(0);
        std::array<uint8_t, 16> dst_seed = getDataUint8Array16(16);
        std::string src_seed_str = toHexString(src_seed.data(), src_seed.size());
        std::string dst_seed_str = toHexString(dst_seed.data(), dst_seed.size());
        uint8_t secured = getDataUint8(32);
        std::string str1 = getString16(33);
        std::string str2 = getString16(49);
        snprintf(buffer, sizeof(buffer), "SrcSeed %s DstSeed %s Secured %d RID/Wifi-Password %s PIC %s", src_seed_str.c_str(), dst_seed_str.c_str(), secured, str1.c_str(), str2.c_str());
        result.append(buffer);
        std::array<uint8_t, 32> hash = getDataUint8Array32(65);
        std::string hash_str = toHexString(hash.data(), hash.size());
        snprintf(buffer, sizeof(buffer), "\nSHA2-256 %s", hash_str.c_str());
        result.append(buffer);
        break;
    }
    return result;
}
