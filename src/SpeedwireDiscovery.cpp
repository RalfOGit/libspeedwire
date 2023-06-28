//
// Simple program to perform SMA speedwire device discovery
// https://www.sma.de/fileadmin/content/global/Partner/Documents/sma_developer/SpeedwireDD-TI-de-10.pdf
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#define poll(a, b, c)  WSAPoll((a), (b), (c))
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <LocalHost.hpp>
#include <AddressConversion.hpp>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireCommand.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>
#include <SpeedwireDevice.hpp>
#include <SpeedwireDiscovery.hpp>
using namespace libspeedwire;


//! Multicast device discovery request packet, according to SMA documentation.
//! However, this does not seem to be supported anymore with version 3.x devices
const unsigned char  SpeedwireDiscovery::multicast_request[] = {
    0x53, 0x4d, 0x41, 0x00, 0x00, 0x04, 0x02, 0xa0,     // sma signature, tag0
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x20,     // 0xffffffff group, 0x0000 length, 0x0020 "SMA Net ?", Version ?
    0x00, 0x00, 0x00, 0x00                              // 0x0000 protocol, 0x00 #long words, 0x00 ctrl
};

//! Unicast device discovery request packet, according to SMA documentation
const unsigned char  SpeedwireDiscovery::unicast_request[] = {
    0x53, 0x4d, 0x41, 0x00, 0x00, 0x04, 0x02, 0xa0,     // sma signature, tag0
    0x00, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x10,     // 0x26 length, 0x0010 "SMA Net 2", Version 0
    0x60, 0x65, 0x09, 0xa0, 0xff, 0xff, 0xff, 0xff,     // 0x6065 protocol, 0x09 #long words, 0xa0 ctrl, 0xffff dst susyID any, 0xffffffff dst serial any
    0xff, 0xff, 0x00, 0x00, 0x7d, 0x00, 0x52, 0xbe,     // 0x0000 dst cntrl, 0x007d src susy id, 0x3a28be52 src serial
    0x28, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // 0x0000 src cntrl, 0x0000 error code, 0x0000 fragment ID
    0x01, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,     // 0x8001 packet ID, 0x0200 command ID, 0x00000000 first register id
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // 0x00000000 last register id, 0x00000000 trailer
    0x00, 0x00
};

// Request: 534d4100000402a00000000100260010 606509a0 ffffffffffff0000 7d0052be283a0000 000000000180 00020000 00000000 00000000 00000000  => command = 0x00000200, first = 0x00000000; last = 0x00000000; trailer = 0x00000000
// Response 534d4100000402a000000001004e0010 606513a0 7d0052be283a00c0 7a01842a71b30000 000000000180 01020000 00000000 00000000 00030000 00ff0000 00000000 01007a01 842a71b3 00000a00 0c000000 00000000 00000000 01010000 00000000


/**
 *  Constructor.
 *  @param host A reference to the LocalHost instance of this machine.
 */
SpeedwireDiscovery::SpeedwireDiscovery(LocalHost& host) :
    localhost(host)
{
    // assemble multicast device discovery packet
    //unsigned char mreq[20];
    //SpeedwireProtocol mcast(mreq, sizeof(mreq));
    //mcast.setDefaultHeader(0xffffffff, 0, 0x0000);
    //mcast.setNetworkVersion(0x0020);
    //if (memcmp(mreq, multicast_request, sizeof(mreq) != 0)) {
    //    perror("diff");
    //}

    // assemble unicast device discovery packet
    //unsigned char ureq[58];
    //SpeedwireProtocol ucast(ureq, sizeof(ureq));
    //ucast.setDefaultHeader(1, 0x26, SpeedwireProtocol::sma_inverter_protocol_id);
    //ucast.setControl(0xa0);
    //SpeedwireInverter uinv(ucast);
    //uinv.setDstSusyID(0xffff);
    //uinv.setDstSerialNumber(0xffffffff);
    //uinv.setDstControl(0);
    //uinv.setSrcSusyID(0x007d);
    //uinv.setSrcSerialNumber(0x3a28be42);
    //uinv.setSrcControl(0);
    //uinv.setErrorCode(0);
    //uinv.setFragmentID(0);
    //uinv.setPacketID(0x8001);
    //uinv.setCommandID(0x00000200);
    //uinv.setFirstRegisterID(0x00000000);
    //uinv.setLastRegisterID(0x00000000);
    //uinv.setDataUint32(0, 0x00000000);  // set trailer
    //if (memcmp(ureq, unicast_request, sizeof(ureq) != 0)) {
    //    perror("diff");
    //}
}


/**
 *  Destructor - clear the device list.
 */
SpeedwireDiscovery::~SpeedwireDiscovery(void) {
    speedwireDevices.clear();
}


