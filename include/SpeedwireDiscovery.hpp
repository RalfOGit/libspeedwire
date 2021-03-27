#ifndef __SPEEDWIREDISCOVERY_HPP__
#define __SPEEDWIREDISCOVERY_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>


class SpeedwireInfo {
public:
    uint16_t    susyID;
    uint32_t    serialNumber;
    std::string deviceClass;
    std::string deviceType;
    std::string peer_ip_address;
    std::string interface_ip_address;

    SpeedwireInfo(void);
    std::string toString(void) const;
    bool operator==(const SpeedwireInfo& rhs) const;
    bool isPreRegistered(void) const;
    bool isFullyRegistered(void) const;
};


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