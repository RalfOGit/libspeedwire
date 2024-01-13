#include <cstring>
#include <stdio.h>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireDevice.hpp>
#include <SpeedwireTagHeader.hpp>
#include <SpeedwireData2Packet.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireDiscoveryProtocol.hpp>
using namespace libspeedwire;


//! Multicast device discovery request packet, according to SMA documentation.
const std::array<uint8_t, 20> SpeedwireDiscoveryProtocol::multicast_request = {
    0x53, 0x4d, 0x41, 0x00,                         // sma signature
    0x00, 0x04, 0x02, 0xa0, 0xff, 0xff, 0xff, 0xff, // 0x0004 length, 0x02a0 tag0, 0xffffffff group
    0x00, 0x00, 0x00, 0x20,                         // 0x0000 length, 0x0020 discovery tag
    0x00, 0x00, 0x00, 0x00                          // 0x0000 length, 0x0000 end-of-data tag
};

// response from SBS2.5
// 534d4100                        // sma signature            => mandatory
// 0004 02a0 00000001              // tag0, group 0x00000001   => mandatory
// 0002 0000 0001                  // 0x0000 tag, 0x0001       => mandatory
// 0004 0010 0001 0003             // data2 tag  protocolid=0x0001, long words=0, control=0x03
// 0004 0020 0000 0001             // discovery tag, 0x00000001 ??
// 0004 0030 c0a8b216              // ip tag, 192.168.178.22
// 0002 0070 ef0c                  // 0x0070 tag, 0xef0c
// 0001 0080 00                    // 0x0080 tag, 0x00
// 0000 0000                       // end of data tag

// response from ST5.0
// 534d4100                        // sma signature            => mandatory
// 0004 02a0 00000001              // tag0, group 0x00000001   => mandatory
// 0002 0000 0001                  // 0x0000 tag, 0x0001       => mandatory
// 0004 0010 0001 0003             // data2 tag  protocolid=0x0001, long words=0, control=0x03
// 0004 0020 0000 0001             // discovery tag, 0x00000001 ??
// 0004 0030 c0a8b216              // ip tag, 192.168.182.18
// 0004 0040 00000000              // 0x0040 tag, 0x00000000
// 0002 0070 ef0c                  // 0x0070 tag, 0xef0c
// 0001 0080 00                    // 0x0080 tag, 0x00
// 0000 0000                       // end of data tag


//! Unicast device discovery request packet, according to SMA documentation
const std::array<uint8_t, 58> SpeedwireDiscoveryProtocol::unicast_request = {
    0x53, 0x4d, 0x41, 0x00, 0x00, 0x04, 0x02, 0xa0,     // sma signature, 0x0004 length, 0x02a0 tag0
    0x00, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x10,     // 0x00000001 group, 0x0026 length, 0x0010 data2 tag
    0x60, 0x65, 0x09, 0xa0, 0xff, 0xff, 0xff, 0xff,     // 0x6065 protocol, 0x09 #long words, 0xa0 ctrl, 0xffff dst susyID any, 0xffffffff dst serial any
    0xff, 0xff, 0x00, 0x00, 0x7d, 0x00, 0x52, 0xbe,     // 0x0000 dst cntrl, 0x007d src susy id, 0x3a28be52 src serial
    0x28, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // 0x0000 src cntrl, 0x0000 error code, 0x0000 fragment ID
    0x01, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,     // 0x8001 packet ID, 0x0200 command ID, 0x00000000 first register id
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // 0x00000000 last register id, 0x00000000 end of data tag
    0x00, 0x00
};

// Request: 534d4100000402a00000000100260010 606509a0 ffffffffffff0000 7d0052be283a0000 000000000180 00020000 00000000 00000000 00000000  => command = 0x00000200, first = 0x00000000; last = 0x00000000; end of data tag = 0x00000000
// Response 534d4100000402a000000001004e0010 606513a0 7d0052be283a00c0 7a01842a71b30000 000000000180 01020000 00000000 00000000 00030000 00ff0000 00000000 01007a01 842a71b3 00000a00 0c000000 00000000 00000000 01010000 00000000

// Response from inverter: id 0x00000300 (Discovery) conn 0x00 type 0x00 (Unsigned32) time 0x0000ff00  data   0x00000000  0x017a0001  0xb3712a84  0x000a0000  0x0000000c  0x00000000  0x00000000  0x00000101
// Response from battery:  id 0x00000300 (Discovery) conn 0x00 type 0x00 (Unsigned32) time 0x0000ff00  data   0x60024170  0x015a0001  0x714f5e45  0x000a0000  0x0000000c  0x00000000  0x00000003  0x00000101


/**
 *  Constructor.
 *  @param header Reference to the SpeedwireHeader instance that encapsulate the SMA header and the pointers to the entire udp packet.
 */
