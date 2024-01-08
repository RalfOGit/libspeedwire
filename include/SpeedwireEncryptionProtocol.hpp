#ifndef __LIBSPEEDWIRE_SPEEDWIREENCRYPTION_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREENCRYPTION_HPP__

#include <cstdint>
#include <array>
#include <SpeedwireHeader.hpp>
#include <SpeedwireData2Packet.hpp>
#include <SpeedwireData.hpp>

namespace libspeedwire {

    /**
     * Class for parsing and assembling of speedwire encryption packets.
     *
     * This class provides accessor methods and validity checks for a speedwire encryption packet stored in memory.
     *
     * The encryption specific part of the speedwire udp packet starts directly after the speedwire packet header.
     *
     *      +----------------------------------------------------------------------------------------+
     *      +  Destination Address                                                                   +
     *      +      2 Bytes   | Susy ID                | Destination devices susy id                  +
     *      +      4 Bytes   | Serial Number          | Destination devices serial number            +
     *      +----------------------------------------------------------------------------------------+
     *      +  Source Address                                                                        +
     *      +      2 Bytes   | Susy ID                | Source devices susy id                       +
     *      +      4 Bytes   | Serial Number          | Source devices serial number                 +
     *      +----------------------------------------------------------------------------------------+
     *      +  Encryption Protocol                                                                   +
     *      +      1 Bytes   | Packet Type            | 0x0001 Request encryption keys               +
     *      +                |                        | 0x0002 Response encryption keys              +
     *      +----------------------------------------------------------------------------------------+
     *      +  Seeds                                                                                 +
     *      +     16 Bytes   | Source Seed            | Packet Type 1 and 2                          +
     *      +     16 Bytes   | Destination Seed       | Packet Type 2 only                           +
     *      +      1 Bytes   | Encryption Status      | 0x00 unsecured, 0x01 secured                 +
     *      +     16 Bytes   | Password               | Packet Type 2 only - RID or WPA-PSK Password +
     *      +     16 Bytes   | PIC                    | Packet Type 2 only - PIC                     +
     *      +     16 Bytes   | Unknown Seed 1         | Packet Type 2 only                           +
     *      +     16 Bytes   | Unknown Seed 2         | Packet Type 2 only                           +
     *      +----------------------------------------------------------------------------------------+
     *
     * The PIC, RID or WPA-PSK password is printed on the nameplate. The RID is used for high security,
     * whereas the WPA-PSK password is used for basic security.
     */
    class SpeedwireEncryptionProtocol {

    protected:
        static constexpr unsigned long sma_packet_type_offset = 0;                                      //!< Offset of the packet type; this offset is 0.
        static constexpr unsigned long sma_src_susy_id_offset = sma_packet_type_offset + 1;             //!< Offset of the source susy id
        static constexpr unsigned long sma_src_serial_number_offset = sma_src_susy_id_offset + 2;       //!< Offset of the source serial number
        static constexpr unsigned long sma_dst_susy_id_offset = sma_src_serial_number_offset + 4;       //!< Offset of the destination susy id
        static constexpr unsigned long sma_dst_serial_number_offset = sma_dst_susy_id_offset + 2;       //!< Offset of the destination serial number
        static constexpr unsigned long sma_data_offset = sma_dst_serial_number_offset + 4;              //!< Offset of the data bytes

        uint8_t* udp;
        unsigned long size;

        static std::string toHexString(uint8_t* buff, const size_t buff_size);

    public:
        //SpeedwireEncryptionProtocol(const void* const udp_packet, const unsigned long udp_packet_size);
        SpeedwireEncryptionProtocol(const SpeedwireHeader& prot);
        SpeedwireEncryptionProtocol(const SpeedwireData2Packet& data2_packet);
        ~SpeedwireEncryptionProtocol(void);

        // accessor methods
        uint8_t  getPacketType(void) const;
        uint16_t getDstSusyID(void) const;
        uint32_t getDstSerialNumber(void) const;
        uint16_t getSrcSusyID(void) const;
        uint32_t getSrcSerialNumber(void) const;
        uint8_t  getDataUint8(unsigned long byte_offset) const;   // byte offset 0 is the first byte after src serial number
        uint32_t getDataUint32(unsigned long byte_offset) const;
        uint64_t getDataUint64(unsigned long byte_offset) const;
        void getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const;
        std::array<uint8_t, 16> getDataUint8Array16(const unsigned long byte_offset) const;
        std::string getString16(const unsigned long byte_offset) const;

        // setter methods
        void setPacketType(const uint8_t value);
        void setDstSusyID(const uint16_t value);
        void setDstSerialNumber(const uint32_t value);
        void setSrcSusyID(const uint16_t value);
        void setSrcSerialNumber(const uint32_t value);
        void setDataUint8(unsigned long byte_offset, const uint8_t value);          // byte offset 0 is the first byte after src serial number
        void setDataUint32(const unsigned long byte_offset, const uint32_t value);
        void setDataUint64(const unsigned long byte_offset, const uint64_t value);
        void setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length);
        void setDataUint8Array16(const unsigned long byte_offset, const std::array<uint8_t, 16>& value);
        void setString16(const unsigned long byte_offset, const std::string& value);

        std::string toString(void) const;
    };

}   // namespace libspeedwire

#endif
