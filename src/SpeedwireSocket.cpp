#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <cstring>
#include <stdio.h>
#include <vector>
#include <SpeedwireSocket.hpp>
#include <AddressConversion.hpp>
using namespace libspeedwire;


/**
 *  Class implementing a platform neutral socket abstraction for speedwire multicast traffic.
 */

// define static constants to help compare ip addresses against the wildcard addresses
static const in_addr  IN4_ADDRESS_ANY = { 0 };    // all 0's
static const in6_addr IN6_ADDRESS_ANY = { 0 };    // all 0's


/**
 *  Constructor
 */
SpeedwireSocket::SpeedwireSocket(const LocalHost &_localhost) :
    localhost(_localhost),
    socket_interface() {
    socket_fd = -1;
    socket_fd_ref_counter = (int*) malloc(sizeof(int));
    if (socket_fd_ref_counter != NULL) *socket_fd_ref_counter = 1;
    socket_family = AF_UNSPEC;
    socket_interface_v4.s_addr = INADDR_ANY;
    memcpy(&socket_interface_v6, &IN6_ADDRESS_ANY, sizeof(socket_interface_v6));
    isInterfaceAny = false;

    // configure ipv4 and ipv6 addresses for mDNS multicast traffic
    memset(&speedwire_multicast_address_v4, 0, sizeof(speedwire_multicast_address_v4));
    speedwire_multicast_address_v4.sin_family = AF_INET;
    speedwire_multicast_address_v4.sin_addr = AddressConversion::toInAddress("239.12.255.254");
    speedwire_multicast_address_v4.sin_port = htons(9522);

    memset(&speedwire_multicast_address_v6, 0, sizeof(speedwire_multicast_address_v6));
    speedwire_multicast_address_v6.sin6_family = AF_INET6;
    speedwire_multicast_address_v6.sin6_addr = AddressConversion::toIn6Address("::");     // FIXME: tbd
    speedwire_multicast_address_v6.sin6_port = htons(9522);
}


SpeedwireSocket::SpeedwireSocket(const SpeedwireSocket& rhs) :
    localhost(rhs.localhost),
    socket_interface(rhs.socket_interface) {
    socket_fd = rhs.socket_fd;
    socket_fd_ref_counter = rhs.socket_fd_ref_counter;
    if (socket_fd_ref_counter != NULL) {
        ++(*socket_fd_ref_counter);
    }
    socket_family = rhs.socket_family;
    socket_interface_v4 = rhs.socket_interface_v4;
    memcpy(&socket_interface_v6, &rhs.socket_interface_v6, sizeof(socket_interface_v6));
    isInterfaceAny = rhs.isInterfaceAny;
    speedwire_multicast_address_v4 = rhs.speedwire_multicast_address_v4;
    speedwire_multicast_address_v6 = rhs.speedwire_multicast_address_v6;
}


SpeedwireSocket &SpeedwireSocket::operator=(const SpeedwireSocket& rhs) {
    if (this != &rhs) {
        *this = rhs;  // calls the copy constructor
    }
    return *this;
}


/**
 *  Destructor
 */
SpeedwireSocket::~SpeedwireSocket(void) {
    if (socket_fd_ref_counter != NULL) {
        if (--(*socket_fd_ref_counter) <= 0) {
            closeSocket();
            socket_fd = -1;
            delete socket_fd_ref_counter;
            socket_fd_ref_counter = NULL;
        }
    }
}


/**
 *  Get socket file descriptor
 */
int SpeedwireSocket::getSocketFd(void) const {
    return socket_fd;
}

/**
 *  Get socket protocol, either AF_INET, AF_INET6 or AF_UNSPEC
 */
int SpeedwireSocket::getProtocol(void) const {
    return socket_family;
}

/**
 *  Return true, if socket protocol is AF_INET
 */
bool SpeedwireSocket::isIpv4(void) const {
    return (socket_family == AF_INET);
}

/**
 *  Return true, if socket protocol is AF_INET6
 */
bool SpeedwireSocket::isIpv6(void) const {
    return (socket_family == AF_INET6);
}

/**
 *  Return true, if this socket has been opened for IN4_ADDRESS_ANY or IN6_ADDRESS_ANY
 */
