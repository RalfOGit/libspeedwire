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
    char buffer[22] = { 0 };
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
 *  Check if the given string contains a valid ipv4 uri address, i.e. xx.xx.xx.xx or xx.xx.xx.xx:pp
 */
bool AddressConversion::isIpv4Uri(const std::string& uri_address) {
    size_t port_offset = uri_address.find(':');
    if (port_offset != std::string::npos && (port_offset + 1) < uri_address.length()) {
        std::string ipv4 = uri_address.substr(0, port_offset);
        std::string port = uri_address.substr(port_offset + 1);
        size_t invalid = port.find_first_not_of("0123456789");
        if (invalid == std::string::npos) {
            return isIpv4(ipv4);
        }
        return false;
    }
    return isIpv4(uri_address);
}

/**
 *  Check if the given string contains a valid ipv4 uri address, i.e. xx::xx:xx:xx or xx::xx:xx:xx%ss [xx::xx:xx:xx%ss]:pp
 */
bool AddressConversion::isIpv6Uri(const std::string& uri_address) {
    size_t trailing_square_bracket = uri_address.find(']');
    // check if there is an [...] encapsulation
    if (uri_address.find('[') == 0 && trailing_square_bracket != std::string::npos && trailing_square_bracket > 0) {
        // check if there is some port appended to the [...] encapsulation, i.e. [...]:pp
        size_t port_offset = uri_address.substr(trailing_square_bracket).find("]:");
        if (port_offset != std::string::npos) {
            port_offset = trailing_square_bracket + port_offset + 2;
            // check if there is some data behind the [...]:
            if (port_offset < uri_address.length()) {
                std::string port = uri_address.substr(port_offset);
                // check if there are just digits behind the [...]:
                size_t invalid = port.find_first_not_of("0123456789");
                if (invalid != std::string::npos) {
                    return false;   // non-digit data behind  [...]:
                }
            }
            else {
                return false;   // no characters behind  [...]:
            }
        }
        // check if there is some data behind the [...] that is not a port
        else if (uri_address.length() > (trailing_square_bracket + 1)) {
            return false;   // at least one character behind [...] and it is not a :
        }
        // check if there is some scope id behind the encapsulated ip address, i.e [...%ss]
        std::string ipv6 = uri_address.substr(1, trailing_square_bracket - 1);
        size_t percent_offset = ipv6.find('%');
        if (percent_offset != std::string::npos) {
            return isIpv6(ipv6.substr(0, percent_offset));
        }
        return isIpv6(ipv6);
    }
    // check if there is some scope id behind the unencapsulated ip address, i.e ...%ss
    size_t percent_offset = uri_address.find('%');
    if (percent_offset != std::string::npos) {
        return isIpv6(uri_address.substr(0, percent_offset));
    }
    return isIpv6(uri_address);
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


/** Cast the given binary ipv4 socket address reference into a binary generic ipv4/ipv6 socket address reference in a type safe way */
struct sockaddr& AddressConversion::toSockAddr(struct sockaddr_in& src) { return (struct sockaddr&)src; }

/** Cast the given binary ipv6 socket address reference into a binary generic ipv4/ipv6 socket address reference in a type safe way */
struct sockaddr& AddressConversion::toSockAddr(struct sockaddr_in6& src) { return (struct sockaddr&)src; }

/** Cast the given binary storage socket address reference into a binary generic ipv4/ipv6 socket address reference in a type safe way */
struct sockaddr& AddressConversion::toSockAddr(struct sockaddr_storage& src) { return (struct sockaddr&)src; }

/** Cast the given binary generic ipv4/ipv6 socket address reference into a binary ipv4 socket address reference in a type safe way */
struct sockaddr_in& AddressConversion::toSockAddrIn(struct sockaddr& src) { return (struct sockaddr_in&)src; }

/** Cast the given binary generic ipv4/ipv6 socket address reference into a binary ipv6 socket address reference in a type safe way */
struct sockaddr_in6& AddressConversion::toSockAddrIn6(struct sockaddr& src) { return (struct sockaddr_in6&)src; }

/** Cast the given const binary ipv4 socket address reference into a binary generic const ipv4/ipv6 socket address reference in a type safe way */
const struct sockaddr& AddressConversion::toSockAddr(const struct sockaddr_in& src) { return (const struct sockaddr&)src; }

/** Cast the given const binary ipv6 socket address reference into a binary generic const ipv4/ipv6 socket address reference in a type safe way */
const struct sockaddr& AddressConversion::toSockAddr(const struct sockaddr_in6& src) { return (const struct sockaddr&)src; }

/** Cast the given const binary storage socket address reference into a binary generic const ipv4/ipv6 socket address reference in a type safe way */
const struct sockaddr& AddressConversion::toSockAddr(const struct sockaddr_storage& src) { return (const struct sockaddr&)src; }

/** Cast the given const binary generic ipv4/ipv6 socket address reference into a binary const ipv4 socket address reference in a type safe way */
const struct sockaddr_in& AddressConversion::toSockAddrIn(const struct sockaddr& src) { return (const struct sockaddr_in&)src; }

/** Cast the given const binary generic ipv4/ipv6 socket address reference into a binary const ipv6 socket address reference in a type safe way */
const struct sockaddr_in6& AddressConversion::toSockAddrIn6(const struct sockaddr& src) { return (const struct sockaddr_in6&)src; }


struct sockaddr_storage AddressConversion::toSockAddrStorage(const struct sockaddr_in& src) {
    struct sockaddr_storage result;
    memset(&result, 0, sizeof(result));
    memcpy(&result, &src, sizeof(src));
    return result;
}

struct sockaddr_storage AddressConversion::toSockAddrStorage(const struct sockaddr_in6& src) {
    struct sockaddr_storage result;
    memset(&result, 0, sizeof(result));
    memcpy(&result, &src, sizeof(src));
    return result;
}


struct sockaddr_in AddressConversion::toSockAddrIn(const struct in_addr& address, const uint16_t port) {
    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
#ifdef __APPLE__
    socket_address.sin_len = sizeof(socket_address);
#endif
    socket_address.sin_port = htons(port);
    socket_address.sin_addr = address;
    return socket_address;
}

struct sockaddr_in6 AddressConversion::toSockAddrIn6(const struct in6_addr& address, const uint16_t port) {
    struct sockaddr_in6 socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin6_family = AF_INET6;
#ifdef __APPLE__
    socket_address.sin6_len = sizeof(socket_address);
#endif
    socket_address.sin6_port = htons(port);
    socket_address.sin6_addr = address;
    return socket_address;
}

struct sockaddr_storage AddressConversion::toSockAddr(const std::string& ip_address, const uint16_t port) {
    struct sockaddr_storage socketaddr;
    memset(&socketaddr, 0, sizeof(socketaddr));
    if (AddressConversion::isIpv4(ip_address)) {
        struct sockaddr_in sockin = AddressConversion::toSockAddrIn(ip_address, port);
        socketaddr = AddressConversion::toSockAddrStorage(sockin);
    }
    else if (AddressConversion::isIpv6(ip_address)) {
        struct sockaddr_in6 sockin6 = AddressConversion::toSockAddrIn6(ip_address, port);
        socketaddr = AddressConversion::toSockAddrStorage(sockin6);
    }
    return socketaddr;
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
 *  Check address scope
 */
bool AddressConversion::isLoopbackAddress(const struct in_addr& address) {
    return (address.s_addr & 0xff000000) == 0x7f000000;     // 127.0.0.0/8
}

bool AddressConversion::isBroadcastAddress(const struct in_addr& address) {
    return address.s_addr == 0xffffffff;                    // 255.255.255.255/32
}

bool AddressConversion::isMulticastAddress(const struct in_addr& address) {
    return (address.s_addr & 0xf0000000) == 0xe0000000;     // 224.0.0.0/4
}

bool AddressConversion::isPrivateAddress(const struct in_addr& address) {
    return (address.s_addr & 0xff000000) == 0x0a000000 ||   // 10.0.0.0/8
        (address.s_addr & 0xfff00000) == 0xac100000 ||   // 172.16.0.0/12
        (address.s_addr & 0xffff0000) == 0xc0a80000;     // 192.168.0.0/16
}

bool AddressConversion::isLinkLocalAddress(const struct in_addr& address) {
    return (address.s_addr & 0xffff0000) == 0xa9fe0000;     // 169.254.0.0/16
}

bool AddressConversion::isLoopbackAddress(const struct in6_addr& address) {
    static const unsigned char loopback[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    return memcmp(&address, &loopback, sizeof(address)) == 0;   // ::1/128
}

bool AddressConversion::isMulticastAddress(const struct in6_addr& address) {
    return (((const unsigned char*)&address)[0] == 0xff);       // ff::/8
}

bool AddressConversion::isLinkLocalAddress(const struct in6_addr& address) {
    static const unsigned char nulls[6] = { 0, 0, 0, 0, 0, 0 };
    return (((const unsigned char*)&address)[0] == 0xfe &&      // fe80::/64
        ((const unsigned char*)&address)[1] == 0x80 &&
        memcmp(((const unsigned char*)&address) + 2, nulls, sizeof(nulls)) == 0);
}

bool AddressConversion::isUniqueLocalAddress(const struct in6_addr& address) {
    return (((const unsigned char*)&address)[0] == 0xfc ||      // fc::/7
        ((const unsigned char*)&address)[0] == 0xfd);
}

bool AddressConversion::isGlobalAddress(const struct in6_addr& address) {
    return !isLoopbackAddress(address) && !isMulticastAddress(address) &&
        !isLinkLocalAddress(address) && !isUniqueLocalAddress(address);
}


// Convert a string representation of a an ethernet mac, eui48 or eui64 address into its binary format
template<int N> static std::array<uint8_t, N> toEUI(const std::string& mac) {
    std::array<uint8_t, N> arr;
    size_t n = 0, i = 0;
    for (; i + 1 < mac.length() && n < N; i += 2) {
        int i1 = AddressConversion::hexToInt(mac[i]);
        int i2 = AddressConversion::hexToInt(mac[i + 1]);
        if (i1 < 0 || i2 < 0) {
            break;
        }
        arr[n++] = i1 * 16 + i2;
        if (i + 2 < mac.length() && (mac[i + 2] == ':' || mac[i + 2] == '-')) {  // skip delimiter characters : or -
            ++i;
        }
    }
    if (i < mac.length() || n != N) {
        //perror("ethernet string to mac address failure");
        arr.fill(0);
    }
    return arr;
}

// Convert a binary ethernet mac, eui48 or eui64 address into a string
template<int N> static std::string euiToString(const std::array<uint8_t, N>& mac) {
    std::string result;
    for (const uint8_t c : mac) {
        if (result.length() > 0) result.append(1, ':');
        result.append(1, "0123456789abcdef"[(c >> 4) & 0xf]);
        result.append(1, "0123456789abcdef"[c & 0xf]);
    }
    return result;
}

/**
 *  Convert a string representation of a an ethernet mac address into its binary format
 */
std::array<uint8_t, 6> AddressConversion::toMacAddress(const std::string& mac) {
    return toEUI<6>(mac);
}

/**
 *  Convert a string representation of a an eui64 address into its binary format
 */
std::array<uint8_t, 8> AddressConversion::toEUI64(const std::string& eui64) {
    return toEUI<8>(eui64);
}

/**
 *  Convert a binary ethernet mac address into a string
 */
std::string AddressConversion::toString(const std::array<uint8_t, 6>& mac) {
    return euiToString<6>(mac);
}

/**
 *  Convert a binary extended mac address into a string
 */
std::string AddressConversion::toString(const std::array<uint8_t, 8>& mac) {
    return euiToString<8>(mac);
}

/**
 *  Convert a binary ethernet mac address into a binary modified extended mac address
 */
std::array<uint8_t, 8> AddressConversion::toEUI64(const std::array<uint8_t, 6>& mac) {
    return { (uint8_t)(mac[0] ^ 0x02), mac[1], mac[2], 0xff, 0xfe, mac[3], mac[4], mac[5] };
}


/**
 *  Extract ip address from uri, i.e. 192.168.1.1:8080 => 192.168.1.1 or [ff02::fb%21}:8080 => ff02
 */
std::string AddressConversion::extractIPAddress(const std::string& uri_address) {
    if (isIpv4Uri(uri_address)) {
        size_t port_offset = uri_address.find(':');
        if (port_offset != std::string::npos) {
            return uri_address.substr(0, port_offset);
        }
        return uri_address;
    }
    else if (isIpv6Uri(uri_address)) {
        std::string::size_type first = uri_address.find('[');
        std::string::size_type last = uri_address.find_first_of("%]");
        first = (first == std::string::npos ? 0 : first + 1);
        if (last == std::string::npos) { last = uri_address.length(); }
        return uri_address.substr(first, last - first);
    }
    return "";
}

/**
 *  Extract port from uri, i.e. 192.168.1.1:8080 or [ff02::fb%21}:8080 => 8080
 */
std::string AddressConversion::extractIPPort(const std::string& uri_address) {
    if (isIpv4Uri(uri_address)) {
        size_t port_offset = uri_address.find(':');
        if (port_offset != std::string::npos) {
            return uri_address.substr(port_offset + 1);
        }
    }
    else if (isIpv6Uri(uri_address)) {
        std::string::size_type offs = uri_address.find("]:");
        if (offs != std::string::npos) {
            return uri_address.substr(offs + 2, uri_address.length() - offs - 2);
        }
    }
    return "";
}

/**
 *  Extract zone id from ipv6 uri, i.e. [ff02::fb%21}:8080 => 21 or [ff02::fb%eth0}:8080 => eth0.
 *  Zone id is the name used for uri notations in RFC 4007. If it is a numeric value, it corresponds
*   to the interface id. If it is a non-numeric value like eth0, it needs to be mapped to the interface
 *  id. The scopeid in sockaddr_in6 is not necessarily identical to the interface id, as multiple interfaces
 *  can belong to the same zone. For Windows all ids are identical: zone id == interface id == scope id.
 */
std::string AddressConversion::extractIPZoneId(const std::string& uri_address) {
    if (isIpv6Uri(uri_address)) {
        std::string::size_type first = uri_address.find('%');
        if (first != std::string::npos) {
            std::string::size_type last = uri_address.find(']');
            if (last == std::string::npos) {
                last = uri_address.length();
            }
            return uri_address.substr(first + 1, last - first - 1);
        }
    }
    return "";
}

/**
 *  Interprete ip addresses in uri format, i.e. 192.168.1.1:8080 or [ff02::fb%21}:8080
 */
struct sockaddr_storage AddressConversion::toSockAddrStorageFromUri(const std::string& uri_address) {
    struct sockaddr_storage socketaddr;
    memset(&socketaddr, 0, sizeof(socketaddr));
    if (isIpv4Uri(uri_address)) {
        const std::string ipv4addr = extractIPAddress(uri_address);
        const std::string ipv4port = extractIPPort(uri_address);
        size_t nchars = 0;
        size_t port = toUint(ipv4port, nchars);
        struct sockaddr_in sockaddr = toSockAddrIn(ipv4addr, (uint16_t)port);
        socketaddr = toSockAddrStorage(sockaddr);
    }
    else if (isIpv6Uri(uri_address)) {
        const std::string ipv6addr = extractIPAddress(uri_address);
        const std::string ipv6port = extractIPPort(uri_address);
        //const std::string ipv6zone = extractIPZoneId(uri_address);
        size_t nchars = 0;
        size_t port = toUint(ipv6port, nchars);
        struct sockaddr_in6 sockaddr = toSockAddrIn6(ipv6addr, (uint16_t)port);
        socketaddr = toSockAddrStorage(sockaddr);
    }
    return socketaddr;
}


/**
 *  Remove occurences of characters in the given set of characters from the given string.
 */
std::string AddressConversion::stripChars(const std::string& string, const std::string& chars) {
    std::string result = string;
    std::string::size_type offset = 0;
    while (offset < result.length()) {
        offset = result.find_first_of(chars, offset);
        if (offset == std::string::npos) break;
        result = result.erase(offset, 1);
    }
    return result;
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


/**
 *  Convert a character string to an unsigned integer value.
 */
size_t AddressConversion::toUint(const std::string& string, size_t& nchars) {
    size_t int_value = 0;
    for (size_t offs = 0; offs < string.length(); ++offs) {
        char c = string[offs];
        if (c < '0' || c > '9') {
            nchars = offs;
            return int_value;
        }
        int digit = c - '0';
        int_value = int_value * 10 + digit;
    }
    nchars = string.length();
    return int_value;
}


/**
 *  Convert an unsigned int to a string, using the given radix
 */
std::string AddressConversion::uintToString(size_t value, unsigned int radix) {
    std::string result;
    do {
        size_t digit = value % radix;
        value /= radix;
        char c = "0123456789ABCDEF"[digit];
        result.insert(0, 1, c);
    } while (value > 0);
    return result;
}
