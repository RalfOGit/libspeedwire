#ifndef __SPEEDWIREDISCOVERY_HPP__
#define __SPEEDWIREDISCOVERY_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>


/**
 *  Class encapsulating information about a speedwire device.
 */
class SpeedwireInfo {
public:
    uint16_t    susyID;                //!< Susy ID of the speedwire device.
    uint32_t    serialNumber;          //!< Serial number of the speedwire device.
    std::string deviceClass;           //!< Device class of the speedwire device, i.e. emeter or inverter.
    std::string deviceType;            //!< Device type of the speedwire device, i.e. emeter or inverter.
    std::string peer_ip_address;       //!< IP address of the device, either on the local subnet or somewhere else.
    std::string interface_ip_address;  //!< IP address of the local interface through which the device is reachable.

    SpeedwireInfo(void);
    std::string toString(void) const;
    bool operator==(const SpeedwireInfo& rhs) const;
    bool isPreRegistered(void) const;
    bool isFullyRegistered(void) const;
};


/**
 *  Class implementing a discovery mechanism for speedwire devices.
 *  Discovery is performed against all potential devices on all local subnets that are connected to the different
 *  network interfaces of this host and against all pre-registered devices that can be located anywhere on the internet.
 */
class SpeedwireDiscovery {

protected:
    static const unsigned char multicast_request[20];
    static const unsigned char unicast_request[58];

    const LocalHost& localhost;

    std::vector<SpeedwireInfo> speedwireDevices;

    bool sendDiscoveryPackets(size_t& broadcast_counter, size_t& prereg_counter, size_t& subnet_counter, size_t& socket_counter);
    bool recvDiscoveryPackets(const SpeedwireSocket &socket);

public:

    SpeedwireDiscovery(LocalHost& localhost);
    ~SpeedwireDiscovery(void);

    bool preRegisterDevice(const std::string peer_ip_address);

    bool registerDevice(const SpeedwireInfo &info);
    void unregisterDevice(const SpeedwireInfo& info);

    const std::vector<SpeedwireInfo>& getDevices(void) const;

    int discoverDevices(void);
};

#endif