bool SpeedwireSocket::isIpAny(void) const {
    return isInterfaceAny;
}

/**
 *  Get interface name, that this socket has been opened with, or ""
 */
const std::string &SpeedwireSocket::getLocalInterfaceAddress(void) const {
    return socket_interface;
}

/**
 *  Get the speedwire ipv4 multicast address
 */
const sockaddr_in SpeedwireSocket::getSpeedwireMulticastIn4Address(void) const {
    return speedwire_multicast_address_v4;
}

/**
 *  Get the speedwire ipv6 multicast address (tbd)
 */
const sockaddr_in6 SpeedwireSocket::getSpeedwireMulticastIn6Address(void) const {
    return speedwire_multicast_address_v6;
}


/**
 *  Open socket for the given interface; the interface is either an ipv4 interface in 
 *  dot notation (e.g. "192.168.178.1") or an ipv6 interface in : notation
 */
int SpeedwireSocket::openSocket(const std::string &local_interface_address, const bool multicast) {
    socket_interface = local_interface_address;
    if (AddressConversion::isIpv4(local_interface_address) == true) {
        socket_family = AF_INET;
        socket_fd = openSocketV4(local_interface_address, multicast);
    }
    else if (AddressConversion::isIpv6(local_interface_address) == true) {
        socket_family = AF_INET6;
        socket_fd = openSocketV6(local_interface_address, multicast);
    }
    else {
        socket_family = AF_UNSPEC;
        perror("openSocket error - unknown protocol");
    }
    socket_interface = local_interface_address;
    return socket_fd;
}


/**
 *  Close socket
 */
int SpeedwireSocket::closeSocket(void) {
    int result = -1;
    if (socket_fd >= 0) {
        //fprintf(stdout, "closeSocket %d\n", socket_fd);
#ifdef _WIN32
        result = closesocket(socket_fd);
#else
        result = close(socket_fd);
#endif
        socket_fd = -1;
        socket_family = AF_UNSPEC;
        socket_interface.clear();
    }
    return result;
}


/**
 *  Open socket for the given interface described in ipv4 dot notation (e.g. "192.168.178.1")
 */
int SpeedwireSocket::openSocketV4(const std::string &local_interface_address, const bool multicast) {

    // convert the given interface address to socket structs
    socket_interface_v4 = AddressConversion::toInAddress(local_interface_address);
    isInterfaceAny = (memcmp(&socket_interface_v4, &IN4_ADDRESS_ANY, sizeof(socket_interface_v4)) == 0);    // if IN4_ADDRESS_ANY

    // open socket
    int fd = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) {
        return -1;
    }

    // set socket options
    unsigned char ttl = 1;
    unsigned char loopback = 1;
    uint32_t reuseaddr = 1;
    // with SO_REUSEADDR, all sockets that are bound to the port receive every incoming multicast UDP datagram destined to the shared
    // port. For backward compatibility reasons, this delivery does not apply to incoming unicast datagrams. Unicast datagrams are never
    // delivered to more than one socket, regardless of how many sockets are bound to the datagram's destination port.
    int result1 = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr, sizeof(reuseaddr));
#ifdef SO_REUSEPORT
    int result2 = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuseaddr, sizeof(reuseaddr));
#else
    int result2 = 0;
#endif
    int result3 = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));
#if 0
    int result4 = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&loopback, sizeof(loopback));
#else
    int result4 = 0;
#endif
    if (result1 < 0 || result2 < 0 || result3 < 0 || result4 < 0) {
        perror("setsockopt v4 failure");
        return -1;
    }

    // bind socket to interface
    // the local interface that this socket will receive multicast traffic from is defined by IP_ADD_MEMBERSHIP socket option;
    // if address is INADDR_ANY, the socket will receive unicast traffic from any local interface;
    // for windows hosts: if address is a local interface address, the socket will receive unicast and multicast traffic from that local interface;
    // for linux hosts: if address is a local interface address, the socket will receive just unicast from that local interface but no multicast traffic
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    //saddr.sin_addr.s_addr = htonl(INADDR_ANY);  // receive udp unicast and multicast traffic directed to port below
    saddr.sin_addr = socket_interface_v4;         // receive udp unicast and multicast traffic directed to port below
    if (multicast == true) {
        saddr.sin_port = speedwire_multicast_address_v4.sin_port;
    } else {
        saddr.sin_port = 0;                       // let the OS choose an available port
    }
