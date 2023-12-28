#ifndef __LIBSPEEDWIRE_SPEEDWIREDISCOVERYPROTOCOL_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDISCOVERYPROTOCOL_HPP__

#include <cstdint>
#include <stdio.h>
#include <string>
#include <array>
#include <SpeedwireHeader.hpp>

namespace libspeedwire {

    /**
     * Class for parsing and assembling of speedwire discovery packets.
     *
     * This class provides accessor methods and validity checks for a speedwire discovery packet stored in memory.
     *
     * The general structure of speedwire discovery request and response packets is described in the support document:
     * https://www.sma.de/fileadmin/content/global/Partner/Documents/SMA_Labs/SpeedwireDD-TI-de-10.pdf
     */
    class SpeedwireDiscoveryProtocol : protected SpeedwireHeader {

    protected:
        static const std::array<uint8_t, 20> multicast_request;
        static const std::array<uint8_t, 58> unicast_request;

        const void* tag0_ptr;
        const void* data2_ptr;
        const void* discovery_ptr;
        const void* ip_addr_ptr;

    public:
        SpeedwireDiscoveryProtocol(const SpeedwireHeader& header);

        bool isMulticastRequestPacket(void) const;
        bool isMulticastResponsePacket(void) const;
        bool isUnicastRequestPacket(void) const;

        bool isValidDiscoveryPacket(void) const;

        uint32_t getIPv4Address(void) const;

        static const std::array<uint8_t, 20>& getMulticastRequest(void) { return multicast_request; }
        void setMulticastRequestPacket(void);
        static unsigned long getMulticastRequestPacketLength(void) { return (unsigned long)multicast_request.size(); }

        void setDefaultResponsePacket(uint32_t group, uint32_t ipaddr);
        unsigned long getDefaultResponsePacketLength(void) const;

        static const std::array<uint8_t, 58>& getUnicastRequest(void) { return unicast_request; }
        void setUnicastRequestPacket(void);
    };

}   // namespace libspeedwire

#endif
