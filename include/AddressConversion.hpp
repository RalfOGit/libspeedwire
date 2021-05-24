#ifndef __LIBSPEEDWIRE_ADDRESSCONVERSIONS_H__
#define __LIBSPEEDWIRE_ADDRESSCONVERSIONS_H__


#ifdef _WIN32
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <inaddr.h>
#include <in6addr.h>
#else
#include <netinet/in.h>
#include <net/if.h>
#endif
#include <cstdint>
#include <string>

namespace libspeedwire {

    /**
     *  Class implementing platform neutral conversions for bsd internet and socket addresses
     */
    class AddressConversion {
    public:

        // conversions from bsd socket and ip address information to and from std::string
        static std::string toString(const struct in_addr& address);
        static std::string toString(const struct in6_addr& address);
        static std::string toString(const struct sockaddr& address);
        static std::string toString(const struct sockaddr_in& address);
        static std::string toString(const struct sockaddr_in6& address);

        static bool isIpv4(const std::string& ip_address);
        static bool isIpv6(const std::string& ip_address);
        static struct in_addr  toInAddress(const std::string& ipv4_address);
        static struct in6_addr toIn6Address(const std::string& ipv6_address);

        // methods to network masks
        static struct in_addr  toInNetMask(const uint32_t prefix_length);
        static struct in6_addr toIn6NetMask(const uint32_t prefix_length);
        static bool resideOnSameSubnet(const struct in_addr& host1, const struct in_addr& host2, const uint32_t prefix_length);
        static bool resideOnSameSubnet(const struct in6_addr& host1, const struct in6_addr& host2, const uint32_t prefix_length);

        // conversions for bsd socket address information
        static struct sockaddr& toSockAddr(struct sockaddr_in& src);
        static struct sockaddr& toSockAddr(struct sockaddr_in6& src);
        static struct sockaddr_in& toSockAddrIn(struct sockaddr& src);
        static struct sockaddr_in6& toSockAddrIn6(struct sockaddr& src);
        static const struct sockaddr& toSockAddr(const struct sockaddr_in& src);
        static const struct sockaddr& toSockAddr(const struct sockaddr_in6& src);
        static const struct sockaddr_in& toSockAddrIn(const struct sockaddr& src);
        static const struct sockaddr_in6& toSockAddrIn6(const struct sockaddr& src);

        // remove non-ip characters like []%/, subnet masks, escape characters, etc
        static const std::string stripIPAddress(const std::string& ip_address);
    };

}   // namespace libspeedwire

#endif
