#ifndef __LIBSPEEDWIRE_SPEEDWIREPROTOCOL_H__
#define __LIBSPEEDWIRE_SPEEDWIREPROTOCOL_H__

#include <cstdint>

namespace libspeedwire {

    /**
     * Class for parsing and assembling of speedwire protocol headers.
     *
     * This class provides accessor methods and validity checks for a speedwire packet stored in memory.
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

    public:

        static constexpr uint16_t sma_emeter_protocol_id = 0x6069;          //!< Protocol id used for SMA emeter packets
        static constexpr uint16_t sma_extended_emeter_protocol_id = 0x6081; //!< Protocol id used for SMA emeter packets sent by home manager
        static constexpr uint16_t sma_inverter_protocol_id = 0x6065;        //!< Protocol id used for SMA inverter packets
        static constexpr uint16_t sma_discovery_protocol_id = 0xffff;       //!< Protocol id used for SMA discovery packets


        SpeedwireHeader(const void* const udp_packet, const unsigned long udp_packet_size);
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
        bool isExtendedEmeterProtocolID(void) const;
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

        static unsigned long getPayloadOffset(uint16_t protocol_id);
        unsigned long getPayloadOffset(void) const;
        uint8_t* getPacketPointer(void) const;
        unsigned long getPacketSize(void) const;
    };

}   // namespace libspeedwire

#endif