#ifdef __APPLE__
    saddr.sin_len = sizeof(struct sockaddr_in);
#endif
    if (bind(fd, (struct sockaddr*)&saddr, sizeof(saddr))) {
        perror("bind v4 failure");
        return -1;
    }

    // join multicast group
    if (multicast == true) {
#ifdef _WIN32
        // windows requires that each interface separately joins the multicast group
        if (isInterfaceAny) {   // if IN4_ADDRESS_ANY
            std::vector<std::string> local_ip_addresses = localhost.getLocalIPAddresses();
            for (auto& addr : local_ip_addresses) {
                if (addr.find(':') == std::string::npos) {
                    struct ip_mreq mreq;
                    mreq.imr_multiaddr = speedwire_multicast_address_v4.sin_addr;
                    mreq.imr_interface = AddressConversion::toInAddress(addr);
                    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
                        perror("setsockopt");
                        return -1;
                    }
                }
            }
        }
        else {
            struct ip_mreq req;
            memset(&req, 0, sizeof(req));
            req.imr_multiaddr = speedwire_multicast_address_v4.sin_addr;
            req.imr_interface = socket_interface_v4;
            if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&req, sizeof(req)) < 0) {
                perror("setsockopt IP_ADD_MEMBERSHIP failure");
                return -1;
            }
        }
#else
        struct ip_mreq req;
        memset(&req, 0, sizeof(req));
        req.imr_multiaddr = speedwire_multicast_address_v4.sin_addr;
        req.imr_interface = socket_interface_v4;
        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&req, sizeof(req)) < 0) {
            perror("setsockopt IP_ADD_MEMBERSHIP failure");
            return -1;
        }
#endif
    }

    // define interface to use for outbound multicast and unicast traffic
    if (!isInterfaceAny) {   // if not IN4_ADDRESS_ANY
        if (multicast == true) {
            if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&socket_interface_v4, sizeof(socket_interface_v4)) < 0) {
                perror("setsockopt IP_MULTICAST_IF failure");
                return -1;
            }
        }
#ifdef IP_UNICAST_IF
        if (setsockopt(fd, IPPROTO_IP, IP_UNICAST_IF, (const char*)&socket_interface_v4, sizeof(socket_interface_v4)) < 0) {
            perror("setsockopt IP_UNICAST_IF failure");
            return -1;
        }
#endif
    }

    // wait until multicast membership messages have been sent
    LocalHost::sleep(1000);

    return fd;
}


/**
 *  Open socket for the given interface described in ipv6 interface in : notation
 */
int SpeedwireSocket::openSocketV6(const std::string &local_interface_address, const bool multicast) {

    // convert the given interface address to socket structs
    socket_interface_v6 = AddressConversion::toIn6Address(local_interface_address);
    isInterfaceAny = (memcmp(&socket_interface_v6, &IN6_ADDRESS_ANY, sizeof(socket_interface_v6)) == 0);    // if IN4_ADDRESS_ANY

    // open socket
    int fd = (int)socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) {
        return -1;
    }

    // set socket options
    uint32_t hops = 1;
    unsigned char ttl = 1;
    unsigned char loopback = 1;
    uint32_t reuseaddr = 1;
    // with SO_REUSEADDR, all sockets that are bound to the port receive every incoming multicast UDP datagram destined to the shared
    // port. For backward compatibility reasons, this delivery does not apply to incoming unicast datagrams. Unicast datagrams are never
    // delivered to more than one socket, regardless of how many sockets are bound to the datagram's destination port.
    int result1 = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
#ifdef SO_REUSEPORT
    int result2 = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuseaddr, sizeof(reuseaddr));
#else
    int result2 = 0;
#endif
    int result3 = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char*)&hops, sizeof(hops));
#if 0
    int result4 = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (const char*)&loopback, sizeof(loopback));
#else
    int result4 = 0;