/**
 *  Pre-register a device, i.e. just provide the ip address of the device.
 *  A pre-registered device is explicitly queried during the device discovery process. As this discovery 
 *  is based on unicast upd packets, the ip address can be on a different subnet or somewhere on the internet.
 *  @param peer_ip_address The IP address of the speedwire peer in dot (ipv4) or colon (ipv6) notation.
 */
bool SpeedwireDiscovery::preRegisterDevice(const std::string peer_ip_address) {
    SpeedwireInfo info;
    info.peer_ip_address = peer_ip_address;
    bool new_device = true;
    for (auto& device : speedwireDevices) {
        if (info.peer_ip_address == device.peer_ip_address) {
            new_device = false;
        }
    }
    if (new_device == true) {
        speedwireDevices.push_back(info);
    }
    return new_device;
}


/**
 *  Fully register a device, i.e. provide a full information data set of the device.
 */
bool SpeedwireDiscovery::registerDevice(const SpeedwireInfo& info) {
    bool new_device = true;
    bool updated_device = false;
    for (auto& device : speedwireDevices) {
        if (device.isPreRegistered() && info.peer_ip_address == device.peer_ip_address) {
            device = info;
            new_device = false;
            updated_device = true;
        }
        else if (device.isFullyRegistered() && info == device) {
            new_device = false;
        }
    }
    if (new_device) {
        speedwireDevices.push_back(info);
        updated_device = true;
    }
    return updated_device;
}


/**
 *  Unregister a device, i.e. delete it from the device list.
 */
void SpeedwireDiscovery::unregisterDevice(const SpeedwireInfo& info) {
    for (std::vector<SpeedwireInfo>::iterator it = speedwireDevices.begin(); it != speedwireDevices.end(); ) {
        if (*it == info) {
            it = speedwireDevices.erase(it);
        } else {
            it++;
        }
    }
}


/**
 *  Get a vector of all known devices.
 */
const std::vector<SpeedwireInfo>& SpeedwireDiscovery::getDevices(void) const {
    return speedwireDevices;
}


/**
 *  Try to find SMA devices on the networks connected to this host.
 */
int SpeedwireDiscovery::discoverDevices(void) {

    // get a list of all local ipv4 interface addresses
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();

    // allocate a pollfd structure for each local ip address and populate it with the socket fds
    std::vector<SpeedwireSocket> sockets = SpeedwireSocketFactory::getInstance(localhost)->getRecvSockets(SpeedwireSocketFactory::SocketType::ANYCAST, localIPs);
    std::vector<struct pollfd> fds;

    for (auto& socket : sockets) {
        struct pollfd pfd;
        pfd.fd = socket.getSocketFd();
        fds.push_back(pfd);
    }

    // configure state machine for discovery requests
    const uint64_t maxWaitTimeInMillis = 2000u;
    uint64_t startTimeInMillis = localhost.getTickCountInMs();
    size_t broadcast_counter = 0;
    size_t prereg_counter = 0;
    size_t subnet_counter = 1;
    size_t socket_counter = 0;

    while ((localhost.getTickCountInMs() - startTimeInMillis) < maxWaitTimeInMillis) {

        // prepare pollfd structure
        for (auto& pfd : fds) {
            pfd.events = POLLIN;
            pfd.revents = 0;
        }

        // send discovery request packet and update counters
        for (int i = 0; i < 10; ++i) {
            if (sendNextDiscoveryPacket(broadcast_counter, prereg_counter, subnet_counter, socket_counter) == false) {
                break;  // done with sending all discovery packets
            }
            startTimeInMillis = localhost.getTickCountInMs();
        }

        // wait for inbound packets on any of the configured sockets
        //fprintf(stdout, "poll() ...\n");
        if (poll(fds.data(), (uint32_t)fds.size(), 10) < 0) {     // timeout 10 ms
            perror("poll failed");
            break;
        }
        //fprintf(stdout, "... done\n");

        // determine the socket that received a packet
        for (int j = 0; j < fds.size(); ++j) {
            if ((fds[j].revents & POLLIN) != 0) {

                // read packet data, analyze it and create a device information record
                recvDiscoveryPackets(sockets[j]);
            }
        }
    }

    return 0;
}


/**
 *  Send discovery packets. For now this is only for ipv4 peers.
 *  State machine implementing the following sequence of packets:
 *  - multicast speedwire discovery requests to all interfaces
 *  - unicast speedwire discovery requests to pre-registered hosts
 *  - unicast speedwire discovery requests to all hosts on the network (only if the network prefix is < /16)
 */
