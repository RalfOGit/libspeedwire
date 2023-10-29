#include <memory.h>
#include <LocalHost.hpp>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireTagHeader.hpp>
#include <SpeedwireData2Packet.hpp>
#include <SpeedwireHeader.hpp>

using namespace libspeedwire;

const uint8_t  SpeedwireHeader::sma_signature[4] = { 0x53, 0x4d, 0x41, 0x00 };     //!< SMA signature: 0x53, 0x4d, 0x41, 0x00 <=> "SMA\0"
const uint8_t  SpeedwireHeader::sma_tag0[4]      = { 0x00, 0x04, 0x02, 0xa0 };     //!< SMA tag0: 0x00, 0x04, 0x02, 0xa0 <=> length: 0x0004  tag: 0x02a0;
const uint8_t  SpeedwireHeader::sma_net_v2[2]    = { 0x00, 0x10 };                 //!< SMA net version 2 indicator: 0x00, 0x10

#define USE_TAG_PARSER 1    // rely on tag header parsing instead of assuming a certain fixed packet layout

/**
 *  Constructor.
 *  @param udp_packet Pointer to a memory area where the speedwire packet is stored in its binary representation
 *  @param udp_packet_size Size of the speedwire packet in memory
 */
SpeedwireHeader::SpeedwireHeader(const void *const udp_packet, const unsigned long udp_packet_size) {
    udp = (uint8_t *)udp_packet;
    size = udp_packet_size;

    // obtain a pointer to the SMA data2 tag header, there should be exactly one for emeter and inverter packets
    data2 = findTagPacket(SpeedwireTagHeader::sma_tag_data2);

    // debug prints
    //if (data2 == NULL) {
    //    LocalHost::hexdump(udp, size);
    //    void* tag = getFirstTagPacket();
    //    while (tag != NULL) {
    //        printf("% s\n", SpeedwireTagHeader::toString(tag).c_str());
    //        if (SpeedwireTagHeader::getTagId(tag) == 0) break;
    //        tag = getNextTagPacket(tag);
    //    }
    //}
}

/** Destructor. */
SpeedwireHeader::~SpeedwireHeader(void) {
    udp = NULL;
    size = 0;
    data2 = NULL;
}

/**
 *  Check if this packet starts with an SMA signature "SMA\0".
 *  @return True if the packet starts with an SMA signature "SMA\0", false otherwise
 */
bool SpeedwireHeader::isSMAPacket(void) const {
    // test SMA signature
    if (size < 4 || memcmp(sma_signature, udp + sma_signature_offset, sizeof(sma_signature)) != 0) {
        return false;
    }
    return true;
}

/**
 *  Check if this packet is a valid SMA data2 packet.
 *  A packet is considered valid if it starts with an SMA signature, followed by the SMA tag0 and an SMA data2 tag.
 *  @param fullcheck if true, also check that there is an SMA end-of-data tag behind the SMA data2 tag and that this concludes the packet.
 *  @return True if the packet header belongs to a valid SMA data2 packet, false otherwise
 */
bool SpeedwireHeader::isValidData2Packet(bool fullcheck) const {

    // test SMA signature
    if (isSMAPacket() == false) {
        return false;
    }

    // test if tag0 is the group id tag
    void* tag0_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_group_id);
    if (tag0_ptr != udp + sma_tag0_offset) {
        return false;
    }
    uint16_t tag0_size = SpeedwireTagHeader::getTagLength(tag0_ptr);
    if (tag0_size != 4) {
        return false;
    }

    // test if there is an SMA net version 2 tag, i.e. a data2 packet and there is at least space for the protocol id
    if (data2 == NULL) {
        // be nice and try to find the tag
        ((SpeedwireHeader*)this)->data2 = findTagPacket(SpeedwireTagHeader::sma_tag_data2);
        if (data2 == NULL) {
            return false;
        }
    }
    uint16_t data2_size = SpeedwireTagHeader::getTagLength(data2);
    if (data2_size < 2) {
        return false;
    }

    if (fullcheck) {
        // test if there is an end of data tag behind the SMA net version 2 payload
        void* eod_tag_ptr = getNextTagPacket(data2);
        if (eod_tag_ptr == NULL) {
            return false;
        }
        uint16_t eod_length = SpeedwireTagHeader::getTagLength(eod_tag_ptr);
        uint16_t eod_tagid = SpeedwireTagHeader::getTagId(eod_tag_ptr);
        if (eod_length != 0 || eod_tagid != 0) {
            return false;
        }

        // test if this is the end of the udp packet
        void* no_more_tag_ptr = getNextTagPacket(eod_tag_ptr);
        if (no_more_tag_ptr != NULL) {
            return false;
        }
    }
    return true;
}



