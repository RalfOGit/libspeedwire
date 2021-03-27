#ifndef __SPEEDWIREPROTOCOL_H__
#define __SPEEDWIREPROTOCOL_H__

#include <cstdint>


/**
 * Class for parsing and assembling of speedwire protocol headers.
 * 
 * The header is in the first 24 bytes of a speedwire udp packet. The header format is
 * described in a public technical SMA document: "SMA Energy Meter Zählerprotokoll". The
 * english version is called "SMA Energy Meter Protocol" and can be found here:
 * https://www.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/EMETER-Protokoll-TI-en-10.pdf
 */

class SpeedwireHeader {

protected:
    static const uint8_t  sma_signature[4];
    static const uint8_t  sma_tag0[4];
    static const uint8_t  sma_net_v2[2];

    static const unsigned long sma_signature_offset;
    static const unsigned long sma_signature_size;
    static const unsigned long sma_tag0_offset;
    static const unsigned long sma_tag0_size;
    static const unsigned long sma_group_offset;
    static const unsigned long sma_group_size;
    static const unsigned long sma_length_offset;
    static const unsigned long sma_length_size;
    static const unsigned long sma_netversion_offset;
    static const unsigned long sma_netversion_size;
    static const unsigned long sma_protocol_offset;
    static const unsigned long sma_protocol_size;
    static const unsigned long sma_long_words_offset;
    static const unsigned long sma_long_words_size;
    static const unsigned long sma_control_offset;
    static const unsigned long sma_control_size;

    uint8_t *udp;
    unsigned long size;

public:

    static const uint16_t sma_emeter_protocol_id;
    static const uint16_t sma_inverter_protocol_id;
    static const uint16_t sma_discovery_protocol_id;

    SpeedwireHeader(const void *const udp_packet, const unsigned long udp_packet_size);
    ~SpeedwireHeader(void);

    bool checkHeader(void) const;

    // getter methods to retrieve header fields
    uint32_t getSignature(void) const;
    uint32_t getTag0(void) const;
    uint32_t getGroup(void) const;
    uint16_t getLength(void) const;
    uint16_t getNetworkVersion(void) const;
    uint16_t getProtocolID(void) const;
    uint8_t  getLongWords(void) const;
    uint8_t  getControl(void) const;
    bool isEmeterProtocolID(void) const;
    bool isInverterProtocolID(void) const;

    // setter methods to fill header fields
    void setDefaultHeader(void);
    void setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID);
    void setSignature(uint32_t value);
    void setTag0(uint32_t value);
    void setGroup(uint32_t value);
    void setLength(uint16_t value);
    void setNetworkVersion(uint16_t value);
    void setProtocolID(uint16_t value);
    void setLongWords(uint8_t value);
    void setControl(uint8_t value);

    unsigned long getPayloadOffset(void) const;     
    uint8_t* getPacketPointer(void) const;
    unsigned long getPacketSize(void) const;
};

#endif