SpeedwireDiscoveryProtocol::SpeedwireDiscoveryProtocol(const SpeedwireHeader& header) : 
    SpeedwireHeader(header.getPacketPointer(), header.getPacketSize()) {

    // collect pointers to relevant tag ids
    tag0_ptr      = findTagPacket(SpeedwireTagHeader::sma_tag_group_id);
    data2_ptr     = findTagPacket(SpeedwireTagHeader::sma_tag_data2);
    discovery_ptr = findTagPacket(SpeedwireTagHeader::sma_tag_discovery);
    ip_addr_ptr   = findTagPacket(SpeedwireTagHeader::sma_tag_ip_address);
}


/**
 *  Check if this packet is a valid SMA multicast discovery request packet.
 *  @return True if the packet header belongs to a valid SMA multicast discovery request packet, false otherwise
 */
bool SpeedwireDiscoveryProtocol::isMulticastRequestPacket(void) const {
    if (tag0_ptr != NULL && data2_ptr == NULL && discovery_ptr != NULL && ip_addr_ptr == NULL) {
        return true;
    }
    return false;
}


/**
 *  Check if this packet is a valid SMA multicast discovery response packet.
 *  @return True if the packet header belongs to a valid SMA multicast discovery response packet, false otherwise
 */
bool SpeedwireDiscoveryProtocol::isMulticastResponsePacket(void) const {
    if ((tag0_ptr != NULL && data2_ptr != NULL && discovery_ptr != NULL && ip_addr_ptr != NULL)) {
        return true;
    }
    return false;
}


/**
 *  Check if this packet is a valid SMA unicast discovery request packet.
 *  @return True if the packet header belongs to a valid SMA unicast discovery request packet, false otherwise
 */
bool SpeedwireDiscoveryProtocol::isUnicastRequestPacket(void) const {
#if 0
    return (getPacketSize() == unicast_request.size() && memcmp(udp, unicast_request.data(), unicast_request.size()) == 0);
#else
    if (tag0_ptr != NULL && data2_ptr != NULL && discovery_ptr == NULL && ip_addr_ptr == NULL && getPacketSize() == unicast_request.size()) {
        SpeedwireData2Packet data2(*this);

        if (data2.isInverterProtocolID()) {
            SpeedwireInverterProtocol inverter(data2);

            if (inverter.getCommandID() == 0x0200 &&
                inverter.getFirstRegisterID() == 0 &&
                inverter.getLastRegisterID()  == 0) {
                return true;
            }
        }
    }
    return false;
#endif
}


/**
 *  Check if this packet is a valid SMA unicast discovery response packet.
 *  @return True if the packet header belongs to a valid SMA unicast discovery response packet, false otherwise
 */
bool SpeedwireDiscoveryProtocol::isUnicastResponsePacket(void) const {
    if (tag0_ptr != NULL && data2_ptr != NULL && discovery_ptr == NULL && ip_addr_ptr == NULL) {
        SpeedwireData2Packet data2(*this);

        if (data2.isInverterProtocolID()) {
            SpeedwireInverterProtocol inverter(data2);

            if (inverter.getCommandID() == 0x0201 &&
                inverter.getFirstRegisterID() == 0 &&
                inverter.getLastRegisterID()  == 0 &&
                inverter.getRawDataLength()   == 40) {
                return true;
            }
        }
    }
    return false;
}


/**
 *  Check if this packet is a valid SMA discovery packet.
 *  A packet is considered valid if it starts with an SMA signature, followed by the SMA tag0 and at least one SMA discovery tag.
 *  @return True if the packet header belongs to a valid SMA discovery packet, false otherwise
 */
bool SpeedwireDiscoveryProtocol::isValidDiscoveryPacket(void) const {

    // test SMA signature "SMA\0"
    if (isSMAPacket() == false) {
        return false;
    }

    // test if the packet contains required tags for discovery request or discovery response packets
    if (isMulticastRequestPacket()  == false &&
        isMulticastResponsePacket() == false) {
        return false;
    }
    return true;
}


/**
 *  Get the ip v4 address from the discovery response packet.
 *  @return the ip v4 address in network byte order
 */
uint32_t SpeedwireDiscoveryProtocol::getIPv4Address(void) const {
    if (ip_addr_ptr != NULL && SpeedwireTagHeader::getTagLength(ip_addr_ptr) == 4u) {
        return SpeedwireByteEncoding::getUint32LittleEndian((uint8_t*)ip_addr_ptr + SpeedwireTagHeader::TAG_HEADER_LENGTH);
    }
    return 0;
}


/**
 *  Populate this speedwire packet with the content of a multicast request.
 */