/**
 *  Check the validity of this speewire packet header.
 *  A header is considered valid if it starts with an SMA signature, followed by the SMA tag0 and an SMA net version 2 indicator.
 *  @return True if the packet header belongs to a valid speedwire packet, false otherwise
 */
bool SpeedwireHeader::checkHeader(void)  const {

#if USE_TAG_PARSER
    return isValidData2Packet();
#else
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
#endif

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
#if USE_TAG_PARSER
    void* tag0_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_group_id);
    if (tag0_ptr != NULL && SpeedwireTagHeader::getTagLength(tag0_ptr) == 4) {
        return SpeedwireByteEncoding::getUint32BigEndian((uint8_t*)tag0_ptr + SpeedwireTagHeader::TAG_HEADER_LENGTH);
    }
    return -1;
#else
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_group_offset);
#endif
}

/** Get packet length - starting to count from the the byte following protocolID, # of long words and control byte. */
uint16_t SpeedwireHeader::getLength(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        return SpeedwireTagHeader::getTagLength(data2);
    }
    return 0;
#else
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_length_offset);
#endif
}

/** Get SMA network version. */
uint16_t SpeedwireHeader::getNetworkVersion(void) const {
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_netversion_offset);
}

/** Get protocol ID field. */
uint16_t SpeedwireHeader::getProtocolID(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        return SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)data2 + sma_protocol_offset);
    }
    return 0;
#else
    return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_protocol_offset);
#endif
}

/** Get number of long words (1 long word = 4 bytes) field. */
uint8_t SpeedwireHeader::getLongWords(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        return SpeedwireByteEncoding::getUint8((const uint8_t* const)data2 + sma_long_words_offset);
    }
    return 0;
#else
    return *(udp + sma_long_words_offset);
#endif
}

/** Get control byte. */
uint8_t SpeedwireHeader::getControl(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        return SpeedwireByteEncoding::getUint8((const uint8_t* const)data2 + sma_control_offset);
    }
    return 0;
#else
    return *(udp + sma_control_offset);
#endif
}

/** Check if protocolID is emeter protocol id. */
bool SpeedwireHeader::isEmeterProtocolID(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        uint16_t protocol_id = SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)data2 + sma_protocol_offset);
        return SpeedwireData2Packet::isEmeterProtocolID(protocol_id);
    }
    return false;
#else
    return isEmeterProtocolID(getProtocolID());
#endif
}

/** Check if protocolID is extended emeter protocol id. */
bool SpeedwireHeader::isExtendedEmeterProtocolID(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        uint16_t protocol_id = SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)data2 + sma_protocol_offset);
        return SpeedwireData2Packet::isExtendedEmeterProtocolID(protocol_id);
    }
    return false;
#else
    return isExtendedEmeterProtocolID(getProtocolID());
#endif
}

/** check if protocolID is inverter protocol id. */
bool SpeedwireHeader::isInverterProtocolID(void) const {
#if USE_TAG_PARSER
    if (data2 != NULL) {
        uint16_t protocol_id = SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)data2 + sma_protocol_offset);
        return SpeedwireData2Packet::isInverterProtocolID(protocol_id);
    }
    return false;
#else
    return isInverterProtocolID(getProtocolID());
#endif
}

/** Set header fields according to defaults. */
void SpeedwireHeader::setDefaultHeader(void) {
    setDefaultHeader(1, 0, 0);
}

/** Set header fields. */
void SpeedwireHeader::setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID) {
#if USE_TAG_PARSER
    // set SMA signature "SMA\0"
    memcpy(udp + sma_signature_offset, sma_signature, sizeof(sma_signature));

    // set tag0 header including the group id
    uint8_t* tag0 = udp + sma_signature_offset + sizeof(sma_signature);
    SpeedwireTagHeader::setTagLength(tag0, 4);
    SpeedwireTagHeader::setTagId(tag0, SpeedwireTagHeader::sma_tag_group_id);
    SpeedwireByteEncoding::setUint32BigEndian(tag0 + SpeedwireTagHeader::TAG_HEADER_LENGTH, group);

    // set data2 tag header including protocol id
    uint8_t *data2 = tag0 + SpeedwireTagHeader::getTotalLength(tag0);
    SpeedwireTagHeader::setTagLength(data2, length);
    SpeedwireTagHeader::setTagId(data2, SpeedwireTagHeader::sma_tag_data2);

    SpeedwireData2Packet data2_packet(*this);
    data2_packet.setProtocolID(protocolID);
    if (SpeedwireData2Packet::isExtendedEmeterProtocolID(protocolID)) {
        data2_packet.setLongWords(0);
        data2_packet.setControl(3);
    }
    else if (SpeedwireData2Packet::isInverterProtocolID(protocolID)) {
        data2_packet.setLongWords((uint8_t)(length / sizeof(uint32_t)));
        data2_packet.setControl(0);
    }

    // set end-of-data tag header
    uint8_t* eod = data2 + SpeedwireTagHeader::getTotalLength(data2);
    SpeedwireTagHeader::setTagLength(eod, 0);
    SpeedwireTagHeader::setTagId(eod, SpeedwireTagHeader::sma_tag_endofdata);

    //LocalHost::hexdump(udp, size);

    this->data2 = data2;
#else
    memcpy(udp + sma_signature_offset, sma_signature, sizeof(sma_signature));
    memcpy(udp + sma_tag0_offset,      sma_tag0,      sizeof(sma_tag0));
    setGroup(group);
    setLength(length);
    memcpy(udp + sma_netversion_offset, sma_net_v2, sizeof(sma_net_v2));
    setProtocolID(protocolID);
    if (isExtendedEmeterProtocolID(protocolID)) {
        setLongWords(0);
        setControl(3);
    } else {
        setLongWords((uint8_t)(length / sizeof(uint32_t)));
        setControl(0);
    }
    data2 = findTagPacket(SpeedwireTagHeader::sma_tag_data2);
#endif
}