bool SpeedwireDiscovery::sendNextDiscoveryPacket(size_t& broadcast_counter, size_t& prereg_counter, size_t& subnet_counter, size_t& socket_counter) {

    // sequentially first send multicast speedwire discovery requests
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    if (broadcast_counter < localIPs.size()) {
        const std::string& addr = localIPs[broadcast_counter];
        SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getSendSocket(SpeedwireSocketFactory::SocketType::MULTICAST, addr);
        //fprintf(stdout, "send broadcast discovery request to %s (via interface %s)\n", localhost.toString(socket.getSpeedwireMulticastIn4Address()).c_str(), socket.getLocalInterfaceAddress().c_str());
        int nbytes = socket.send(multicast_request, sizeof(multicast_request));
        broadcast_counter++;
        return true;
    }
    // followed by pre-registered ip addresses
    if (prereg_counter < localIPs.size()) {
        for (auto& device : speedwireDevices) {
            if (device.isPreRegistered()) {
                SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getSendSocket(SpeedwireSocketFactory::SocketType::UNICAST, localIPs[prereg_counter]);
                sockaddr_in sockaddr;
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_addr = AddressConversion::toInAddress(device.peer_ip_address);
                sockaddr.sin_port = htons(9522);
                //fprintf(stdout, "send unicast discovery request to %s (via interface %s)\n", device.peer_ip_address.c_str(), socket.getLocalInterfaceAddress().c_str());
                int nbytes = socket.sendto(unicast_request, sizeof(unicast_request), sockaddr);
            }
        }
        prereg_counter++;
        return true;
    }
    // followed by unicast speedwire discovery requests
    if (socket_counter < localIPs.size()) {
        // determine address range of local subnet
        const std::string& addr = localIPs[socket_counter];
        uint32_t subnet_length = 32 - localhost.getInterfacePrefixLength(addr);
        uint32_t max_subnet_counter = ((uint32_t)1 << subnet_length) - 1;
        // skip full scan if the local network prefix is < /16
        if (max_subnet_counter > 0xffff) {
            subnet_counter = max_subnet_counter;
        }
        if (subnet_counter < max_subnet_counter) {
            // assemble address of the recipient
            struct in_addr inaddr = AddressConversion::toInAddress(addr);
            uint32_t saddr = ntohl(inaddr.s_addr);      // ip address of the interface
            saddr = saddr & (~max_subnet_counter);      // mask subnet addresses, such that the subnet part is 0
            saddr = saddr + (uint32_t)subnet_counter;   // add subnet address
            sockaddr_in sockaddr;
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_addr.s_addr = htonl(saddr);
            sockaddr.sin_port = htons(9522);
            // send to socket
            SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getSendSocket(SpeedwireSocketFactory::SocketType::UNICAST, addr);
            //fprintf(stdout, "send unicast discovery request to %s (via interface %s)\n", localhost.toString(sockaddr).c_str(), socket.getLocalInterfaceAddress().c_str());
            int nbytes = socket.sendto(unicast_request, sizeof(unicast_request), sockaddr);
            ++subnet_counter;
            return true;
        }
        // proceed with the next local interface
        if (subnet_counter >= max_subnet_counter && socket_counter < localIPs.size()) {
            subnet_counter = 1;
            ++socket_counter;
            return true;
        }
    }
    return false;
}


/**
 *  Receive a discovery packet, analyze it and create a device information record.
 */