#endif
    if (result1 < 0 || result2 < 0 || result3 < 0 || result4 < 0) {
        perror("setsockopt v6 failure");
        return -1;
    }

    // bind socket to interface
    // the local interface that this socket will receive multicast traffic from is defined by IP_ADD_MEMBERSHIP socket option;
    // if address is INADDR_ANY, the socket will receive unicast traffic from any local interface;
    // for windows hosts: if address is a local interface address, the socket will receive unicast and multicast traffic from that local interface;
    // for linux hosts: if address is a local interface address, the socket will receive just unicast from that local interface but no multicast traffic
    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin6_family = AF_INET6;
    //memcpy(&saddr.sin6_addr, &IN6_ADDRESS_ANY, sizeof(IN6_ADDRESS_ANY));  // receive udp traffic directed to port below
    saddr.sin6_addr = socket_interface_v6;
    if (multicast == true) {
        saddr.sin6_port = speedwire_multicast_address_v6.sin6_port;
    }
    else {
        saddr.sin6_port = 0; // let the OS choose an available port
    }
#ifdef __APPLE__
    saddr.sin6_len = sizeof(struct sockaddr_in6);
#endif
    if (bind(fd, (struct sockaddr*)&saddr, sizeof(saddr))) {
        perror("bind v6 failure");
        return -1;
    }

    // get the ipv6 interface index
    uint32_t ifindex = localhost.getInterfaceIndex(local_interface_address);
    if (ifindex == -1) {
        ifindex = 0;
    }

    // join multicast group
    if (multicast == true) {
        struct ipv6_mreq req;
        memset(&req, 0, sizeof(req));
        req.ipv6mr_multiaddr = speedwire_multicast_address_v6.sin6_addr;
        req.ipv6mr_interface = ifindex;
        if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&req, sizeof(req)) < 0) {
            perror("setsockopt IPV6_JOIN_GROUP failure");
            return -1;
        }
    }

    // define interface to use for outbound multicast and unicast traffic
    if (!isInterfaceAny) {   // if not IN6_ADDRESS_ANY
        if (multicast == true) {
            if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, (const char*)&ifindex, sizeof(ifindex)) < 0) {
                perror("setsockopt IPV6_MULTICAST_IF failure");
                return -1;
            }
        }
#ifdef IPV6_UNICAST_IF
        if (setsockopt(fd, IPPROTO_IP, IPV6_UNICAST_IF, (const char*)&ifindex, sizeof(ifindex)) < 0) {
            perror("setsockopt IP_UNICAST_IF failure");
            return -1;
        }
#endif
    }

    // wait until multicast membership messages have been sent
    LocalHost::sleep(1000);

    return fd;
}


/**
 *  Receive udp packet from an ipv4 socket and also provide the source address of the sender
 */
int SpeedwireSocket::recvfrom(const void *buff, const size_t buff_size, struct sockaddr_in &src) const {

    // wait for packet data
    socklen_t srclen = sizeof(src);
    int nbytes = ::recvfrom(socket_fd, (char*)buff, (int)buff_size, 0, (struct sockaddr *) &src, &srclen); // (char *) cast for WIN32 compatibility
    if (nbytes < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {  // this is by design, as we are using non-blocking io sockets
            return 0;
        }
#endif
        perror("recvfrom failure");
    }

#if 0
    char str[256];
    strcpy(str, inet_ntoa(src.sin_addr));
    fprintf(stderr, "source address: %s", str);
#endif

    return nbytes;
}


/**
 *  Receive udp packet from an ipv6 socket and also provide the source address of the sender
 */
int SpeedwireSocket::recvfrom(const void *buff, const size_t buff_size, struct sockaddr_in6 &src) const {

    // wait for packet data
    socklen_t srclen = sizeof(src);
    int nbytes = ::recvfrom(socket_fd, (char*)buff, (int)buff_size, 0, (struct sockaddr *) &src, &srclen); // (char *) cast for WIN32 compatibility
    if (nbytes < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {  // this is by design, as we are using non-blocking io sockets
            return 0;
        }
#endif
        perror("recvfrom failure");
    }

#if 0
    char str[256];
    strcpy(str, inet_ntoa(src.sin_addr));
    fprintf(stderr, "source address: %s", str);
#endif

    return nbytes;
}


