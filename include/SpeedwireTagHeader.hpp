#ifndef __LIBSPEEDWIRE_SPEEDWIRETAGHEADER_H__
#define __LIBSPEEDWIRE_SPEEDWIRETAGHEADER_H__

#include <cstdint>
#include <string>
#include <stdio.h>
#include <SpeedwireByteEncoding.hpp>

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
        static constexpr unsigned long sma_length_offset = 0;  //!< Offset of the length field
        static constexpr unsigned long sma_tag_offset = 2;     //!< Offset of the SMA tag
        static constexpr unsigned long sma_payload_offset = 4; //!> Offset of the payload data

    public:

        static constexpr uint16_t sma_tag_group_id   = 0x02a0;  //!< Tag id used for group id payload data (0x2a <> 42)
        static constexpr uint16_t sma_tag_data2      = 0x0010;  //!< Tag id used for SMA Data2 payload data
        static constexpr uint16_t sma_tag_discovery  = 0x0020;  //!< Tag id used for multicast discovery payload data
        static constexpr uint16_t sma_tag_ip_address = 0x0030;  //!< Tag id used for ip address payload data in discovery responses
        static constexpr uint16_t sma_tag_endofdata  = 0x0000;  //!< Tag id used for end-of-data mark

        static constexpr unsigned long TAG_HEADER_LENGTH = sma_payload_offset;

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

        /** Set length field in tag header */
        static void setTagLength(const void* current_tag, const uint16_t length) {
            SpeedwireByteEncoding::setUint16BigEndian((uint8_t*)current_tag + sma_length_offset, length);
        }

        /** Set tag id field in tag header */
        static void setTagId(const void* current_tag, const uint16_t id) {
            SpeedwireByteEncoding::setUint16BigEndian((uint8_t*)current_tag + sma_tag_offset, id);
        }

        /** Get total length of tag header and payload in bytes. */
        static unsigned long getTotalLength(const void* const current_tag) {
            return TAG_HEADER_LENGTH + getTagLength(current_tag);
        }

        static std::string toString(const void* current_tag) {
            char buff[64];
            snprintf(buff, sizeof(buff), "tag: len %d tagid %04x", getTagLength(current_tag), getTagId(current_tag));
            return buff;
        }
    };

}   // namespace libspeedwire

#endif
