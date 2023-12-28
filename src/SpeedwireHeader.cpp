#include <memory.h>
#include <LocalHost.hpp>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireTagHeader.hpp>
#include <SpeedwireData2Packet.hpp>
#include <SpeedwireHeader.hpp>

using namespace libspeedwire;

const uint8_t  SpeedwireHeader::sma_signature[4] = { 0x53, 0x4d, 0x41, 0x00 };     //!< SMA signature: 0x53, 0x4d, 0x41, 0x00 <=> "SMA\0"

/**
 *  Constructor.
 *  @param udp_packet Pointer to a memory area where the speedwire packet is stored in its binary representation
 *  @param udp_packet_size Size of the speedwire packet in memory
 */
SpeedwireHeader::SpeedwireHeader(const void *const udp_packet, const unsigned long udp_packet_size) {
    udp = (uint8_t *)udp_packet;
    size = udp_packet_size;

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
    const void* tag0_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_group_id);
    if (tag0_ptr != udp + sma_tag0_offset) {
        return false;
    }
    uint16_t tag0_size = SpeedwireTagHeader::getTagLength(tag0_ptr);
    if (tag0_size != 4) {
        return false;
    }

    // test if there is a data2 packet and there is at least space for the protocol id
    const void* data2_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_data2);
    if (data2_ptr == NULL) {
        return false;
    }
    uint16_t data2_size = SpeedwireTagHeader::getTagLength(data2_ptr);
    if (data2_size < 2) {
        return false;
    }

    if (fullcheck) {
        // test if the data2 packet is directly following tag0
        if (((uint8_t*)tag0_ptr + SpeedwireTagHeader::getTotalLength(tag0_ptr)) != data2_ptr) {
            return false;
        }
        
        // test if there is an end of data tag behind the data2 packet payload
        const void* eod_tag_ptr = getNextTagPacket(data2_ptr);
        if (eod_tag_ptr == NULL) {
            return false;
        }
        uint16_t eod_length = SpeedwireTagHeader::getTagLength(eod_tag_ptr);
        uint16_t eod_tagid  = SpeedwireTagHeader::getTagId(eod_tag_ptr);
        if (eod_length != 0 || eod_tagid != 0) {
            return false;
        }

        // test if the end of data tag packet is directly following the data2 packet payload
        if (((uint8_t*)data2_ptr + SpeedwireTagHeader::getTotalLength(data2_ptr)) != eod_tag_ptr) {
            return false;
        }

        // test if this is the end of the udp packet
        const void* no_more_tag_ptr = getNextTagPacket(eod_tag_ptr);
        if (no_more_tag_ptr != NULL) {
            return false;
        }
    }
    return true;
}


/**
 *  Check if this packet is a valid SMA discovery packet.
 *  A packet is considered valid if it starts with an SMA signature, followed by the SMA tag0 and at least one SMA discovery tag.
 *  @return True if the packet header belongs to a valid SMA discovery packet, false otherwise
 */
bool SpeedwireHeader::isValidDiscoveryPacket(void) const {

    // test SMA signature
    if (isSMAPacket() == false) {
        return false;
    }

    // test if tag0 is the group id tag
    const void* tag0_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_group_id);
    if (tag0_ptr != udp + sma_tag0_offset) {
        return false;
    }
    uint16_t tag0_size = SpeedwireTagHeader::getTagLength(tag0_ptr);
    if (tag0_size != 4) {
        return false;
    }

    // test if there is a discovery tag packet
    const void* discovery_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_discovery);
    if (discovery_ptr == NULL) {
        return false;
    }

    // test if there is an ip address tag packet, this is present in discovery response packets
    const void* ipaddr_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_ip_address);
    if (ipaddr_ptr != NULL) {

        // check length of ip address packet, must be >= 4 to hold at least an ipv4 address
        uint16_t ipaddr_size = SpeedwireTagHeader::getTagLength(ipaddr_ptr);
        if (ipaddr_size < 4) {
            return false;
        }
    }
    return true;
}


/** Get SMA signature bytes. */
uint32_t SpeedwireHeader::getSignature(void) const {
    return SpeedwireByteEncoding::getUint32BigEndian(udp + sma_signature_offset);
}


/** Set header fields according to defaults. */
void SpeedwireHeader::setDefaultHeader(void) {
    setDefaultHeader(1, 0, 0);
}

/** Set header fields. */
void SpeedwireHeader::setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID) {
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


/** Get pointer to udp packet. */
uint8_t* SpeedwireHeader::getPacketPointer(void) const {
    return udp;
}

/** Get size of udp packet. */
unsigned long SpeedwireHeader::getPacketSize(void) const {
    return size;
}


/** Get pointer to first tag; this starts directly after the magic word "SMA\0", i.e. at byte offset 4. */
const void* SpeedwireHeader::getFirstTagPacket(void) const {
    uint8_t* first_tag = udp + sma_tag0_offset;
    if (tagPacketFitsIntoUdp(first_tag)) {
        return first_tag;
    }
    return NULL;
}

/** Get pointer to next tag starting from the given tag. */
const void* SpeedwireHeader::getNextTagPacket(const void* const current_tag) const {
    if (tagPacketFitsIntoUdp(current_tag)) {
        uint8_t* next_tag = (uint8_t*)current_tag + SpeedwireTagHeader::getTotalLength(current_tag);
        if (tagPacketFitsIntoUdp(next_tag)) {
            return next_tag;
        }
    }
    return NULL;
}

/** Find the given tag id in the sequence of tag headers and return a pointer to the tag */
const void* SpeedwireHeader::findTagPacket(uint16_t tag_id) const {
    const void* tag = getFirstTagPacket();
    while (tag != NULL) {
        uint16_t id = SpeedwireTagHeader::getTagId(tag);
        if (id == tag_id) {
            return tag;
        }
        if (id == 0 && SpeedwireTagHeader::getTagLength(tag) == 0) {  // end-of-data marker
            break;
        }
        tag = getNextTagPacket(tag);
    }
    return NULL;
}

/** Find the the end-of-data tag header in the sequence of tag headers and return a pointer to the tag */
const void* SpeedwireHeader::findEodTagPacket(void) const {
    const void* tag = getFirstTagPacket();
    while (tag != NULL) {
        uint16_t id = SpeedwireTagHeader::getTagId(tag);
        if (id == 0 && SpeedwireTagHeader::getTagLength(tag) == 0) {  // end-of-data marker
            return tag;
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
