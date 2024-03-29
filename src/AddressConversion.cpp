#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <cstring>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif

#include <AddressConversion.hpp>
using namespace libspeedwire;


/**
 *  Convert a binary ipv4 address into a string
 */
std::string AddressConversion::toString(const struct in_addr& address) {
    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr = address;
    return toString(socket_address);
}

/**
 *  Convert a binary ipv6 address into a string
 */
std::string AddressConversion::toString(const struct in6_addr& address) {
    struct sockaddr_in6 socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin6_family = AF_INET6;
    socket_address.sin6_addr = address;
    return toString(socket_address);
}

/**
 *  Convert a binary generic ipv4 / ipv6 socket address into a string
 */
std::string AddressConversion::toString(const struct sockaddr& address) {
    if (address.sa_family == AF_INET) {
        return toString(toSockAddrIn(address));
    }
    else if (address.sa_family == AF_INET6) {
        return toString(toSockAddrIn6(address));
    }
    return "unknown sockaddr family";
}

/**
 *  Convert a binary ipv4 socket address into a string
 */
std::string AddressConversion::toString(const struct sockaddr_in& address) {
    char buffer[20] = { 0 };
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    memset(host, 0, sizeof(host));
    memset(service, 0, sizeof(service));
    int len = 0;
    if (getnameinfo((const struct sockaddr*)&address, (socklen_t)sizeof(address), host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST) != 0) {
        perror("getnameinfo failed");
    }
    else {
        if (address.sin_port != 0) {
            len = snprintf(buffer, sizeof(buffer), "%s:%s\0", host, service);
        }
        else {
            len = snprintf(buffer, sizeof(buffer), "%s\0", host);
        }
    }
    return std::string(buffer);
}

/**
 *  Convert a binary ipv6 socket address into a string
 */
std::string AddressConversion::toString(const struct sockaddr_in6& address) {
    char buffer[256] = { 0 };
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    memset(host, 0, sizeof(host));
    memset(service, 0, sizeof(service));
    int len = 0;
    if (getnameinfo((const struct sockaddr*)&address, (socklen_t)sizeof(address), host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST) != 0) {
        perror("getnameinfo failed");
    }
    else {
        if (address.sin6_port != 0) {
            len = snprintf(buffer, sizeof(buffer), "[%s]:%s\0", host, service);
        }
        else {
            len = snprintf(buffer, sizeof(buffer), "%s\0", host);
        }
    }
    return std::string(buffer);
}

/**
 *  Check if the given string contains a valid ipv4 address
 */
bool AddressConversion::isIpv4(const std::string& ip_address) {
    struct in_addr addr;
    return (inet_pton(AF_INET, ip_address.c_str(), &addr) == 1);
}

/**
 *  Check if the given string contains a valid ipv6 address
 */
bool AddressConversion::isIpv6(const std::string& ip_address) {
    struct in6_addr addr;
    return (inet_pton(AF_INET6, ip_address.c_str(), &addr) == 1);
}

/**
 *  Convert an ipv4 string to an ipv4 binary address
 */
struct in_addr AddressConversion::toInAddress(const std::string& ipv4_address) {
    struct in_addr addr;
    memset(&addr, 0, sizeof(addr));
    if (inet_pton(AF_INET, ipv4_address.c_str(), &(addr)) != 1) {
        perror("inet_pton failure");
    }
    return addr;
}

/**
 *  Convert an ipv6 string to an ipv6 binary address
 */
struct in6_addr AddressConversion::toIn6Address(const std::string& ipv6_address) {
    struct in6_addr addr;
    memset(&addr, 0, sizeof(addr));
    std::string::size_type imask = ipv6_address.find_first_of('%');
    std::string ipv6 = ((imask == std::string::npos) ? ipv6_address : ipv6_address.substr(0, imask));
    if (inet_pton(AF_INET6, ipv6.c_str(), &(addr)) != 1) {
        perror("inet_pton failure");
    }
    return addr;
}


/**
 *  Convert the given ip address prefix length to an ipv4 netmask suitable for use with struct in_addr
 */