void SpeedwireDiscoveryProtocol::setMulticastRequestPacket(void) {
#if 1
    memcpy(udp, multicast_request.data(), multicast_request.size());
#else
    // set SMA signature "SMA\0"
    memcpy(udp + sma_signature_offset, sma_signature, sizeof(sma_signature));

    // set tag0 header including the group id 0xffffffff
    uint8_t* tag0 = udp + sma_signature_offset + sizeof(sma_signature);
    SpeedwireTagHeader::setTagLength(tag0, 4);
    SpeedwireTagHeader::setTagId(tag0, SpeedwireTagHeader::sma_tag_group_id);
    SpeedwireByteEncoding::setUint32BigEndian(tag0 + SpeedwireTagHeader::TAG_HEADER_LENGTH, 0xffffffff);

    // set discovery tag header without payload
    uint8_t* discovery = tag0 + SpeedwireTagHeader::getTotalLength(tag0);
    SpeedwireTagHeader::setTagLength(discovery, 0);
    SpeedwireTagHeader::setTagId(discovery, SpeedwireTagHeader::sma_tag_discovery);

    // set end-of-data tag header
    uint8_t* eod = discovery + SpeedwireTagHeader::getTotalLength(discovery);
    SpeedwireTagHeader::setTagLength(eod, 0);
    SpeedwireTagHeader::setTagId(eod, SpeedwireTagHeader::sma_tag_endofdata);

    //LocalHost::hexdump(udp, size);
    return;
#endif
}


/**
 *  Populate this speedwire packet with the content of a multicast response.
 */
void SpeedwireDiscoveryProtocol::setDefaultResponsePacket(uint32_t group_id, uint32_t ip_addr) {
    // set SMA signature "SMA\0"
    memcpy(udp + sma_signature_offset, sma_signature, sizeof(sma_signature));

    // set tag0 header including the group id
    uint8_t* tag0 = udp + sma_signature_offset + sizeof(sma_signature);
    SpeedwireTagHeader::setTagLength(tag0, 4);
    SpeedwireTagHeader::setTagId(tag0, SpeedwireTagHeader::sma_tag_group_id);
    SpeedwireByteEncoding::setUint32BigEndian(tag0 + SpeedwireTagHeader::TAG_HEADER_LENGTH, group_id);

    // set 0x0000 tag with a 2 bytes payload of 0x0001
    uint8_t* null_tag = tag0 + SpeedwireTagHeader::getTotalLength(tag0);
    SpeedwireTagHeader::setTagLength(null_tag, 2);
    SpeedwireTagHeader::setTagId(null_tag, 0x0000);
    SpeedwireByteEncoding::setUint16BigEndian(null_tag + SpeedwireTagHeader::TAG_HEADER_LENGTH, 0x0001);

    // this is the end of the mandatory part

    // set data2 tag header including protocol id 0x0001, # long words 0, control 0x03
    uint8_t* data2 = null_tag + SpeedwireTagHeader::getTotalLength(null_tag);
    SpeedwireTagHeader::setTagLength(data2, 4);
    SpeedwireTagHeader::setTagId(data2, SpeedwireTagHeader::sma_tag_data2);
    SpeedwireByteEncoding::setUint16BigEndian(data2 + SpeedwireTagHeader::TAG_HEADER_LENGTH, 0x0001);
    SpeedwireByteEncoding::setUint16BigEndian(data2 + SpeedwireTagHeader::TAG_HEADER_LENGTH + 2, 0x0003);

    // set discovery tag header with a 4 bytes payload of 0x00000001
    uint8_t* discovery = data2 + SpeedwireTagHeader::getTotalLength(data2);
    SpeedwireTagHeader::setTagLength(discovery, 4);
    SpeedwireTagHeader::setTagId(discovery, SpeedwireTagHeader::sma_tag_discovery);
    SpeedwireByteEncoding::setUint32BigEndian(discovery + SpeedwireTagHeader::TAG_HEADER_LENGTH, 0x00000001);

    // set ip address tag header with a 4 bytes ip address
    uint8_t* ipaddr = discovery + SpeedwireTagHeader::getTotalLength(discovery);
    SpeedwireTagHeader::setTagLength(ipaddr, 4);
    SpeedwireTagHeader::setTagId(ipaddr, SpeedwireTagHeader::sma_tag_ip_address);
    SpeedwireByteEncoding::setUint32LittleEndian(ipaddr + SpeedwireTagHeader::TAG_HEADER_LENGTH, ip_addr);  // network byte order!

    // set 0x0070 tag header with a 2 bytes payload of 0xef0c
    uint8_t* seventy = ipaddr + SpeedwireTagHeader::getTotalLength(ipaddr);
    SpeedwireTagHeader::setTagLength(seventy, 2);
    SpeedwireTagHeader::setTagId(seventy, 0x0070);
    SpeedwireByteEncoding::setUint16BigEndian(seventy + SpeedwireTagHeader::TAG_HEADER_LENGTH, 0xef0c);

    // set 0x0080 tag header with a 1 byte payload of 0x00
    uint8_t* eighty = seventy + SpeedwireTagHeader::getTotalLength(seventy);
    SpeedwireTagHeader::setTagLength(eighty, 1);
    SpeedwireTagHeader::setTagId(eighty, 0x0080);
    SpeedwireByteEncoding::setUint8(eighty + SpeedwireTagHeader::TAG_HEADER_LENGTH, 0x00);

    // set end-of-data tag header
    uint8_t* eod = eighty + SpeedwireTagHeader::getTotalLength(eighty);
    SpeedwireTagHeader::setTagLength(eod, 0);
    SpeedwireTagHeader::setTagId(eod, SpeedwireTagHeader::sma_tag_endofdata);

    //LocalHost::hexdump(udp, size);
    return;
}