/** Calculate the total length in bytes of a default sma packet with the given payload length */
unsigned long SpeedwireHeader::getDefaultHeaderTotalLength(uint32_t group, uint16_t length, uint16_t protocolID) const {
    return 
        sizeof(sma_signature) +                             // "SMA\0"
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 4 +         // tag0 packet
        SpeedwireTagHeader::TAG_HEADER_LENGTH + length +    // data2 packet
        SpeedwireTagHeader::TAG_HEADER_LENGTH;              // end-of-data packet
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
#if USE_TAG_PARSER
    if (data2 != NULL) {
        SpeedwireData2Packet data2_packet(*this);
        unsigned long offset = data2_packet.getPayloadOffset();
        unsigned long tag_offset = (unsigned long)((ptrdiff_t)((uint8_t*)data2 - udp));
        return tag_offset + offset;
    }
    return 0;
#else
    return getPayloadOffset(getProtocolID());
#endif
}

/** Get payload offset in udp packet; i.e. the offset of the first payload byte behind the header fields. */
unsigned long SpeedwireHeader::getPayloadOffset(uint16_t protocol_id) {
    // Note: for backward compatibility reasons, the tag parser cannot be used here
    if (protocol_id == SpeedwireData2Packet::sma_emeter_protocol_id) {    // emeter protocol data payload starts directly after the protocolID field
        return sma_protocol_offset + sma_protocol_size;
    }
    // for sma_inverter_protocol_id and sma_extended_emeter_protocol_id
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



/** Get pointer to first tag; this starts directly after the magic word "SMA\0", i.e. at byte offset 4. */
void* const SpeedwireHeader::getFirstTagPacket(void) const {
    uint8_t* first_tag = udp + sma_tag0_offset;
    if (tagPacketFitsIntoUdp(first_tag)) {
        return first_tag;
    }
    return NULL;
}

/** Get pointer to next tag starting from the given tag. */
void* const SpeedwireHeader::getNextTagPacket(const void* const current_tag) const {
    if (tagPacketFitsIntoUdp(current_tag)) {
        uint8_t* next_tag = (uint8_t*)current_tag + SpeedwireTagHeader::getTotalLength(current_tag);
        if (tagPacketFitsIntoUdp(next_tag)) {
            return next_tag;
        }
    }
    return NULL;
}

/** Find the given tag id in the sequence of tag headers and return a pointer to the tag */
void* const SpeedwireHeader::findTagPacket(uint16_t tag_id) const {
    void* tag = getFirstTagPacket();
    while (tag != NULL) {
        uint16_t id = SpeedwireTagHeader::getTagId(tag);
        if (id == tag_id) {
            return tag;
        }
        if (id == 0) {  // end-of-data marker
            break;
        }
        tag = getNextTagPacket(tag);
    }
    return NULL;
}

/** Check if the entire tag including its payload is contained inside the udp packet. */
bool SpeedwireHeader::tagPacketFitsIntoUdp(const void* const tag) const {
    ptrdiff_t payload_offset = (ptrdiff_t)((uint8_t*)tag + SpeedwireTagHeader::TAG_HEADER_LENGTH - udp);
    if (tag != NULL && size >= payload_offset) {
        uint8_t* next_tag = (uint8_t*)tag + SpeedwireTagHeader::getTotalLength(tag);
        ptrdiff_t next_tag_offset = (ptrdiff_t)(next_tag - udp);
        if (size >= next_tag_offset) {
            return true;
        }
    }
    return false;
}


