#ifndef __LIBSPEEDWIRE_SPEEDWIREPROTOCOL_H__
#define __LIBSPEEDWIRE_SPEEDWIREPROTOCOL_H__

#include <cstdint>
#include <SpeedwireByteEncoding.hpp>

#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

namespace libspeedwire {

    /**
     * Class for parsing and assembling of speedwire protocol headers.
     *
     * This class provides accessor methods and validity checks for a speedwire packet stored in memory.
     * 
     * The overall speedwire packet format is:
     * - The packets start with a 4 byte SMA Signature containing the ascii encoded string "SMA\0".
     * - After the signature follows a sequence of tag packets, where each tag packet starts with a tag
     *   header followed a sequence of tag payload bytes.
     * - The last tag packet is an end-of-data packet with 0 bytes of tag payload and a tag id of 0.
     * 
     * Emeter and inverter speedwire packets follow a standard format consisting of a tag0 packet holding
     * the group id, a data2 packet holding the payload and an end-of-data packet.
     * 
     *      +---------------------------------------------------------------------------------+
     *      +      4 Bytes   | SMA Signature "SMA\0"                                          +
     *      +---------------------------------------------------------------------------------+
     *      +  Tag Packet 0                                                                   +
     *      +      2 Bytes   | Tag0 Length            | 4: # of bytes following Tag0 ID       +
     *      +      2 Bytes   | Tag0 ID                | 0x02a0                                +
     *      +      4 Bytes   | Group ID               | 0x00000001                            +
     *      +---------------------------------------------------------------------------------+
     *      +  Tag Packet 1                                                                   +
     *      +      2 Bytes   | Data2 Tag Length       | # of bytes following Data2 Tag ID     +
     *      +      2 Bytes   | Data2 Tag ID           | 0x0010                                +
     *      +      2 Bytes   | Protocol ID            | always encoded for Data2 tag packets  +
     *      +        Bytes   | Data                   |                                       +
     *      +---------------------------------------------------------------------------------+
     *      +  Tag Packet 2                                                                   +
     *      +      2 Bytes   | End-of-Data Tag Length | 0x0000: # of bytes following Tag ID   +
     *      +      2 Bytes   | End-of-Data Tag ID     | 0x0000                                +
     *      +---------------------------------------------------------------------------------+
     *
     * The header format is described in a public technical SMA document: "SMA Energy Meter Zählerprotokoll".
     * The english version is called "SMA Energy Meter Protocol" and can be found here:
     * https://developer.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/EMETER-Protokoll-TI-en-10.pdf
     */
    class SpeedwireHeader {

    protected:
        static const uint8_t  sma_signature[4];

        static constexpr unsigned long sma_signature_offset = 0;                //!< Offset of the SMA signature; this offset is 0.
        static constexpr unsigned long sma_tag0_offset = sizeof(sma_signature); //!< Offset of the SMA tag0

        uint8_t* udp;
        unsigned long size;

    public:

        SpeedwireHeader(const void* const udp_packet, const unsigned long udp_packet_size);
        ~SpeedwireHeader(void);

        bool isSMAPacket(void) const;
        bool isValidData2Packet(bool fullcheck = false) const;
        bool isValidDiscoveryPacket(void) const;

        // getter methods to retrieve header fields
        uint32_t getSignature(void) const;

        // setter methods to fill header fields
        void setDefaultHeader(void);
        void setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID);
        unsigned long getDefaultHeaderTotalLength(uint32_t group, uint16_t length, uint16_t protocolID) const;

        // setter methods to set header fields
        void setSignature(uint32_t value);

        // methods to retrieve packet pointers, offsets and payload sizes
        uint8_t* getPacketPointer(void) const;
        unsigned long getPacketSize(void) const;

        // methods to retrieve tag headers
        const void* getFirstTagPacket(void) const;
        const void* getNextTagPacket(const void* const current_tag) const;
        const void* findTagPacket(uint16_t tag_id) const;
        bool tagPacketFitsIntoUdp(const void* const tag) const;
    };

}   // namespace libspeedwire

#endif