bool SpeedwireDiscovery::recvDiscoveryPackets(const SpeedwireSocket& socket) {
    bool result = false;

    std::string peer_ip_address;
    char udp_packet[1600];
    int nbytes = 0;
    if (socket.isIpv4()) {
        struct sockaddr_in addr;
        nbytes = socket.recvfrom(udp_packet, sizeof(udp_packet), addr);
        addr.sin_port = 0;
        peer_ip_address = AddressConversion::toString(addr);
    }
    else if (socket.isIpv6()) {
        struct sockaddr_in6 addr;
        nbytes = socket.recvfrom(udp_packet, sizeof(udp_packet), addr);
        addr.sin6_port = 0;
        peer_ip_address = AddressConversion::toString(addr);
    }
    if (nbytes > 0) {
        SpeedwireHeader protocol(udp_packet, nbytes);
        if (protocol.checkHeader()) {
            unsigned long payload_offset = protocol.getPayloadOffset();
            // check for speedwire multicast device discovery responses
            if (protocol.getProtocolID() == 0x0001) {
                printf("received speedwire discovery response packet\n");
            }
            // check for emeter protocol
            else if (protocol.isEmeterProtocolID() || protocol.isExtendedEmeterProtocolID()) {
                //LocalHost::hexdump(udp_packet, nbytes);
                SpeedwireEmeterProtocol emeter(protocol);
                SpeedwireInfo info;
                info.susyID = emeter.getSusyID();
                info.serialNumber = emeter.getSerialNumber();
                const SpeedwireDevice&device = SpeedwireDevice::fromSusyID(info.susyID);
                if (device.deviceClass != SpeedwireDeviceClass::UNKNOWN) {
                    info.deviceClass = toString(device.deviceClass);
                    info.deviceType = device.name;
                }
                else {
                    info.deviceClass = "Emeter";
                    info.deviceType = "Emeter";
                }
                info.peer_ip_address = peer_ip_address;
                info.interface_ip_address = localhost.getMatchingLocalIPAddress(peer_ip_address);
                if (info.interface_ip_address == "" && socket.isIpAny() == false) {
                    info.interface_ip_address = socket.getLocalInterfaceAddress();
                }
                if (registerDevice(info)) {
                    printf("%s\n", info.toString().c_str());
                    result = true;
                }
            }
            // check for inverter protocol and ignore loopback packets
            else if (protocol.isInverterProtocolID() &&
                (nbytes != sizeof(unicast_request) || memcmp(udp_packet, unicast_request, sizeof(unicast_request)) != 0)) {
                SpeedwireInverterProtocol inverter_packet(protocol);
                //LocalHost::hexdump(udp_packet, nbytes);
                //printf("%s\n", inverter_packet.toString().c_str());
                SpeedwireInfo info;
                info.susyID = inverter_packet.getSrcSusyID();
                info.serialNumber = inverter_packet.getSrcSerialNumber();
                info.deviceClass = "Inverter";
                info.deviceType = "Inverter";
                info.peer_ip_address = peer_ip_address;
                info.interface_ip_address = localhost.getMatchingLocalIPAddress(peer_ip_address);
                if (info.interface_ip_address == "" && socket.isIpAny() == false) {
                    info.interface_ip_address = socket.getLocalInterfaceAddress();
                }
#if 1
                // try to get further information about the device
                SpeedwireCommand command(localhost, speedwireDevices);
                SpeedwireInfo updatedInfo = command.queryDeviceType(info);
                if (registerDevice(updatedInfo)) {
                    printf("%s\n", updatedInfo.toString().c_str());
                    result = true;
                }
#else
                if (registerDevice(info)) {
                    printf("%s\n", info.toString().c_str());
                    result = true;
                }
#endif
#if 0
                // dump reply information; this is just the src susyid and serialnumber together with some unknown bits
                printf("%s\n", inverter_packet.toString().c_str());
                std::vector<SpeedwireRawData> raw_data = inverter_packet.getRawDataElements();
                for (auto& rd : raw_data) {
                    size_t num_values = rd.getNumberOfValues();
                    for (size_t i = 0; i < num_values; ++i) {
                        uint32_t value = rd.getValueAsUnsignedLong(i);
                        printf("0x%08lx  %ld\n", value, value);
                    }
                }
#endif
            }
            else if (!protocol.isInverterProtocolID()) {
                uint16_t id = protocol.getProtocolID();
                printf("received unknown response packet 0x%04x\n", id);
                perror("unexpected response");
            }
        }
    }
    return result;
}


/*====================================*/

/**
 *  Default constructor.
 *  Just initialize all member variables to a defined state; set susyId and serialNumber to 0
 */
SpeedwireInfo::SpeedwireInfo(void) : susyID(0), serialNumber(0), deviceClass(), deviceType(), peer_ip_address(), interface_ip_address() {}


/**
 *  Convert speedwire information to a single line string.
 */
std::string SpeedwireInfo::toString(void) const {
    char buffer[256] = { 0 };
    snprintf(buffer, sizeof(buffer), "SusyID %u  Serial %u  Class %-16s  Type %-14s  IP %s  IF %s", 
             susyID, serialNumber, deviceClass.c_str(), deviceType.c_str(), peer_ip_address.c_str(), interface_ip_address.c_str());
    return std::string(buffer);
}


/**
 *  Compare two instances; assume that if SusyID, Serial and IP is the same, it is the same device.
 */
bool SpeedwireInfo::operator==(const SpeedwireInfo& rhs) const {
    return (susyID == rhs.susyID && serialNumber == rhs.serialNumber && peer_ip_address == rhs.peer_ip_address);
}


/**
 *  Check if this instance is just pre-registered, i.e a device ip address is given.
 */
bool SpeedwireInfo::isPreRegistered(void) const {
    return (peer_ip_address.length() > 0 && susyID == 0 && serialNumber == 0);
}


/**
 *  Check if this instance is fully registered, i.e all device information is given.
 */
bool SpeedwireInfo::isFullyRegistered(void) const {
    return (susyID != 0 && serialNumber != 0 && deviceClass.length() > 0 && deviceType.length() > 0 && 
            peer_ip_address.length() > 0 && interface_ip_address.length() > 0);
}
