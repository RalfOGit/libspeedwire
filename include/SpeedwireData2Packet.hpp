#ifndef __LIBSPEEDWIRE_SPEEDWIREDATA2PACKET_H__
#define __LIBSPEEDWIRE_SPEEDWIREDATA2PACKET_H__

#include <cstdint>
#include <string>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireTagHeader.hpp>
#include <SpeedwireHeader.hpp>

namespace libspeedwire {

    /**
     *  Definition of an SMA data2 packet. An SMA IP protocol packet starts with the magic "SMA\0" word followed by a sequence of tag packets:
     * 
     *  - SMA\0
     *  - tag0 (group_id)
     *  - data2
     *  - end-of-data
     * 
     *  This class wraps and extends class SpeedwireTagHeader by adding support for the data2 specific header fields
     *  Each data2 tag is composed of:
     * 
     *  - uint16_t length
     *  - uint16_t tagid
     *  - a sequence of 'length' bytes, where data2 packets and data2 inverter packets include additional header fields
     *  - uint16_t protocol id      => mandatory for data2 packets
     *  - uint8_t  # long words     => data2 inverter packets only
     *  - uint8_t  control          => data2 inverter packets only
     *  - ... payload ...
     */
    class SpeedwireData2Packet {

    protected:
        uint8_t* udp;                                         //!> Pointer to first byte of this data2 packet
        unsigned long offset_from_start_of_speedwire_packet;  //!> Offset of this data2 packet in its encapsulating speedwire packet

        static constexpr unsigned long sma_protocol_offset = SpeedwireTagHeader::TAG_HEADER_LENGTH;     //!> Data2 packet only - offset of the protocol id
        static constexpr unsigned long sma_protocol_size = 2;                                           //!> Data2 packet only - size of the protocol id in bytes
        static constexpr unsigned long sma_long_words_offset = sma_protocol_offset + sma_protocol_size; //!< Data2 Inverter packet only - offset of the long words field, i.e. length/4
        static constexpr unsigned long sma_long_words_size = 1;                                         //!< Data2 Inverter packet only - size of the long words field in bytes
        static constexpr unsigned long sma_control_offset = sma_long_words_offset + sma_long_words_size;//!< Data2 Inverter packet only - offset of the control word field
        static constexpr unsigned long sma_control_size = 1;                                            //!< Data2 Inverter packet only - size of the control word field in bytes

    public:

        // Protocol ids used by SMA. These follow recommendations stated in RFC1661 for PPP traffic:
        static constexpr uint16_t sma_data1_protodol_id = 0x4041;           //!< Protocol id used for SMA data1 packets
        static constexpr uint16_t sma_susy_protocol_id = 0x4043;            //!< Protocol id used for SMA software update system packets
        static constexpr uint16_t sma_tcpip_suppl_protocol_id = 0x4051;     //!< Protocol id used for SMA TCP/IP supplementary module packets
        static constexpr uint16_t sma_emeter_protocol_id = 0x6069;          //!< Protocol id used for SMA emeter packets
        static constexpr uint16_t sma_extended_emeter_protocol_id = 0x6081; //!< Protocol id used for SMA emeter packets sent by home manager
        static constexpr uint16_t sma_inverter_protocol_id = 0x6065;        //!< Protocol id used for SMA inverter packets
        static constexpr uint16_t sma_encryption_protocol_id = 0x6075;      //!< Protocol id used for SMA encryption packets

        /**
         *  Constructor.
         *  @param header Reference to the SpeedwireHeader instance that encapsulate the SMA header and the pointers to the entire udp packet.
         */
        SpeedwireData2Packet(const SpeedwireHeader& header) {
            // obtain a pointer to the SMA data2 tag header, there should be exactly one for emeter and inverter packets
            udp = (uint8_t*)header.findTagPacket(SpeedwireTagHeader::sma_tag_data2);
            offset_from_start_of_speedwire_packet = (unsigned long)(ptrdiff_t)(udp - header.getPacketPointer());
        }

        /** Destructor. */
        ~SpeedwireData2Packet(void) {
            udp = NULL;
        }

        /** Get length field from tag header; a length field is always present */
        uint16_t getTagLength(void) const { return SpeedwireTagHeader::getTagLength(udp); }

        /** Get tag id field from tag header; a tag id field is always present */
        uint16_t getTagId(void) const { return SpeedwireTagHeader::getTagId(udp); }

