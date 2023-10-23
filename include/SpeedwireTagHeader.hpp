#ifndef __LIBSPEEDWIRE_SPEEDWIRETAGHEADER_H__
#define __LIBSPEEDWIRE_SPEEDWIRETAGHEADER_H__

#include <cstdint>
#include <string>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireHeader.hpp>

namespace libspeedwire {

    /**
     *  Definition of an SMA tag. An SMA IP protocol packet starts with the magic "SMA\0" word followed by a sequence of tag packets.
     *  Each tag is composed of:
     *  - uint16_t length
     *  - uint16_t tagid
     *  - a sequence of 'length' bytes, where data2 packets and data2 inverter packets include additional header fields
     *  - uint16_t protocol id      => data2 packets (tagid 0x0010)
     *  - uint8_t  # long words     => data2 inverter packets
     *  - uint8_t  control          => data2 inverter packets
     */
    class SpeedwireTagHeader {
    protected:
        static constexpr unsigned long sma_length_offset = 0;                                           //!< Offset of the length field
        static constexpr unsigned long sma_length_size = 2;                                             //!< Size of the length field in bytes
        static constexpr unsigned long sma_tag_offset = sma_length_size;                                //!< Offset of the SMA tag
        static constexpr unsigned long sma_tag_size = 2;                                                //!< Size of the SMA tag in bytes
        static constexpr unsigned long sma_payload_offset = sma_tag_offset + sma_tag_size;              //!> Offset of the payload data
        static constexpr unsigned long sma_protocol_offset = sma_payload_offset;                        //!> Data2 packet only - Offset of the protocol id
        static constexpr unsigned long sma_protocol_size = 2;                                           //!> Data2 packet only - Size of the protocol id in bytes
        static constexpr unsigned long sma_long_words_offset = sma_protocol_offset + sma_protocol_size; //!< Data2 Inverter packet only - offset of the long words field, i.e. length/4
        static constexpr unsigned long sma_control_offset = sma_long_words_offset + 1;                  //!< Data2 Inverter packet only - offset of the control word field
        static constexpr unsigned long sma_control_size = 1;                                            //!< Data2 Inverter packet only - size of the control word field

    public:

        static constexpr uint16_t sma_tag_group_id        = 0x02a0;  //!< Tag id used for group id payload data
        static constexpr uint16_t sma_tag_data2           = 0x0010;  //!< Tag id used for SMA Data2 payload data
        static constexpr uint16_t sma_tag_mcast_discovery = 0x0020;  //!< Tag id used for multicast discovery payload data
        static constexpr uint16_t sma_tag_discovery       = 0x0200;  //!< Tag id used for discovery payload data

        static constexpr unsigned long TAG_DATA_OFFSET = sma_payload_offset;

        /** Constructor */
        SpeedwireTagHeader(void) {}
        ~SpeedwireTagHeader(void) {}

        /** Get length field from tag header; a length field is always present */
        static uint16_t getTagLength(const void* const current_tag) {
            return SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)current_tag + sma_length_offset);  // length count starts after tag field
        }

        /** Get tag id field from tag header; a tag id field is always present */
        static uint16_t getTagId(const void* const current_tag) {
            return SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)current_tag + sma_tag_offset);
        }

        /** Data2 packet only: Get protocol id field from tag header; a length field is always present */
        static uint16_t getProtocolID(const void* const current_tag) {
            return SpeedwireByteEncoding::getUint16BigEndian((const uint8_t* const)current_tag + sma_protocol_offset);
        }

        /** Data2 Inverter packet only: Get number of long words (1 long word = 4 bytes) field. */
        static uint8_t getLongWords(const void* const current_tag) {
            return SpeedwireByteEncoding::getUint8((const uint8_t* const)current_tag + sma_long_words_offset);

        }

        /** Data2 Inverter packet only: Get control byte. */
        static uint8_t getControl(const void* const current_tag) {
            return SpeedwireByteEncoding::getUint8((const uint8_t* const)current_tag + sma_control_offset);
        }

        /** Set length field in tag header */
        static void setTagLength(const void* current_tag, const uint16_t length) {
            SpeedwireByteEncoding::setUint16BigEndian((uint8_t*)current_tag + sma_length_offset, length);
        }

        /** Set tag id field in tag header */
        static void setTagId(const void* current_tag, const uint16_t id) {
            SpeedwireByteEncoding::setUint16BigEndian((uint8_t*)current_tag + sma_tag_offset, id);
        }

        /** Data2 Set protocol id field in tag header */
        static void setProtocolID(const void* current_tag, const uint16_t protocolid) {
            SpeedwireByteEncoding::setUint16BigEndian((uint8_t*)current_tag + sma_protocol_offset, protocolid);
        }

        /** Data2 Inverter packet only: Set long words field in tag header */
        static void setLongWords(const void* current_tag, const uint8_t longwords) {
            SpeedwireByteEncoding::setUint8((uint8_t*)current_tag + sma_long_words_offset, longwords);
        }

        /** Data2 Inverter packet only: Set control field in tag header */
        static void setControl(const void* current_tag, const uint8_t control) {
            SpeedwireByteEncoding::setUint8((uint8_t*)current_tag + sma_control_offset, control);
        }

        /** Get total length of tag header and payload in bytes. */
        static unsigned long getTotalLength(const void* const current_tag) {
            return sma_payload_offset + getTagLength(current_tag);
        }

        /** Get payload offset in tag packet; i.e. the offset of the first payload byte behind the header fields. */
        static unsigned long getPayloadOffset(const void* const current_tag) {
            uint16_t protocol_id = getProtocolID(current_tag);

            // emeter protocol data payload starts directly after the protocolID field
            switch (protocol_id) {
            case SpeedwireHeader::sma_emeter_protocol_id:
                return sma_protocol_offset + sma_protocol_size;
            case SpeedwireHeader::sma_extended_emeter_protocol_id: 
            case SpeedwireHeader::sma_inverter_protocol_id:
                return sma_control_offset + sma_control_size;
            default:
                return sma_protocol_offset;
            }
        }

        static std::string toString(const void* current_tag) {
            char buff[32];
            if (getTagId(current_tag) == sma_tag_data2) {
                uint16_t protocol = getProtocolID(current_tag);
                snprintf(buff, sizeof(buff), "tag: len %d tagid %04x protocol %04x", getTagLength(current_tag), getTagId(current_tag), protocol);
            }
            else {
                snprintf(buff, sizeof(buff), "tag: len %d tagid %04x", getTagLength(current_tag), getTagId(current_tag));
            }
            return buff;
        }
    };

}   // namespace libspeedwire

#endif
