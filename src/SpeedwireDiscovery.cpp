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
#include <SpeedwireDiscoveryProtocol.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireCommand.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>
#include <SpeedwireDevice.hpp>
#include <SpeedwireDiscovery.hpp>
using namespace libspeedwire;


/**
 *  Constructor.
 *  @param host A reference to the LocalHost instance of this machine.
 */
SpeedwireDiscovery::SpeedwireDiscovery(LocalHost& host) : localhost(host) {}


/**
 *  Destructor - clear the device list.
 */
SpeedwireDiscovery::~SpeedwireDiscovery(void) {
    speedwireDevices.clear();
}


/**
 *  Pre-register a device IP address, i.e. just provide the ip address of the device.
 *  A pre-registered device is explicitly queried during the device discovery process. As this discovery 
 *  is based on unicast upd packets, the ip address can be on a different subnet or somewhere on the internet.
 *  @param peer_ip_address The IP address of the speedwire peer in dot (ipv4) or colon (ipv6) notation.
 */
bool SpeedwireDiscovery::preRegisterDevice(const std::string peer_ip_address) {
    SpeedwireDevice info;
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
 *  Pre-register a required device by its serial number.
 *  A reqistered device will receive more discovery effort, in case it is not detected.
 *  is based on unicast upd packets, the ip address can be on a different subnet or somewhere on the internet.
 *  @param serial_number the serial number of the required device.
 */
bool SpeedwireDiscovery::requireDevice(const uint32_t serial_number) {
    SpeedwireDevice info;
    info.serialNumber = serial_number;
    bool new_device = true;
    for (auto& device : speedwireDevices) {
        if (info.serialNumber == device.serialNumber) {
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
bool SpeedwireDiscovery::registerDevice(const SpeedwireDevice& info) {
    bool new_device = true;
    bool updated_device = false;
    for (auto& device : speedwireDevices) {
        if (device.hasIPAddressOnly() && info.peer_ip_address == device.peer_ip_address) {
            device = info;
            new_device = false;
            updated_device = true;
        }
        else if (device.hasSerialNumberOnly() && info.serialNumber == device.serialNumber) {
            device = info;
            new_device = false;
            updated_device = true;
        }
        else if (device.isComplete() && info == device) {
            device = info;
            new_device = false;
            //updated_device = true;
        }
    }
    if (new_device) {
        speedwireDevices.push_back(info);
        updated_device = true;
    }
    // remove duplicate device entries, this can happen if the same device was pre-registered by IP and by serial number
    if (new_device == false) {
        for (std::vector<SpeedwireDevice>::iterator it1 = speedwireDevices.begin(); it1 != speedwireDevices.end(); it1++) {
            for (std::vector<SpeedwireDevice>::iterator it2 = it1 + 1; it2 != speedwireDevices.end(); ) {
                if (*it1 == *it2) {
                    it2 = speedwireDevices.erase(it2);
                }
                else {
                    it2++;
                }
            }
        }
    }
    return updated_device;
}


/**
 *  Unregister a device, i.e. delete it from the device list.
 */
void SpeedwireDiscovery::unregisterDevice(const SpeedwireDevice& info) {
    for (std::vector<SpeedwireDevice>::iterator it = speedwireDevices.begin(); it != speedwireDevices.end(); ) {
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
const std::vector<SpeedwireDevice>& SpeedwireDiscovery::getDevices(void) const {
    return speedwireDevices;
}


/**
 *  Get the number of all pre-registered devices, where just the ip address is known.
 */
unsigned long SpeedwireDiscovery::getNumberOfPreRegisteredIPDevices(void) const {
    unsigned long count = 0;
    for (const auto& device : speedwireDevices) {
        count += (device.hasIPAddressOnly() ? 1 : 0);
    }
    return count;
}


/**
 *  Get the number of all pre-registered required devices, but they are still not yet discovered.
 */
unsigned long SpeedwireDiscovery::getNumberOfMissingDevices(void) const {
    unsigned long count = 0;
    for (const auto& device : speedwireDevices) {
        count += (device.hasSerialNumberOnly() ? 1 : 0);
    }
    return count;
}


/**
 *  Get the number of all fully registered devices.
 */
unsigned long SpeedwireDiscovery::getNumberOfFullyRegisteredDevices(void) const {
    unsigned long count = 0;
    for (const auto& device : speedwireDevices) {
        count += (device.isComplete() ? 1 : 0);
    }
    return count;
}


/**
 *  Get the number of all known devices.
 */
unsigned long SpeedwireDiscovery::getNumberOfDevices(void) const {
    return (unsigned long)speedwireDevices.size();
}


/**
 *  Try to find SMA devices on the networks connected to this host.
 *  @return the number of discovered devices
 */
int SpeedwireDiscovery::discoverDevices(const bool full_scan) {

    // get a list of all local ipv4 interface addresses
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();

    // allocate a pollfd structure for each local ip address and populate it with the socket fds
    std::vector<SpeedwireSocket> sockets = SpeedwireSocketFactory::getInstance(localhost)->getRecvSockets(SpeedwireSocketFactory::SocketType::ANYCAST, localIPs);

    // configure state machine for discovery requests
    const uint64_t maxWaitTimeInMillis = 2000u;
    size_t broadcast_counter = 0;
    size_t prereg_counter = 0;
    size_t subnet_counter = 1;
    size_t socket_counter = (full_scan ? 0 : 0xffffffff);
    size_t num_retries = 3;

    uint64_t startTimeInMillis = localhost.getTickCountInMs();
    while ((localhost.getTickCountInMs() - startTimeInMillis) < maxWaitTimeInMillis) {
        int num_sends = ((broadcast_counter == 0 || prereg_counter == 0) ? 1 : 10);
        //printf("broadcast_counter %llu prereg_counter %llu subnet_counter %llu  socket_counter %llu\n", broadcast_counter, prereg_counter, subnet_counter, socket_counter);

        // send discovery request packet and update counters
        if (num_retries > 0) {
            for (int i = 0; i < num_sends; ++i) {
                if (sendNextDiscoveryPacket(broadcast_counter, prereg_counter, subnet_counter, socket_counter) == false) {
                    --num_retries;
                    broadcast_counter = 0;  // retry multicast discovery of unknown devices
                    prereg_counter = 0;     // retry unicast discovery of pre-registered devices
                    break;  // done with sending all discovery packets
                }
                startTimeInMillis = localhost.getTickCountInMs();
            }
        }
        else {
            broadcast_counter = localIPs.size();
            prereg_counter = localIPs.size();
        }

        // wait for inbound packets on any of the configured sockets
        pollSockets(sockets, (num_sends > 1 ? 10 : 200));
    }

    // try to get further information about the devices by querying device type information from the peers
    completeDeviceInformation();

    // return the number of discovered and fully registered devices
    return getNumberOfFullyRegisteredDevices();
}


int SpeedwireDiscovery::pollSockets(const std::vector<SpeedwireSocket>& sockets, int timeout) {
    // prepare pollfd structure
    std::vector<struct pollfd> fds;
    for (auto& socket : sockets) {
        struct pollfd pfd;
        pfd.fd = socket.getSocketFd();
        fds.push_back(pfd);
    }

    int result = 0;
    while (true) {

        for (auto& pfd : fds) {
            pfd.events = POLLIN;
            pfd.revents = 0;
        }

        // wait for inbound packets on any of the configured sockets
        //fprintf(stdout, "poll() ...\n");
        int result = poll(fds.data(), (uint32_t)fds.size(), timeout);
        if (result < 0) {   // error
            perror("poll failed");
            return -1;
        }
        else if (result == 0) {  // timeout
            //fprintf(stdout, "... timeout\n");
            break;
        }
        else {
            //fprintf(stdout, "... done\n");

            // determine the socket that received a packet
            // read packet data, analyze it and create a device information record
            for (int j = 0; j < fds.size(); ++j) {
                if ((fds[j].revents & POLLIN) != 0) {
                    recvDiscoveryPackets(sockets[j]);
                    ++result;
                }
            }
        }
    }
    return result;
}


/**
 *  Send discovery packets. For now this is only for ipv4 peers.
 *  State machine implementing the following sequence of packets:
 *  - multicast speedwire discovery requests to all interfaces
 *  - unicast speedwire discovery requests to pre-registered devices
 *  - unicast speedwire discovery requests to all hosts on the network (only if the network prefix is < /16)
 */
bool SpeedwireDiscovery::sendNextDiscoveryPacket(size_t& broadcast_counter, size_t& prereg_counter, size_t& subnet_counter, size_t& socket_counter) {

    // sequentially first send multicast speedwire discovery requests
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    if (broadcast_counter < localIPs.size()) {
        broadcast_counter = localIPs.size();
        sendMulticastDiscoveryRequestToSockets();
        return true;
    }
    // followed by pre-registered ip addresses
    if (prereg_counter < localIPs.size()) {
        prereg_counter = localIPs.size();
        sendUnicastDiscoveryRequestToDevices();
        return true;
    }
    // followed by a full scan based on unicast speedwire discovery requests
    if (socket_counter < localIPs.size()) {
        return sendUnicastDiscoveryRequestToSockets(subnet_counter, socket_counter);
    }
    return false;
}


/**
 *  Send multicast discovery packets to each local interface.
 */
bool SpeedwireDiscovery::sendMulticastDiscoveryRequestToSockets(void) {
    const std::array<uint8_t, 20>& multicast_request = SpeedwireDiscoveryProtocol::getMulticastRequest();
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    for (const auto& if_addr : localIPs) {
        SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getSendSocket(SpeedwireSocketFactory::SocketType::MULTICAST, if_addr);
        //fprintf(stdout, "send broadcast discovery request to %s (via interface %s)\n", AddressConversion::toString(socket.getSpeedwireMulticastIn4Address()).c_str(), socket.getLocalInterfaceAddress().c_str());
        int nbytes = socket.sendto(multicast_request.data(), (unsigned long)multicast_request.size(), socket.getSpeedwireMulticastIn4Address(), AddressConversion::toInAddress(if_addr));
    }
    return true;
}


/**
 *  Send multicast discovery packets to each pre-registered device.
 */
bool SpeedwireDiscovery::sendMulticastDiscoveryRequestToDevices(void) {
    const std::array<uint8_t, 20>& multicast_request = SpeedwireDiscoveryProtocol::getMulticastRequest();
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    for (auto& device : speedwireDevices) {
        if (device.hasIPAddressOnly()) {
            struct in_addr dev_addr = AddressConversion::toInAddress(device.peer_ip_address);
            for (const auto& local_if_addr : localIPs) {
                struct in_addr if_addr = AddressConversion::toInAddress(local_if_addr);
                if (AddressConversion::resideOnSameSubnet(if_addr, dev_addr, 24)) {
                    SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getSendSocket(SpeedwireSocketFactory::SocketType::UNICAST, local_if_addr);
                    sockaddr_in sockaddr;
                    sockaddr.sin_family = AF_INET;
                    sockaddr.sin_addr = dev_addr;
                    sockaddr.sin_port = htons(9522);
                    //fprintf(stdout, "send multicast discovery request to %s (via interface %s)\n", device.peer_ip_address.c_str(), socket.getLocalInterfaceAddress().c_str());
                    int nbytes = socket.sendto(multicast_request.data(), (unsigned long)multicast_request.size(), sockaddr);
                }
            }
        }
    }
    return true;
}


/**
 *  Send unicast discovery packets to each pre-registered device.
 */
bool SpeedwireDiscovery::sendUnicastDiscoveryRequestToDevices(void) {
    const std::array<uint8_t, 58>& unicast_request = SpeedwireDiscoveryProtocol::getUnicastRequest();
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    for (auto& device : speedwireDevices) {
        if (device.hasIPAddressOnly()) {
            struct in_addr dev_addr = AddressConversion::toInAddress(device.peer_ip_address);
            for (const auto& local_if_addr : localIPs) {
                SpeedwireSocket socket = SpeedwireSocketFactory::getInstance(localhost)->getSendSocket(SpeedwireSocketFactory::SocketType::UNICAST, local_if_addr);
                sockaddr_in sockaddr;
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_addr = dev_addr;
                sockaddr.sin_port = htons(9522);
                //fprintf(stdout, "send unicast discovery request to %s (via interface %s)\n", device.peer_ip_address.c_str(), socket.getLocalInterfaceAddress().c_str());
                int nbytes = socket.sendto(unicast_request.data(), (unsigned long)unicast_request.size(), sockaddr);
            }
        }
    }
    return true;
}


/**
 *  Send unicast discovery packets to each ip address in the subnet of each socket. This is used for full subnet scans.
 */
bool SpeedwireDiscovery::sendUnicastDiscoveryRequestToSockets(size_t& subnet_counter, size_t& socket_counter) {
    // determine address range of local subnet
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    const std::string& addr = localIPs[socket_counter];
    uint32_t subnet_length = 32 - localhost.getInterfacePrefixLength(addr);
    uint32_t max_subnet_counter = ((uint32_t)1 << subnet_length) - 1;
    // skip full scan if the local network prefix is < /24
    if (max_subnet_counter > 0xff) {
        fprintf(stdout, "skipping full scan for interface %s for ip addresses 1 ... %lu\n", addr.c_str(), max_subnet_counter);
        subnet_counter = max_subnet_counter;
    }
    else if (subnet_counter == 1) {
        fprintf(stdout, "starting full scan for interface %s for ip addresses 1 ... %lu\n", addr.c_str(), max_subnet_counter);
    }
    if (subnet_counter < max_subnet_counter && socket_counter < localIPs.size()) {
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
        //fprintf(stdout, "send unicast discovery request to %s (via interface %s)\n", AddressConversion::toString(sockaddr).c_str(), socket.getLocalInterfaceAddress().c_str());
        const std::array<uint8_t, 58>& unicast_request = SpeedwireDiscoveryProtocol::getUnicastRequest();
        int nbytes = socket.sendto(unicast_request.data(), (unsigned long)unicast_request.size(), sockaddr);
        ++subnet_counter;
        return true;
    }
    // proceed with the next local interface
    if (subnet_counter >= max_subnet_counter && socket_counter < localIPs.size()) {
        fprintf(stdout, "completed full scan for interface %s\n", addr.c_str());
        subnet_counter = 1;
        ++socket_counter;
        return true;
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

        // check for speedwire device discovery responses
        if (protocol.isValidDiscoveryPacket()) {

            // find ip address tag packet
            SpeedwireDiscoveryProtocol discovery_packet(protocol);
            struct in_addr in;
            in.s_addr = discovery_packet.getIPv4Address();
            if (in.s_addr != 0) {
                std::string ip = AddressConversion::toString(in);
                printf("received speedwire discovery response packet from %s - ipaddr tag %s\n", peer_ip_address.c_str(), ip.c_str());
                preRegisterDevice(ip);
            }
        }
        else if (protocol.isValidData2Packet()) {

            SpeedwireData2Packet data2_packet(protocol);
            uint16_t length = data2_packet.getTagLength();
            uint16_t protocolID = data2_packet.getProtocolID();

            // check for emeter protocol
            if (SpeedwireData2Packet::isEmeterProtocolID(protocolID) || SpeedwireData2Packet::isExtendedEmeterProtocolID(protocolID)) {
                //LocalHost::hexdump(udp_packet, nbytes);
                SpeedwireEmeterProtocol emeter(protocol);
                SpeedwireDevice device;
                device.susyID = emeter.getSusyID();
                device.serialNumber = emeter.getSerialNumber();
                const SpeedwireDeviceType &device_type = SpeedwireDeviceType::fromSusyID(device.susyID);
                if (device_type.deviceClass != SpeedwireDeviceClass::UNKNOWN) {
                    device.deviceClass = toString(device_type.deviceClass);
                    device.deviceModel = device_type.name;
                }
                else {
                    device.deviceClass = "Emeter";
                    device.deviceModel = "Emeter";
                }
                device.peer_ip_address = peer_ip_address;
                device.interface_ip_address = localhost.getMatchingLocalIPAddress(peer_ip_address);
                if (device.interface_ip_address == "" && socket.isIpAny() == false) {
                    device.interface_ip_address = socket.getLocalInterfaceAddress();
                }
                if (registerDevice(device)) {
                    printf("found susyid %u serial %lu ip %s\n", device.susyID, device.serialNumber, device.peer_ip_address.c_str());
                    result = true;
                }
            }
            // check for inverter protocol and ignore loopback packets
            else if (SpeedwireData2Packet::isInverterProtocolID(protocolID) &&
                SpeedwireDiscoveryProtocol(protocol).isUnicastRequestPacket() == false) {
                SpeedwireInverterProtocol inverter_packet(protocol);
                //LocalHost::hexdump(udp_packet, nbytes);
                //printf("%s\n", inverter_packet.toString().c_str());
                SpeedwireDevice device;
                device.susyID = inverter_packet.getSrcSusyID();
                device.serialNumber = inverter_packet.getSrcSerialNumber();
                device.deviceClass = "Inverter";
                device.deviceModel = "Inverter";
                device.peer_ip_address = peer_ip_address;
                device.interface_ip_address = localhost.getMatchingLocalIPAddress(peer_ip_address);
                if (device.interface_ip_address.length() == 0 && socket.isIpAny() == false) {
                    device.interface_ip_address = socket.getLocalInterfaceAddress();
                }
                // try to get further information about the device by examining the susy id; this is not accurate
                const SpeedwireDeviceType& device_type = SpeedwireDeviceType::fromSusyID(device.susyID);
                if (device_type.deviceClass != SpeedwireDeviceClass::UNKNOWN) {
                    device.deviceClass = toString(device_type.deviceClass);
                    device.deviceModel = device_type.name;
                }
                if (registerDevice(device)) {
                    printf("found susyid %u serial %lu ip %s\n", device.susyID, device.serialNumber, device.peer_ip_address.c_str());
                    result = true;
                }
#if 0
                // dump reply information; this is just the src susyid and serialnumber together with some unknown bits
                printf("%s\n", inverter_packet.toString().c_str());
                std::vector<SpeedwireRawData> raw_data = inverter_packet.getRawDataElements();
                for (auto& rd : raw_data) {
                    printf("%s\n", rd.toString().c_str());
                }
#endif
            }
            else if (!SpeedwireData2Packet::isInverterProtocolID(protocolID)) {
                printf("received unknown response packet 0x%04x\n", protocolID);
                perror("unexpected response");
            }
        }
    }
    return result;
}


/**
 *  Receive a discovery packet, analyze it and create a device information record.
 */
bool SpeedwireDiscovery::completeDeviceInformation(void) {
    const uint32_t max_retries = 1;
    uint32_t num_retries = 0;

    while (num_retries < max_retries) {
        // try to get further information about the device by querying device type information from the peer
        for (auto& device : speedwireDevices) {
            if (device.interface_ip_address.length() == 0 || device.interface_ip_address == "0.0.0.0") {
                device.interface_ip_address = localhost.getMatchingLocalIPAddress(device.peer_ip_address);
            }
            // if the ip address and interface address is known, just query the device
            if (device.isComplete() == false && device.peer_ip_address.length() != 0 && device.interface_ip_address.length() != 0) {
                SpeedwireCommand command(localhost, speedwireDevices);
                SpeedwireDevice updatedDevice = command.queryDeviceType(device);
                if (updatedDevice.isComplete() == true) {
                    registerDevice(updatedDevice);
                    printf("%s\n", updatedDevice.toString().c_str());
                }
            }
        }
        ++num_retries;
    }
    for (auto& device : speedwireDevices) {
        printf("%s\n", device.toString().c_str());
    }
    return true;
}

