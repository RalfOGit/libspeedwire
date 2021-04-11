#include <memory.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireHeader.hpp>


SpeedwireHeader::SpeedwireHeader(const void *const udp_packet, const unsigned long udp_packet_len) {
    udp = (uint8_t *)udp_packet;
    size = udp_packet_len;
}

SpeedwireHeader::~SpeedwireHeader(void) {
    udp = NULL;
    size = 0;
}

// test validity of packet header
bool SpeedwireHeader::checkHeader(void)  const {

    // test if udp packet is large enough to hold the header structure
    if (size < (sma_protocol_offset + sma_protocol_size)) {
        return false;
    }

    // test SMA signature
    for (size_t i = 0; i < sizeof(sma_signature); ++i) {
        if (sma_signature[i] != udp[sma_signature_offset + i]) {
            return false;
        }
    }

    // test SMA tag0
    for (size_t i = 0; i < sizeof(sma_tag0); ++i) {
        if (sma_tag0[i] != udp[sma_tag0_offset + i]) {
            return false;
        }
    }

    // test group field
    //uint16_t group = getGroup();

    // test length field
    //uint16_t length = getLength();

    // test SMA net version 2
    for (size_t i = 0; i < sizeof(sma_net_v2); ++i) {
        if (sma_net_v2[i] != udp[sma_netversion_offset + i]) {
            return false;
        }
    }

    return true;
}

// get SMA signature
uint32_t SpeedwireHeader::getSignature(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_signature_offset);
}

// get tag0
uint32_t SpeedwireHeader::getTag0(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_tag0_offset);
}

// get group
uint32_t SpeedwireHeader::getGroup(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_group_offset);
}

// get packet length - starting to count from the the byte following protocolID, # of long words and control byte
uint16_t SpeedwireHeader::getLength(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_length_offset);
}

// get network version
uint16_t SpeedwireHeader::getNetworkVersion(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_netversion_offset);
}

// get protocol ID
uint16_t SpeedwireHeader::getProtocolID(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_protocol_offset);
}

// get number of long words (1 long word = 4 bytes)
uint8_t SpeedwireHeader::getLongWords(void) const {
    return *(udp + sma_long_words_offset);
}

// get control byte
uint8_t SpeedwireHeader::getControl(void) const {
    return *(udp + sma_control_offset);
}

// check if protocolID is emeter
bool SpeedwireHeader::isEmeterProtocolID(void) const {
    return (getProtocolID() == sma_emeter_protocol_id);
}

// check if protocolID is inverter
bool SpeedwireHeader::isInverterProtocolID(void) const {
    return (getProtocolID() == sma_inverter_protocol_id);
}


// set header fields according to defaults
void SpeedwireHeader::setDefaultHeader(void) {
    setDefaultHeader(1, 0, 0);
}
void SpeedwireHeader::setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID) {
    for (size_t i = 0; i < sizeof(sma_signature); ++i) {
        udp[sma_signature_offset + i] = sma_signature[i];
    }
    for (size_t i = 0; i < sizeof(sma_tag0); ++i) {
        udp[sma_tag0_offset + i] = sma_tag0[i];
    }
    setGroup(group);
    setLength(length);
    for (size_t i = 0; i < sizeof(sma_net_v2); ++i) {
        udp[sma_netversion_offset + i] = sma_net_v2[i];
    }
    setProtocolID(protocolID);
    setLongWords((uint8_t)(length / sizeof(uint32_t)));
    setControl(0);
}

// set SMA signature
void SpeedwireHeader::setSignature(uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_signature_offset, value);
}

// set tag0
void SpeedwireHeader::setTag0(uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_tag0_offset, value);
}

// set group
void SpeedwireHeader::setGroup(uint32_t value) {
    SpeedwireByteEncoding::setUint32BigEndian(udp + sma_group_offset, value);
}

// set packet length
void SpeedwireHeader::setLength(uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_length_offset, value);

}

// set network version
void SpeedwireHeader::setNetworkVersion(uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_netversion_offset, value);
}

// set protocol ID
void SpeedwireHeader::setProtocolID(uint16_t value) {
    SpeedwireByteEncoding::setUint16BigEndian(udp + sma_protocol_offset, value);
}

// set number of long words (1 long word = 4 bytes)
 void SpeedwireHeader::setLongWords(uint8_t value) {
    SpeedwireByteEncoding::setUint8(udp + sma_long_words_offset, value);
}

// get control byte
void SpeedwireHeader::setControl(uint8_t value)  {
    SpeedwireByteEncoding::setUint8(udp + sma_control_offset, value);
}


// get payload offset in udp packet
unsigned long SpeedwireHeader::getPayloadOffset(void) const {
    if (getProtocolID() == sma_emeter_protocol_id) {    // emeter protocol data payload starts directly after the protocolID field
        return sma_protocol_offset + sma_protocol_size;
    }
    return sma_control_offset + sma_control_size;
}

// get pointer to udp packet
uint8_t* SpeedwireHeader::getPacketPointer(void) const {
    return udp;
}

// get size of udp packet
unsigned long SpeedwireHeader::getPacketSize(void) const {
    return size;
}