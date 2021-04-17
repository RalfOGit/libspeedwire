#include <memory.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireHeader.hpp>

const uint8_t  SpeedwireHeader::sma_signature[4] = { 0x53, 0x4d, 0x41, 0x00 };     //!< SMA signature: 0x53, 0x4d, 0x41, 0x00 <=> "SMA\0"
const uint8_t  SpeedwireHeader::sma_tag0[4]      = { 0x00, 0x04, 0x02, 0xa0 };     //!< SMA tag0: 0x00, 0x04, 0x02, 0xa0 <=> length: 0x0004  tag: 0x02a0;
const uint8_t  SpeedwireHeader::sma_net_v2[2]    = { 0x00, 0x10 };                 //!< SMA net version 2 indicator: 0x00, 0x10


/**
 *  Constructor.
 *  @param udp_packet Pointer to a memory area where the speedwire packet is stored in its binary representation
 *  @param udp_packet_size Size of the speedwire packet in memory
 */
SpeedwireHeader::SpeedwireHeader(const void *const udp_packet, const unsigned long udp_packet_size) {
    udp = (uint8_t *)udp_packet;
    size = udp_packet_size;
}

/** Destructor. */
SpeedwireHeader::~SpeedwireHeader(void) {
    udp = NULL;
    size = 0;
}

/**
 *  Check the validity of this speewire packet header.
 *  A header is considered valid if it starts with an SMA signature, followed by the SMA tag0 and an SMA net version 2 indicator.
 *  @return True if the packet header belongs to a valid speedwire packet, false otherwise
 */
bool SpeedwireHeader::checkHeader(void)  const {

    // test if udp packet is large enough to hold the header structure
    if (size < (sma_protocol_offset + sma_protocol_size)) {
        return false;
    }

    // test SMA signature
    if (memcmp(sma_signature, udp + sma_signature_offset, sizeof(sma_signature)) != 0) {
        return false;
    }

    // test SMA tag0
    if (memcmp(sma_tag0, udp + sma_tag0_offset, sizeof(sma_tag0)) != 0) {
        return false;
    }

    // test group field
    //uint16_t group = getGroup();

    // test length field
    //uint16_t length = getLength();

    // test SMA net version 2
    if (memcmp(sma_net_v2, udp + sma_netversion_offset, sizeof(sma_net_v2)) != 0) {
        return false;
    }

    return true;
}

/** Get SMA signature bytes. */
uint32_t SpeedwireHeader::getSignature(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_signature_offset);
}

/** Get SMA tag0 bytes. */
uint32_t SpeedwireHeader::getTag0(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_tag0_offset);
}

/** Get group field. */
uint32_t SpeedwireHeader::getGroup(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_group_offset);
}

/** Get packet length - starting to count from the the byte following protocolID, # of long words and control byte. */
uint16_t SpeedwireHeader::getLength(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_length_offset);
}

/** Get SMA network version. */
uint16_t SpeedwireHeader::getNetworkVersion(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_netversion_offset);
}

/** Get protocol ID field. */
uint16_t SpeedwireHeader::getProtocolID(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_protocol_offset);
}

/** Get number of long words (1 long word = 4 bytes) field. */
uint8_t SpeedwireHeader::getLongWords(void) const {
    return *(udp + sma_long_words_offset);
}

/** Get control byte. */
uint8_t SpeedwireHeader::getControl(void) const {
    return *(udp + sma_control_offset);
}

/** Check if protocolID is emeter protocol id. */
bool SpeedwireHeader::isEmeterProtocolID(void) const {
    return (getProtocolID() == sma_emeter_protocol_id);
}

/** check if protocolID is inverter protocol id. */
bool SpeedwireHeader::isInverterProtocolID(void) const {
    return (getProtocolID() == sma_inverter_protocol_id);
}


/** Set header fields according to defaults. */
void SpeedwireHeader::setDefaultHeader(void) {
    setDefaultHeader(1, 0, 0);
}

/** Set header fields. */
void SpeedwireHeader::setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID) {
    memcpy(udp + sma_signature_offset, sma_signature, sizeof(sma_signature));
    memcpy(udp + sma_tag0_offset,      sma_tag0,      sizeof(sma_tag0));
    setGroup(group);
    setLength(length);
    memcpy(udp + sma_netversion_offset, sma_net_v2, sizeof(sma_net_v2));
    setProtocolID(protocolID);
    setLongWords((uint8_t)(length / sizeof(uint32_t)));
    setControl(0);
}

/** Set SMA signature bytes. */
void SpeedwireHeader::setSignature(uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_signature_offset, value);
}

/** Set SMA tag0 bytes. */
void SpeedwireHeader::setTag0(uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_tag0_offset, value);
}

/** Set group field. */
void SpeedwireHeader::setGroup(uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_group_offset, value);
}

/** Set packet length field. */
void SpeedwireHeader::setLength(uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_length_offset, value);

}

/** Set SMA network version field. */
void SpeedwireHeader::setNetworkVersion(uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_netversion_offset, value);
}

/** Set protocol ID field. */
void SpeedwireHeader::setProtocolID(uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_protocol_offset, value);
}

/** Set number of long words (1 long word = 4 bytes) field. */
 void SpeedwireHeader::setLongWords(uint8_t value) {
    SpeedwireByteEncoding::setUint8(udp + sma_long_words_offset, value);
}

/** Set control byte. */
void SpeedwireHeader::setControl(uint8_t value)  {
    SpeedwireByteEncoding::setUint8(udp + sma_control_offset, value);
}


/** Get payload offset in udp packet; i.e. the offset of the first payload byte behind the header fields. */
unsigned long SpeedwireHeader::getPayloadOffset(void) const {
    if (getProtocolID() == sma_emeter_protocol_id) {    // emeter protocol data payload starts directly after the protocolID field
        return sma_protocol_offset + sma_protocol_size;
    }
    return sma_control_offset + sma_control_size;
}

/** Get pointer to udp packet. */
uint8_t* SpeedwireHeader::getPacketPointer(void) const {
    return udp;
}

/** Get size of udp packet. */
unsigned long SpeedwireHeader::getPacketSize(void) const {
    return size;
}