struct in_addr AddressConversion::toInNetMask(const uint32_t prefix_length) {
    struct in_addr mask;
    mask.s_addr = 0;
    if (prefix_length <= 32) {
        mask.s_addr = htonl(0xffffffff << (32 - prefix_length));
    }
    return mask;
}

/**
 *  Convert the given ip address prefix length to an ipv6 netmask suitable for use with struct in6_addr
 */
struct in6_addr AddressConversion::toIn6NetMask(const uint32_t prefix_length) {
    struct in6_addr mask;
    const int mask_size = sizeof(mask.s6_addr);
    memset(mask.s6_addr, 0, mask_size);
    if (prefix_length <= 8 * mask_size) {
        for (int32_t i = 8 * mask_size - 1; i >= (int32_t)prefix_length; --i) {
            mask.s6_addr[mask_size - i / 8 - 1] |= 1 << (i % 8);
        }
    }
    return mask;
}

/**
 *  Check if both given ipv4 hosts are residing on the same subnet as defined by its prefix_length.
 */
bool AddressConversion::resideOnSameSubnet(const struct in_addr& host1, const struct in_addr& host2, const uint32_t prefix_length) {
    struct in_addr netmask = toInNetMask(prefix_length);
    return ((host1.s_addr & netmask.s_addr) == (host2.s_addr & netmask.s_addr));
}

/**
 *  Check if both given ipv6 hosts are residing on the same subnet as defined by its prefix_length.
 */
bool AddressConversion::resideOnSameSubnet(const struct in6_addr& host1, const struct in6_addr& host2, const uint32_t prefix_length) {
    struct in6_addr netmask = toIn6NetMask(prefix_length);
    for (int i = 0; i < sizeof(netmask.s6_addr); ++i) {
        if ((host1.s6_addr[i] & netmask.s6_addr[i]) != (host2.s6_addr[i] & netmask.s6_addr[i])) {
            return false;
        }
    }
    return true;
}

/**
 *  Check if both given ipv4 or ipv6 hosts are residing on the same subnet as defined by its prefix_length.
 */
bool AddressConversion::resideOnSameSubnet(const std::string& host1, const std::string& host2, const uint32_t prefix_length) {
    if (isIpv4(host1) && isIpv4(host2)) {
        struct in_addr inaddr1 = AddressConversion::toInAddress(host1);
        struct in_addr inaddr2 = AddressConversion::toInAddress(host2);
        return resideOnSameSubnet(inaddr1, inaddr2, prefix_length);
    }
    if (isIpv6(host1) && isIpv6(host2)) {
        struct in6_addr inaddr1 = AddressConversion::toIn6Address(host1);
        struct in6_addr inaddr2 = AddressConversion::toIn6Address(host2);
        return resideOnSameSubnet(inaddr1, inaddr2, prefix_length);
    }
    return false;
}


/**
 *  Remove non-ip address related characters like []%/, subnet masks, escape characters, etc
 */
std::string AddressConversion::stripIPAddress(const std::string& ip_address) {
    std::string::size_type first_1 = ip_address.find('[');
    std::string::size_type first_index = (first_1 != std::string::npos ? first_1 : 0);
    std::string::size_type last_1 = ip_address.find('%');
    std::string::size_type last_2 = ip_address.find('/');
    std::string::size_type last_3 = ip_address.find(']');
    std::string::size_type last_n = (last_1 < last_2 ? (last_1 < last_3 ? last_1 : last_3) : (last_2 < last_3 ? last_2 : last_3));
    std::string::size_type last_index = (last_n != std::string::npos ? last_n : ip_address.size());
    return ip_address.substr(first_index, last_index);
}


/** Cast the given binary ipv4 socket address reference into a binary generic ipv4/ipv6 socket address reference in a type safe way */
struct sockaddr& AddressConversion::toSockAddr(struct sockaddr_in& src) { return (struct sockaddr&)src; }

/** Cast the given binary ipv6 socket address reference into a binary generic ipv4/ipv6 socket address reference in a type safe way */
struct sockaddr& AddressConversion::toSockAddr(struct sockaddr_in6& src) { return (struct sockaddr&)src; }

