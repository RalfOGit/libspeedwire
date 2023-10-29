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
        static const uint8_t  sma_tag0[4];
        static const uint8_t  sma_net_v2[2];

        static constexpr unsigned long sma_signature_offset = 0;                                          //!< Offset of the SMA signature; this offset is 0.
        static constexpr unsigned long sma_tag0_offset = sizeof(sma_signature);                           //!< Offset of the SMA tag0
        static constexpr unsigned long sma_group_offset = sma_tag0_offset + sizeof(sma_tag0);             //!< Offset of the group field
        static constexpr unsigned long sma_length_offset = sma_group_offset + 4;                          //!< Offset of the length field
        static constexpr unsigned long sma_netversion_offset = sma_length_offset + 2;                     //!< Offset of the SMA netv2 version field
        static constexpr unsigned long sma_protocol_offset = sma_netversion_offset + sizeof(sma_net_v2);  //!< Offset of the speedwire protocol id field
        static constexpr unsigned long sma_protocol_size = 2;                                             //!< Size of the speedwire protocol id field in bytes
        static constexpr unsigned long sma_long_words_offset = sma_protocol_offset + sma_protocol_size;   //!< Inverter packet only - offset of the long words field, i.e. length/4
        static constexpr unsigned long sma_control_offset = sma_long_words_offset + 1;                    //!< Inverter packet only - offset of the control word field
        static constexpr unsigned long sma_control_size = 1;                                              //!< Inverter packet only - size of the control word field

        uint8_t* udp;
        unsigned long size;
        void* data2;

    public:

        // Protocol ids used by SMA. These follow recommendations stated in RFC1661 for PPP traffic:
        DEPRECATED static constexpr uint16_t sma_data1_protodol_id = 0x4041;           //!< Protocol id used for SMA data1 packets
        DEPRECATED static constexpr uint16_t sma_susy_protocol_id = 0x4043;            //!< Protocol id used for SMA software update system packets
        DEPRECATED static constexpr uint16_t sma_tcpip_suppl_protocol_id = 0x4051;     //!< Protocol id used for SMA TCP/IP supplementary module packets
        /* DEPRECATED */ static constexpr uint16_t sma_emeter_protocol_id = 0x6069;          //!< Protocol id used for SMA emeter packets
        /* DEPRECATED */ static constexpr uint16_t sma_extended_emeter_protocol_id = 0x6081; //!< Protocol id used for SMA emeter packets sent by home manager
        /* DEPRECATED */ static constexpr uint16_t sma_inverter_protocol_id = 0x6065;        //!< Protocol id used for SMA inverter packets


        SpeedwireHeader(const void* const udp_packet, const unsigned long udp_packet_size);
        ~SpeedwireHeader(void);

        bool isSMAPacket(void) const;
        bool isValidData2Packet(bool fullcheck = false) const;
        DEPRECATED bool checkHeader(void) const;

        // getter methods to retrieve header fields
        uint32_t getSignature(void) const;
        DEPRECATED uint32_t getTag0(void) const;
        DEPRECATED uint32_t getGroup(void) const;
        DEPRECATED uint16_t getLength(void) const;
        DEPRECATED uint16_t getNetworkVersion(void) const;
        DEPRECATED uint16_t getProtocolID(void) const;
        DEPRECATED uint8_t  getLongWords(void) const;
        DEPRECATED uint8_t  getControl(void) const;
        DEPRECATED static bool isEmeterProtocolID        (uint16_t protocol_id) { return (protocol_id == sma_emeter_protocol_id); }
        DEPRECATED static bool isExtendedEmeterProtocolID(uint16_t protocol_id) { return (protocol_id == sma_extended_emeter_protocol_id); }
        DEPRECATED static bool isInverterProtocolID      (uint16_t protocol_id) { return (protocol_id == sma_inverter_protocol_id); }
        DEPRECATED bool isEmeterProtocolID(void) const;
        DEPRECATED bool isExtendedEmeterProtocolID(void) const;
        DEPRECATED bool isInverterProtocolID(void) const;

        // setter methods to fill header fields
        void setDefaultHeader(void);
        void setDefaultHeader(uint32_t group, uint16_t length, uint16_t protocolID);
        unsigned long getDefaultHeaderTotalLength(uint32_t group, uint16_t length, uint16_t protocolID) const;

        void setSignature(uint32_t value);

        DEPRECATED void setTag0(uint32_t value);
        DEPRECATED void setGroup(uint32_t value);
        DEPRECATED void setLength(uint16_t value);
        DEPRECATED void setNetworkVersion(uint16_t value);
        DEPRECATED void setProtocolID(uint16_t value);
        DEPRECATED void setLongWords(uint8_t value);
        DEPRECATED void setControl(uint8_t value);

        // methods to retrieve packet pointers, offsets and payload sizes
        DEPRECATED static unsigned long getPayloadOffset(uint16_t protocol_id);
        DEPRECATED unsigned long getPayloadOffset(void) const;
        uint8_t* getPacketPointer(void) const;
        unsigned long getPacketSize(void) const;

        // methods to retrieve tag headers
        void* const getFirstTagPacket(void) const;
        void* const getNextTagPacket(const void* const current_tag) const;
        void* const findTagPacket(uint16_t tag_id) const;
        bool tagPacketFitsIntoUdp(const void* const tag) const;
    };

}   // namespace libspeedwire

#endif