/**
 *  Send udp multicast packet to the speedwire multicast address
 */
int SpeedwireSocket::send(const void* const buff, const unsigned long size) const {
    int nbytes = -1;
    if (isIpv4()) {
        nbytes = sendto(buff, size, speedwire_multicast_address_v4);
    }
    else if (isIpv6()) {
        nbytes = sendto(buff, size, speedwire_multicast_address_v6);
    }
    return nbytes;
}


/**
 *  Send udp multicast packet to the given address provided as string
 */
int SpeedwireSocket::sendto(const void* const buff, const unsigned long size, const std::string& dest) const {
    if (dest.find(':') == std::string::npos) {
        struct sockaddr_in addr = speedwire_multicast_address_v4;  // use as template
        addr.sin_addr = AddressConversion::toInAddress(dest);
        return sendto(buff, size, addr);
    }
    else {
        struct sockaddr_in6 addr = speedwire_multicast_address_v6;  // use as template
        addr.sin6_addr = AddressConversion::toIn6Address(dest);
        return sendto(buff, size, addr);
    }
    return -1;
}


/**
 *  Send udp multicast packet to the given ipv4 address
 */
int SpeedwireSocket::sendto(const void* const buff, const unsigned long size, const struct sockaddr_in &dest) const {
    int nbytes = sendto(buff, size, *(struct sockaddr*)&dest);
    return nbytes;
}


/**
 *  Send udp multicast packet to the given ipv6 address
 */
int SpeedwireSocket::sendto(const void* const buff, const unsigned long size, const struct sockaddr_in6 &dest) const {
    int nbytes = sendto(buff, size, *(struct sockaddr*)&dest);
    return nbytes;
}


/**
 *  Send udp multicast packet to the given ipv4 or ipv6 address
 */
int SpeedwireSocket::sendto(const void* const buff, const unsigned long size, const struct sockaddr& dest) const {
    return sendto(buff, size, dest, socket_interface_v4);
}


/**
 *  Send udp multicast packet to the given ipv4 or ipv6 address - this is the most low-level implementation
 */
int SpeedwireSocket::sendto(const void* const buff, const unsigned long size, const struct sockaddr& dest, const struct in_addr& local_interface_address) const {
    if (dest.sa_family == AF_INET) {
        if (local_interface_address.s_addr == 0) {
            perror("setsockopt IP_MULTICAST_IF failure - interface address is INADDR_ANY");
            return -1;
        }
        const struct sockaddr_in& destv4 = AddressConversion::toSockAddrIn(dest);
        if ((ntohl(destv4.sin_addr.s_addr) >> 24) == 239) {
            if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&local_interface_address, sizeof(local_interface_address)) < 0) {
                perror("setsockopt IP_MULTICAST_IF failure");
                return -1;
            }
        }
#ifdef IP_UNICAST_IF
        if (setsockopt(socket_fd, IPPROTO_IP, IP_UNICAST_IF, (const char*)&local_interface_address, sizeof(local_interface_address)) < 0) {
            perror("setsockopt IP_UNICAST_IF failure");
            return -1;
        }
#endif
    }
    // FIXME: not implemented yet
    //else if (dest.sa_family == AF_INET6) {
    //    const struct sockaddr_in6& destv6 = InternetAddressConversions::toSockAddrIn6(dest);
    //    if (destv6.sin6_addr.u.Byte[0] == 255) {
    //        uint32_t ifindex = localhost.getInterfaceIndex(socket_interface);
    //        if (ifindex == -1) { ifindex = 0; }
    //        if (setsockopt(socket_fd, IPPROTO_IP, IPV6_MULTICAST_IF, (const char*)&ifindex, sizeof(ifindex)) < 0) {
    //            perror("setsockopt IPV6_MULTICAST_IF failure");
    //            return -1;
    //        }
    //    }
    //}
    int nbytes = ::sendto(socket_fd, (char*)buff, size, 0, &dest, sizeof(dest));
    if (nbytes < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error == WSAENETUNREACH) {
            //perror("sendto failure - a socket operation was attempted to an unreachable network");
        }
        else {
            perror("sendto failure");
        }
#else
        perror("sendto failure");
#endif
    }
    return nbytes;
}

