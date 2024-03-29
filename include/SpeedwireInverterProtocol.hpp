#ifndef __LIBSPEEDWIRE_SPEEDWIREINVERTER_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREINVERTER_HPP__

#include <cstdint>
#include <SpeedwireCommand.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireData2Packet.hpp>
#include <SpeedwireData.hpp>

namespace libspeedwire {

    /**
     * Class for parsing and assembling of speedwire inverter packets.
     *
     * This class provides accessor methods and validity checks for a speedwire inverter packet stored in memory.
     *
     * The inverter specific part of the speedwire udp packet starts directly after the control field of the speedwire
     * packet header. The format is not publicly documented by the manufacturer. Therefore the names and meanings
     * of data fields can be plainly wrong.
     *
     * The protocol is based on register IDs that can be read from or written to. The mechanism of reading register
     * information is implemented, whereas the mechanism for writing register information is mostly unexplored.
     * For writing to registers it is recommended to rely on the well-documented Modbus API.
     * 
     *      +---------------------------------------------------------------------------------+
     *      +  Destination Address                                                            +
     *      +      2 Bytes   | Susy ID                | Destination devices susy id           +
     *      +      4 Bytes   | Serial Number          | Destination devices serial number     +
     *      +      2 Bytes   | Control                | unknown semantics                     +
     *      +---------------------------------------------------------------------------------+
     *      +  Source Address                                                                 +
     *      +      2 Bytes   | Susy ID                | Source devices susy id                +
     *      +      4 Bytes   | Serial Number          | Source devices serial number          +
     *      +      2 Bytes   | Control                | unknown semantics                     +
     *      +---------------------------------------------------------------------------------+
     *      +  Inverter Protocol                                                              +
     *      +      2 Bytes   | Error Code             | 0x0000 if no error                    +
     *      +      2 Bytes   | Fragment Counter       | Count down counter for # of packets   +
     *      +      2 Bytes   | Packet ID              | Unique packet identification          +
     *      +      4 Bytes   | Command ID             | see enum Command                      +
     *      +      4 Bytes   | First Register ID      | First Register ID                     +
     *      +      4 Bytes   | Last Register ID       | Last Register ID                      +
     *      +---------------------------------------------------------------------------------+
     *      +  Register Data                                                                  +
     *      +      n Bytes   | Register Data #0 First | see class SpeedwireRawData            +
     *      +      n Bytes   | ...                    |                                       +
     *      +      n Bytes   | Register Data #n Last  |                                       +
     *      +---------------------------------------------------------------------------------+
     */
    class SpeedwireInverterProtocol {

    protected:
        static constexpr unsigned long sma_dst_susy_id_offset = 0;                                      //!< Offset of the destination susy id; this offset is 0.
        static constexpr unsigned long sma_dst_serial_number_offset = sma_dst_susy_id_offset + 2;       //!< Offset of the destination serial number
        static constexpr unsigned long sma_dst_control_offset = sma_dst_serial_number_offset + 4;       //!< Offset of the destination control field
        static constexpr unsigned long sma_src_susy_id_offset = sma_dst_control_offset + 2;             //!< Offset of the source susy id
        static constexpr unsigned long sma_src_serial_number_offset = sma_src_susy_id_offset + 2;       //!< Offset of the source serial number
        static constexpr unsigned long sma_src_control_offset = sma_src_serial_number_offset + 4;       //!< Offset of the source control field
        static constexpr unsigned long sma_error_code_offset = sma_src_control_offset + 2;              //!< Offset of the error code field
        static constexpr unsigned long sma_fragment_counter_offset = sma_error_code_offset + 2;         //!< Offset of the fragment counter field
        static constexpr unsigned long sma_packet_id_offset = sma_fragment_counter_offset + 2;          //!< Offset of the packet id field
        static constexpr unsigned long sma_command_id_offset = sma_packet_id_offset + 2;                //!< Offset of the command id field
        static constexpr unsigned long sma_first_register_id_offset = sma_command_id_offset + 4;        //!< Offset of the first register id field
        static constexpr unsigned long sma_last_register_id_offset = sma_first_register_id_offset + 4;  //!< Offset of the last register id fiele
        static constexpr unsigned long sma_data_offset = sma_last_register_id_offset + 4;               //!< Offset of the data bytes

        uint8_t* udp;
        unsigned long size;

    public:
        //SpeedwireInverterProtocol(const void* const udp_packet, const unsigned long udp_packet_size);
        SpeedwireInverterProtocol(const SpeedwireHeader& prot);
        SpeedwireInverterProtocol(const SpeedwireData2Packet& data2_packet);
        ~SpeedwireInverterProtocol(void);

        // accessor methods
        uint16_t getDstSusyID(void) const;
        uint32_t getDstSerialNumber(void) const;
        uint16_t getDstControl(void) const;
        uint16_t getSrcSusyID(void) const;
        uint32_t getSrcSerialNumber(void) const;
        uint16_t getSrcControl(void) const;
        uint16_t getErrorCode(void) const;
        uint16_t getFragmentCounter(void) const;
        uint16_t getPacketID(void) const;
        Command  getCommandID(void) const;
        uint32_t getFirstRegisterID(void) const;
        uint32_t getLastRegisterID(void) const;
        uint32_t getDataUint32(unsigned long byte_offset) const;   // offset 0 is the first byte after last register index
        uint64_t getDataUint64(unsigned long byte_offset) const;
        void getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const;
        uint32_t getRawDataLength(void) const;
        const void* getFirstRawDataElement(void) const;
        const void* getNextRawDataElement(const void* const current, uint32_t length) const;
        SpeedwireRawData getRawData(const void* const current, uint32_t length) const;
        SpeedwireRawData getRawTimelineData(const void* const current, uint32_t length, const SpeedwireDataType& data_type) const;
        SpeedwireRawData getRawConnector0Data(const void* const current, uint32_t length, const SpeedwireDataType& data_type) const;
        std::vector<SpeedwireRawData> getRawDataElements(void) const;
        std::string toString(void) const;

        // setter methods
        void setDstSusyID(const uint16_t value);
        void setDstSerialNumber(const uint32_t value);
        void setDstControl(const uint16_t value);
        void setSrcSusyID(const uint16_t value);
        void setSrcSerialNumber(const uint32_t value);
        void setSrcControl(const uint16_t value);
        void setErrorCode(const uint16_t value);
        void setFragmentCounter(const uint16_t value);
        void setPacketID(const uint16_t value);
        void setCommandID(const Command value);
        void setFirstRegisterID(const uint32_t value);
        void setLastRegisterID(const uint32_t value);
        void setDataUint32(const unsigned long byte_offset, const uint32_t value);   // offset 0 is the first byte after last register index
        void setDataUint64(const unsigned long byte_offset, const uint64_t value);
        void setDataUint8Array(const unsigned long byte_offset, const uint8_t* const value, const unsigned long value_length);
        DEPRECATED void setTrailer(const unsigned long offset);
    };

}   // namespace libspeedwire

#endif