        /** Data2 packet only: Get protocol id field from tag header; a protocol id field is always present */
        uint16_t getProtocolID(void) const { return SpeedwireByteEncoding::getUint16BigEndian(udp + sma_protocol_offset); }

        /** Data2 Inverter packet only: Get number of long words (1 long word = 4 bytes) field. */
        uint8_t getLongWords(void) const { return SpeedwireByteEncoding::getUint8(udp + sma_long_words_offset); }

        /** Data2 Inverter packet only: Get control byte. */
        uint8_t getControl(void) const { return SpeedwireByteEncoding::getUint8(udp + sma_control_offset); }

        /** Set length field in tag header */
        void setTagLength(const uint16_t length) { SpeedwireTagHeader::setTagLength(udp, length); }

        /** Set tag id field in tag header */
        void setTagId(const uint16_t id) { SpeedwireTagHeader::setTagId(udp, id); }

        /** Data2 Set protocol id field in tag header */
        void setProtocolID(const uint16_t protocolid) { SpeedwireByteEncoding::setUint16BigEndian(udp + sma_protocol_offset, protocolid); }

        /** Data2 Inverter packet only: Set long words field in tag header */
        void setLongWords(const uint8_t longwords) { SpeedwireByteEncoding::setUint8(udp + sma_long_words_offset, longwords); }

        /** Data2 Inverter packet only: Set control field in tag header */
        void setControl(const uint8_t control) { SpeedwireByteEncoding::setUint8(udp + sma_control_offset, control); }

        /** Get total length of tag header and payload in bytes. */
        unsigned long getTotalLength(void) const { return SpeedwireTagHeader::getTotalLength(udp); }

        /** Get offset of the first byte of this data2 packets tag header from the start of the speedwire packet */
        unsigned long getHeaderOffsetFromStartOfSpeedwirePacket(void) const { return offset_from_start_of_speedwire_packet; }

        /** Get pointer to udp packet. */
        uint8_t* getPacketPointer(void) const {
            return udp;
        }

        /** Get 'functional' payload offset from start of this data2 packet; i.e. the offset of the first payload byte behind all header fields.
            This must not be confused with the offset to the protocol_id field. Technically the payload offset starts with the protocol_id field.
            The functional payload starts behind additional data2 specific header fields, depending on the protocol id. */
        unsigned long getPayloadOffset(void) const {
            // get protocol id to determine the functional payload offset
            uint16_t protocol_id = getProtocolID();
            switch (protocol_id) {
            case sma_extended_emeter_protocol_id:
            case sma_inverter_protocol_id:
                return sma_control_offset + sma_control_size;   // inverter and extended emeter protocol data payload starts after the control byte
            case sma_encryption_protocol_id:
            case sma_emeter_protocol_id:
            default:
                return sma_protocol_offset + sma_protocol_size; // emeter and encryption protocol data payload starts directly after the protocolID field
            }
        }

        /** Get a string description of the tag header including the protocol id */
        std::string toString(void) const {
            char buff[64];
            snprintf(buff, sizeof(buff), "tag: len %d tagid %04x protocol %04x", getTagLength(), getTagId(), getProtocolID());
            return buff;
        }

        static bool isEmeterProtocolID        (uint16_t protocol_id) { return (protocol_id == sma_emeter_protocol_id); }
        static bool isExtendedEmeterProtocolID(uint16_t protocol_id) { return (protocol_id == sma_extended_emeter_protocol_id); }
        static bool isInverterProtocolID(uint16_t protocol_id) { return (protocol_id == sma_inverter_protocol_id); }
        static bool isEncryptionProtocolID(uint16_t protocol_id) { return (protocol_id == sma_encryption_protocol_id); }

        /** Check if protocolID is emeter protocol id. */
        bool isEmeterProtocolID(void) const { return isEmeterProtocolID(getProtocolID()); }

        /** Check if protocolID is extended emeter protocol id. */
        bool isExtendedEmeterProtocolID(void) const { return isExtendedEmeterProtocolID(getProtocolID()); }

        /** check if protocolID is inverter protocol id. */
        bool isInverterProtocolID(void) const { return isInverterProtocolID(getProtocolID()); }

        /** check if protocolID is encryption protocol id. */
        bool isEncryptionProtocolID(void) const { return isEncryptionProtocolID(getProtocolID()); }
    };

}   // namespace libspeedwire

#endif