/** Cast the given binary generic ipv4/ipv6 socket address reference into a binary ipv4 socket address reference in a type safe way */
struct sockaddr_in& AddressConversion::toSockAddrIn(struct sockaddr& src) { return (struct sockaddr_in&)src; }

/** Cast the given binary generic ipv4/ipv6 socket address reference into a binary ipv6 socket address reference in a type safe way */
struct sockaddr_in6& AddressConversion::toSockAddrIn6(struct sockaddr& src) { return (struct sockaddr_in6&)src; }

/** Cast the given const binary ipv4 socket address reference into a binary generic const ipv4/ipv6 socket address reference in a type safe way */
const struct sockaddr& AddressConversion::toSockAddr(const struct sockaddr_in& src) { return (const struct sockaddr&)src; }

/** Cast the given const binary ipv6 socket address reference into a binary generic const ipv4/ipv6 socket address reference in a type safe way */
const struct sockaddr& AddressConversion::toSockAddr(const struct sockaddr_in6& src) { return (const struct sockaddr&)src; }

/** Cast the given const binary generic ipv4/ipv6 socket address reference into a binary const ipv4 socket address reference in a type safe way */
const struct sockaddr_in& AddressConversion::toSockAddrIn(const struct sockaddr& src) { return (const struct sockaddr_in&)src; }

/** Cast the given const binary generic ipv4/ipv6 socket address reference into a binary const ipv6 socket address reference in a type safe way */
const struct sockaddr_in6& AddressConversion::toSockAddrIn6(const struct sockaddr& src) { return (const struct sockaddr_in6&)src; }


struct sockaddr_in AddressConversion::toSockAddrIn(const struct in_addr& address, const uint16_t port) {
    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);
    socket_address.sin_addr = address;
    return socket_address;
}

struct sockaddr_in6 AddressConversion::toSockAddrIn6(const struct in6_addr& address, const uint16_t port) {
    struct sockaddr_in6 socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin6_family = AF_INET6;
    socket_address.sin6_port = htons(port);
    socket_address.sin6_addr = address;
    return socket_address;
}

struct sockaddr AddressConversion::toSockAddr(const struct in_addr& address, const uint16_t port) {
    return toSockAddr(toSockAddrIn(address, port));
}

struct sockaddr AddressConversion::toSockAddr(const struct in6_addr& address, const uint16_t port) {
    return toSockAddr(toSockAddrIn6(address, port));
}

struct sockaddr_in AddressConversion::toSockAddrIn(const std::string& ipv4_address, const uint16_t port) {
    return toSockAddrIn(toInAddress(ipv4_address), port);
}

struct sockaddr_in6 AddressConversion::toSockAddrIn6(const std::string& ipv6_address, const uint16_t port) {
    return toSockAddrIn6(toIn6Address(ipv6_address), port);
}


/**
 *  Convert a string representation of a an ethernet mac address into its binary format
 */
std::array<uint8_t, 6> AddressConversion::toMacAddress(const std::string& mac) {
    std::array<uint8_t, 6> arr;
    //arr.fill(0);
    size_t n = 0, i = 0;
    for (; i + 1 < mac.length() && n < arr.size(); i += 2) {
        int i1 = hexToInt(mac[i]);
        int i2 = hexToInt(mac[i+1]);
        if (i1 < 0 || i2 < 0) {
            break;
        }
        arr[n++] = i1 * 16 + i2;
        if (i + 2 < mac.length() && (mac[i + 2] == ':' || mac[i + 2] == '-')) {  // skip delimiter characters : or -
            ++i;
        }
    }
    if (i < mac.length() || n != arr.size()) {
        //perror("ethernet string to mac address failure");
        arr.fill(0);
    }
    return arr;
}

/**
 *  Convert a binary ethernet mac address into a string
 */
std::string AddressConversion::toString(const std::array<uint8_t, 6>& mac) {
    char temp[18];
    int n = snprintf(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (n == 17) {
        return temp;
    }
    return std::string();
}

/**
 *  Convert a hexadecimal character to an int value
 */
int AddressConversion::hexToInt(const char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

