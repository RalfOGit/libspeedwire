#ifndef __SPEEDWIREINVERTER_HPP__
#define __SPEEDWIREINVERTER_HPP__

#include <cstdint>
#include <SpeedwireHeader.hpp>


class SpeedwireInverterProtocol {

protected:
    static constexpr unsigned long sma_dst_susy_id_offset       = 0;
    static constexpr unsigned long sma_dst_serial_number_offset = sma_dst_susy_id_offset + 2;
    static constexpr unsigned long sma_dst_control_offset       = sma_dst_serial_number_offset + 4;
    static constexpr unsigned long sma_src_susy_id_offset       = sma_dst_control_offset + 2;
    static constexpr unsigned long sma_src_serial_number_offset = sma_src_susy_id_offset + 2;
    static constexpr unsigned long sma_src_control_offset       = sma_src_serial_number_offset + 4;
    static constexpr unsigned long sma_error_code_offset        = sma_src_control_offset + 2;
    static constexpr unsigned long sma_fragment_id_offset       = sma_error_code_offset + 2;
    static constexpr unsigned long sma_packet_id_offset         = sma_fragment_id_offset + 2;
    static constexpr unsigned long sma_command_id_offset        = sma_packet_id_offset + 2;
    static constexpr unsigned long sma_first_register_id_offset = sma_command_id_offset + 4;
    static constexpr unsigned long sma_last_register_id_offset  = sma_first_register_id_offset + 4;
    static constexpr unsigned long sma_data_offset              = sma_last_register_id_offset + 4;

    uint8_t* udp;
    unsigned long size;
    uint16_t packet_id;

public:
    SpeedwireInverterProtocol(const void* const udp_packet, const unsigned long udp_packet_size);
    SpeedwireInverterProtocol(SpeedwireHeader &prot);
    ~SpeedwireInverterProtocol(void);

    // accessor methods
    uint16_t getDstSusyID(void) const;
    uint32_t getDstSerialNumber(void) const;
    uint16_t getDstControl(void) const;
    uint16_t getSrcSusyID(void) const;
    uint32_t getSrcSerialNumber(void) const;
    uint16_t getSrcControl(void) const;
    uint16_t getErrorCode(void) const;
    uint16_t getFragmentID(void) const;
    uint16_t getPacketID(void) const;
    uint32_t getCommandID(void) const;
    uint32_t getFirstRegisterID(void) const;
    uint32_t getLastRegisterID(void) const;
    uint32_t getDataUint32(unsigned long byte_offset) const;   // offset 0 is the first byte after last register index
    uint64_t getDataUint64(unsigned long byte_offset) const;
    void getDataUint8Array(const unsigned long byte_offset, uint8_t* buff, const size_t buff_size) const;

    // setter methods
    void setDstSusyID(const uint16_t value);
    void setDstSerialNumber(const uint32_t value);
    void setDstControl(const uint16_t value);
    void setSrcSusyID(const uint16_t value);
    void setSrcSerialNumber(const uint32_t value);
    void setSrcControl(const uint16_t value);
    void setErrorCode(const uint16_t value);
    void setFragmentID(const uint16_t value);
    void setPacketID(const uint16_t value);
    void setCommandID(const uint32_t value);
    void setFirstRegisterID(const uint32_t value);
    void setLastRegisterID(const uint32_t value);
    void setDataUint32(const unsigned long byte_offset, const uint32_t value);   // offset 0 is the first byte after last register index
    void setDataUint64(const unsigned long byte_offset, const uint64_t value);
    void setDataUint8Array(const unsigned long byte_offset, const uint8_t *const value, const unsigned long value_length);
    void setTrailer(const unsigned long offset);
};

#endif
