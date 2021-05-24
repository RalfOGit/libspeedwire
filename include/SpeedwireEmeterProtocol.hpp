#ifndef __LIBSPEEDWIRE_SPEEDWIREEMETER_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREEMETER_HPP__

#include <cstdint>
#include <stdio.h>
#include <string>
#include <SpeedwireHeader.hpp>

namespace libspeedwire {

    /**
     * Class for parsing and assembling of speedwire emeter packets.
     *
     * This class provides accessor methods and validity checks for a speedwire emeter packet stored in memory.
     *
     * The emeter specific part of the speedwire udp packet starts directly after the protocol id field of speedwire
     * packet header. The format is described in a public technical SMA document: "SMA Energy Meter Zählerprotokoll".
     * The english version is called "SMA Energy Meter Protocol" and can be found here:
     * https://www.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/EMETER-Protokoll-TI-en-10.pdf
     */
    class SpeedwireEmeterProtocol {

    protected:
        static constexpr unsigned long sma_susy_id_offset = 0;                             //!< Susy ID offset within the emeter specific part of the speedwire udp packet; i.e. this offset is 0.
        static constexpr unsigned long sma_serial_number_offset = 2;                             //!< Serial number offset within the emeter specific part of the speedwire udp packet.
        static constexpr unsigned long sma_time_offset = sma_serial_number_offset + 4;  //!< Timestamp offset within the emeter specific part of the speedwire udp packet.
        static constexpr unsigned long sma_first_obis_offset = sma_time_offset + 4;           //!< Offset of the first obis element within the emeter specific part of the speedwire udp packet.
        static constexpr uint8_t sma_firmware_version_channel = 144;                           //!< Obis channel used to mark the firmware version obis element.

        uint8_t* udp;
        unsigned long size;

    public:
        //SpeedwireEmeterProtocol(const void* const udp_packet, const unsigned long udp_packet_size);
        SpeedwireEmeterProtocol(const SpeedwireHeader& protocol);
        ~SpeedwireEmeterProtocol(void);

        // accessor methods
        uint16_t    getSusyID(void) const;
        uint32_t    getSerialNumber(void) const;
        uint32_t    getTime(void) const;
        void        setSusyID(const uint16_t susy);
        void        setSerialNumber(const uint32_t serial);
        void        setTime(const uint32_t time);
        void* const getFirstObisElement(void) const;
        void* const getNextObisElement(const void* const current_element) const;
        void* const setObisElement(void* const current_element, const void* const obis);

        // methods to get obis information with current_element pointing to the first byte of the given obis field
        static uint8_t getObisChannel(const void* const current_element);
        static uint8_t getObisIndex(const void* const current_element);
        static uint8_t getObisType(const void* const current_element);
        static uint8_t getObisTariff(const void* const current_element);
        static uint32_t getObisValue4(const void* const current_element);
        static uint64_t getObisValue8(const void* const current_element);
        static unsigned long getObisLength(const void* const current_element);

        // methods to set obis information with current_element pointing to the first byte of the given obis field
        static void setObisChannel(const void* current_element, const uint8_t channel);
        static void setObisIndex(const void* current_element, const uint8_t index);
        static void setObisType(const void* current_element, const uint8_t type);
        static void setObisTariff(const void* current_element, const uint8_t tariff);
        static void setObisValue4(const void* current_element, const uint32_t value);
        static void setObisValue8(const void* current_element, const uint64_t value);

        // print methods with current_element pointing to the first byte of the given obis field
        static std::string toHeaderString(const void* const current_element);
        static std::string toValueString(const void* const current_element, const bool hex);
        static std::string toString(const void* const current_element);
    };

}   // namespace libspeedwire

#endif
