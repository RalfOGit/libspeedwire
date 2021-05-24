//
// Simple program to perform SMA speedwire device discovery
// https://www.sma.de/fileadmin/content/global/Partner/Documents/sma_developer/SpeedwireDD-TI-de-10.pdf
//

#ifdef _WIN32
    #include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
    #include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>	 // for sleep()
#endif

#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <LocalHost.hpp>
#include <AddressConversion.hpp>
#include <SpeedwireSocketSimple.hpp>
using namespace libspeedwire;


const char      *SpeedwireSocketSimple::multicast_group = "239.12.255.254";
const int        SpeedwireSocketSimple::multicast_port  = 9522;
int              SpeedwireSocketSimple::fd = -1;
SpeedwireSocketSimple *SpeedwireSocketSimple::instance = NULL;


SpeedwireSocketSimple::SpeedwireSocketSimple(void) {
    fd = -1;
}

SpeedwireSocketSimple::~SpeedwireSocketSimple(void) {
    if (fd >= 0) {
        close();
    }
}

// singleton
SpeedwireSocketSimple *SpeedwireSocketSimple::getInstance(void) {
    if (instance == NULL) {
        instance = new SpeedwireSocketSimple();
        fd = -1;
    }
    if (fd < 0){
        fd = instance->open();
        if (fd < 0) {
            perror("cannot open socket");
            instance->close();
            delete instance;
            instance = NULL;
        }
    }
    return instance;
}

// open a socket to send and receive speedwire udp packets
int SpeedwireSocketSimple::open(void) {

#ifdef _WIN32
    // initialize Windows Socket API with given VERSION.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        perror("WSAStartup failure");
        return -1;
    }
#endif

    // create what looks like an ordinary UDP socket
    int fd = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) {
        perror("cannot create socket");
        return -1;
    }

    // allow multiple sockets to use the same PORT number
    uint32_t yes = 1;
    if ( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0 ) { // (char *) cast for WIN32 compatibility
        perror("Reusing ADDR failed");
        return -1;
    }

    // bind socket to interface
    // for udp, this defines the source(!) addresses to receive data from,
    // the local interface that this socket will receive multicast traffic
    // from is defined by the IP_ADD_MEMBERSHIP socket option.
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(multicast_port);

    // bind to any available interface address
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    // use setsockopt() to request that the kernel joins the multicast group
#ifdef _WIN32
    // windows requires that each interface separately joins the multicast group
    std::vector<std::string> local_ip_addresses = LocalHost::getInstance().getLocalIPv4Addresses();
    for (auto& addr : local_ip_addresses) {
        if (AddressConversion::isIpv6(addr) == true) continue;  // ignore ipv6 addresses
        struct ip_mreq mreq;
        mreq.imr_multiaddr = AddressConversion::toInAddress(multicast_group);
        mreq.imr_interface = AddressConversion::toInAddress(addr);
        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
            perror("setsockopt");
            return -1;
        }
    }
#else
    struct ip_mreq mreq;
    mreq.imr_multiaddr = AddressConversion::toInAddress(multicast_group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if ( setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0 ){
        perror("setsockopt");
        return -1;
    }
#endif

    // wait until multicast membership messages have been sent
    LocalHost::sleep(1000);

    return fd;
}

// send udp packet to the speedwire multicast address
int SpeedwireSocketSimple::send(const void *const buff, const unsigned long size) {

    // set up multicast destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = AddressConversion::toInAddress(multicast_group);
    dest_addr.sin_port = htons(multicast_port);

    // now just sendto() our destination
    int nbytes = sendto(buff, size, dest_addr);
    if (nbytes < 0) {
        perror("sendto failure");
    }
    return nbytes;
}

// send udp packet to the given destination address
int SpeedwireSocketSimple::sendto(const void *const buff, const unsigned long size, const struct sockaddr_in &dest) {

#if 0
    // if there are multiple interfaces connected to this host, configure the outbound interface
    struct in_addr src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    //src_addr.s_addr = inet_addr("192.168.168.16");    // use specific interface
    src_addr.s_addr = htonl(INADDR_ANY);                // use default interface
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &src_addr, sizeof(src_addr));
#endif

    // now just sendto() our destination
    int nbytes = ::sendto(fd, (char*)buff, size, 0, (struct sockaddr*) &dest, sizeof(dest)); // (char *) cast for WIN32 compatibility
    if (nbytes < 0) {
        perror("sendto failure");
    }
    return nbytes;
}

// receive udp packet
int SpeedwireSocketSimple::recv(void *buff, const unsigned long size) {

    // initialize the source address struct
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    // wait for packet data
    int nbytes = recvfrom(buff, size, addr);
    if (nbytes < 0) {
        perror("recvfrom failure");
    }

    return nbytes;
}

// receive udp packet and also provide the source address of the sender
int SpeedwireSocketSimple::recvfrom(void *buff, const unsigned long size, struct sockaddr_in &src) {

    // wait for packet data
    socklen_t srclen = sizeof(src);
    int nbytes = ::recvfrom(fd, (char*)buff, size, 0, (struct sockaddr *) &src, &srclen); // (char *) cast for WIN32 compatibility
    if (nbytes < 0) {
        perror("recvfrom failure");
    }

    return nbytes;
}

// close the speedwire socket
int SpeedwireSocketSimple::close(void) {
#ifdef _WIN32
    int result = closesocket(fd);
    WSACleanup();
#else
    int result = ::close(fd);
#endif
    fd = -1;
    return result;
}
