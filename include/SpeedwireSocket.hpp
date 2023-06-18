#ifndef __LIBSPEEDWIRE_SPEEDWIRESOCKET_H__
#define __LIBSPEEDWIRE_SPEEDWIRESOCKET_H__

#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <string>
#include <LocalHost.hpp>

namespace libspeedwire {

    /**
     *  Class implementing a platform neutral socket abstraction for speedwire multicast traffic.
     */
    class SpeedwireSocket {

    private:

        int socket_fd;
        int* socket_fd_ref_counter;
        int socket_protocol;

        std::string     socket_interface;
        struct in_addr  socket_interface_v4;
        struct in6_addr socket_interface_v6;
        bool isInterfaceAny;

        const LocalHost& localhost;

        struct sockaddr_in  speedwire_multicast_address_v4;
        struct sockaddr_in6 speedwire_multicast_address_v6;

        int openSocketV4(const std::string& local_interface_address, const bool multicast);
        int openSocketV6(const std::string& local_interface_address, const bool multicast);

    public:

        // constructor & destructor
        SpeedwireSocket(const LocalHost& localhost);
        SpeedwireSocket(const SpeedwireSocket& rhs);
        SpeedwireSocket& operator=(const SpeedwireSocket& rhs);
        ~SpeedwireSocket(void);

        // getter methods for socket related information
        int getSocketFd(void) const;
        int getProtocol(void) const;
        const std::string& getLocalInterfaceAddress(void) const;
        const sockaddr_in  getSpeedwireMulticastIn4Address(void) const;
        const sockaddr_in6 getSpeedwireMulticastIn6Address(void) const;
        bool isIpv4(void) const;
        bool isIpv6(void) const;
        bool isIpAny(void) const;

        // open and close a speedwire socket on the given interface
        int openSocket(const std::string& local_interface_address, const bool multicast);
        int closeSocket(void);

        // receive data from the socket and return the sender address
        int recvfrom(const void* buff, const size_t buff_size, struct sockaddr_in& src) const;
        int recvfrom(const void* buff, const size_t buff_size, struct sockaddr_in6& src) const;

        // send data to the socket
        int send(const void* const buff, const unsigned long size) const;
        int sendto(const void* const buff, const unsigned long size, const struct sockaddr& dest) const;
        int sendto(const void* const buff, const unsigned long size, const struct sockaddr_in& dest) const;
        int sendto(const void* const buff, const unsigned long size, const struct sockaddr_in6& dest) const;
        int sendto(const void* const buff, const unsigned long size, const std::string& dest) const;
    };

}   // namespace libspeedwire

#endif