/**
 *  Calculate the total length in bytes of a default discovery response packet.
 *  @return 57 bytes
 */
unsigned long SpeedwireDiscoveryProtocol::getDefaultResponsePacketLength(void) const {
#if 0
    return
        sizeof(sma_signature) +                             // "SMA\0"
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 4 +         // tag0 packet with group id payload
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 2 +         // tag 0x0000 packet with 2 bytes payload
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 2 + 1 + 1 + // data2 packet with protocol id, # long words, control byte
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 4 +         // discovery packet with 4 bytes payload
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 4 +         // ip address packet with 4 bytes payload
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 2 +         // tag 0x0070 packet with 2 bytes payload
        SpeedwireTagHeader::TAG_HEADER_LENGTH + 1 +         // tag 0x0080 packet with 1 byte payload
        SpeedwireTagHeader::TAG_HEADER_LENGTH;              // end-of-data packet
#else
    uint8_t buffer[128]; // should be sufficient
    memset(buffer, 0, sizeof(buffer));
    SpeedwireDiscoveryProtocol packet(SpeedwireHeader(buffer, sizeof(buffer)));
    packet.setDefaultResponsePacket(0x0001, 0x00000000);
    const void* tag = packet.findEodTagPacket();
    return (unsigned long)((const uint8_t*)tag - buffer + SpeedwireTagHeader::TAG_HEADER_LENGTH);
#endif
}


/**
 *  Return a byte array representing a unicast discovery request packet.
 */
std::array<uint8_t, 58> SpeedwireDiscoveryProtocol::getUnicastRequest(void) {
    // create an inverter packet from the unicast data
    std::array<uint8_t, 58> unicast_req = unicast_request;
    SpeedwireHeader speedwire_packet(unicast_req.data(), (unsigned long)unicast_req.size());
    SpeedwireInverterProtocol inverter_packet(speedwire_packet);

    // insert local devices susy id and serial number
    static const SpeedwireAddress& local_address = SpeedwireAddress::getLocalAddress();
    inverter_packet.setSrcSusyID(local_address.susyID);
    inverter_packet.setSrcSerialNumber(local_address.serialNumber);

    // update packet id
    static uint16_t packet_id = 0x8001;
    inverter_packet.setPacketID(packet_id);
    packet_id = (packet_id + 1) | 0x8000;

    return unicast_req;
}


/**
 *  Populate this speedwire packet with the content of a unicast request.
 */
void SpeedwireDiscoveryProtocol::setUnicastRequestPacket(void) {
#if 1
    std::array<uint8_t, 58> unicast_req = getUnicastRequest();
    memcpy(udp, unicast_req.data(), unicast_req.size());
#else
    // assemble unicast device discovery packet
    unsigned char ureq[58];
    SpeedwireProtocol ucast(ureq, sizeof(ureq));
    ucast.setDefaultHeader(1, 0x26, SpeedwireProtocol::sma_inverter_protocol_id);
    ucast.setControl(0xa0);
    SpeedwireInverter uinv(ucast);
    uinv.setDstSusyID(0xffff);
    uinv.setDstSerialNumber(0xffffffff);
    uinv.setDstControl(0);
    uinv.setSrcSusyID(0x007d);
    uinv.setSrcSerialNumber(0x3a28be42);
    uinv.setSrcControl(0);
    uinv.setErrorCode(0);
    uinv.setFragmentID(0);
    uinv.setPacketID(0x8001);
    uinv.setCommandID(0x00000200);
    uinv.setFirstRegisterID(0x00000000);
    uinv.setLastRegisterID(0x00000000);
    uinv.setDataUint32(0, 0x00000000);  // set trailer
    if (memcmp(ureq, unicast_request, sizeof(ureq) != 0)) {
        perror("diff");
    }
#endif
}
