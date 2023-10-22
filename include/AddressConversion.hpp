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
#include <array>

namespace libspeedwire {

    /**
     *  Class implementing platform neutral conversions for bsd internet and socket addresses
     */
    class AddressConversion {
    public:

        // conversions from bsd socket and ip address information to std::string
        static std::string toString(const struct in_addr& address);
        static std::string toString(const struct in6_addr& address);
        static std::string toString(const struct sockaddr& address);
        static std::string toString(const struct sockaddr_in& address);
        static std::string toString(const struct sockaddr_in6& address);

        // conversions for ip addresses
        static bool isIpv4(const std::string& ip_address);
        static bool isIpv6(const std::string& ip_address);
        static struct in_addr  toInAddress(const std::string& ipv4_address);
        static struct in6_addr toIn6Address(const std::string& ipv6_address);

        // conversions for network masks
        static struct in_addr  toInNetMask(const uint32_t prefix_length);
        static struct in6_addr toIn6NetMask(const uint32_t prefix_length);
        static bool resideOnSameSubnet(const struct in_addr& host1, const struct in_addr& host2, const uint32_t prefix_length);
        static bool resideOnSameSubnet(const struct in6_addr& host1, const struct in6_addr& host2, const uint32_t prefix_length);

        // type casts for bsd socket address information
        static struct sockaddr& toSockAddr(struct sockaddr_in& src);
        static struct sockaddr& toSockAddr(struct sockaddr_in6& src);
        static struct sockaddr_in& toSockAddrIn(struct sockaddr& src);
        static struct sockaddr_in6& toSockAddrIn6(struct sockaddr& src);
        static const struct sockaddr& toSockAddr(const struct sockaddr_in& src);
        static const struct sockaddr& toSockAddr(const struct sockaddr_in6& src);
        static const struct sockaddr_in& toSockAddrIn(const struct sockaddr& src);
        static const struct sockaddr_in6& toSockAddrIn6(const struct sockaddr& src);

        // conversions for bsd socket address information
        static struct sockaddr toSockAddr(const struct in_addr& address, const uint16_t port = 0);
        static struct sockaddr toSockAddr(const struct in6_addr& address, const uint16_t port = 0);

        // conversions for ethernet mac addresses
        static std::array<uint8_t, 6> toMacAddress(const std::string& mac);
        static std::string toString(const std::array<uint8_t, 6>& mac);

        // remove non-ip characters like []%/, subnet masks, escape characters, etc
        static std::string stripIPAddress(const std::string& ip_address);

        static int hexToInt(const char nibble);
    };

}   // namespace libspeedwire

#endif
