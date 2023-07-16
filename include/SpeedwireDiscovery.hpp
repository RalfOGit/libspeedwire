#ifndef __LIBSPEEDWIRE_SPEEDWIREDISCOVERY_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDISCOVERY_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireDevice.hpp>

namespace libspeedwire {

    /**
     *  Class implementing a discovery mechanism for speedwire devices.
     *  Discovery is performed against all potential devices on all local subnets that are connected to the different
     *  network interfaces of this host and against all pre-registered devices that can be located anywhere on the internet.
     *  The implementation uses a state machine implementing the following sequence of packets:
     *  - multicast speedwire discovery requests to all interfaces
     *  - unicast speedwire discovery requests to pre-registered hosts
     *  - unicast speedwire discovery requests to all hosts on the network (only if the network prefix is < /16)
     */
    class SpeedwireDiscovery {

    protected:
        static const unsigned char multicast_request[20];
        static const unsigned char unicast_request[58];

        const LocalHost& localhost;

        std::vector<SpeedwireDevice> speedwireDevices;

        bool sendNextDiscoveryPacket(size_t& broadcast_counter, size_t& wakeup_counter, size_t& prereg_counter, size_t& subnet_counter, size_t& socket_counter);
        bool recvDiscoveryPackets(const SpeedwireSocket& socket);

    public:

        SpeedwireDiscovery(LocalHost& localhost);
        ~SpeedwireDiscovery(void);

        bool preRegisterDevice(const std::string peer_ip_address);
        bool requireDevice(const uint32_t serial_number);

        bool registerDevice(const SpeedwireDevice& info);
        void unregisterDevice(const SpeedwireDevice& info);

        const std::vector<SpeedwireDevice>& getDevices(void) const;

        const unsigned long getNumberOfPreRegisteredIPDevices(void) const;
        const unsigned long getNumberOfMissingDevices(void) const;
        const unsigned long getNumberOfFullyRegisteredDevices(void) const;
        const unsigned long getNumberOfDevices(void) const;

        int discoverDevices(const bool full_scan = false);
    };

}   // namespace libspeedwire

#endif