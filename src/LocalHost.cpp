#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <cstring>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <chrono>
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

#include <LocalHost.hpp>
#include <AddressConversion.hpp>

/**
 *  Class implementing platform neutral abstractions for host related information
 */


/**
 *  Constructor
 */
LocalHost::LocalHost(void) {

#ifdef _WIN32
    // initialize Windows Socket API with given VERSION.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        perror("WSAStartup failure");
    }
#endif

    // query hostname
    cacheHostname(LocalHost::queryHostname());

    // query interfaces
    cacheLocalIPAddresses(LocalHost::queryLocalIPAddresses());

    // query interface information, such as interface names, mac addresses and local ip addressesl and interface indexes
    const std::vector<LocalHost::InterfaceInfo> &infos = LocalHost::queryLocalInterfaceInfos();
    cacheLocalInterfaceInfos(infos);
}

/**
 *  Destructor
 */
LocalHost::~LocalHost(void) {
}


/**
 *  Getter for cached hostname
 */
const std::string &LocalHost::getHostname(void) const {
    return hostname;
}

/**
 *  Setter to cache the hostname
 */
void LocalHost::cacheHostname(const std::string &hostname) {
    this->hostname = hostname;
}


/**
 *  Getter for cached interface names
 */
const std::vector<std::string> &LocalHost::getLocalIPAddresses(void) const {
    return local_ip_addresses;
}

const std::vector<std::string> LocalHost::getLocalIPv4Addresses(void) const {
    return local_ipv4_addresses;
}

const std::vector<std::string> LocalHost::getLocalIPv6Addresses(void) const {
    return local_ipv6_addresses;
}

/**
 *  Setter to cache the interface names
 */
void LocalHost::cacheLocalIPAddresses(const std::vector<std::string> &local_ip_addresses) {
    this->local_ip_addresses = local_ip_addresses;
    local_ipv4_addresses.clear();
    local_ipv6_addresses.clear();
    for (auto& a : local_ip_addresses) {
        if (a.find(':') == std::string::npos) {
            local_ipv4_addresses.push_back(a);
        } else {
            local_ipv6_addresses.push_back(a);
        }
    }
}


/**
 *  Getter for cached interface informations
 */
const std::vector<LocalHost::InterfaceInfo> &LocalHost::getLocalInterfaceInfos(void) const {
    return local_interface_infos;
}

/**
 *  Setter to cache the interface informations
 */
void LocalHost::cacheLocalInterfaceInfos(const std::vector<LocalHost::InterfaceInfo> &infos) {
    local_interface_infos = infos;
}

/**
 *  Getter for obtaining the mac address for a given ip address that is associated with a local interface
 */
const std::string LocalHost::getMacAddress(const std::string &local_ip_address) const {
    for (auto &info : local_interface_infos) {
        for (auto &addr : info.ip_addresses) {
            if (local_ip_address == addr) {
                return info.mac_address;
            }
        }
    }
    return "";
}

/**
 *  Getter for obtaining the interface name for a given ip address that is associated with a local interface
 */
const std::string LocalHost::getInterfaceName(const std::string& local_ip_address) const {
    for (auto& info : local_interface_infos) {
        for (auto& addr : info.ip_addresses) {
            if (local_ip_address == addr) {
                return info.if_name;
            }
        }
    }
    return "";
}

/**
 *  Getter for obtaining the interface index for a given ip address that is associated with a local interface.
 *  This is needed for setting up ipv6 multicast sockets.
 */
const uint32_t LocalHost::getInterfaceIndex(const std::string & local_ip_address) const {
    for (auto& info : local_interface_infos) {
        for (auto &addr : info.ip_addresses) {
            if (addr == local_ip_address) {
                return info.if_index;
            }
        }
    }
    return (uint32_t)-1;
}

/**
 *  Getter for obtaining the interface address prefix length for a given ip address that is associated with a local interface.
 */
const uint32_t LocalHost::getInterfacePrefixLength(const std::string& local_ip_address) const {
    for (auto& info : local_interface_infos) {
        auto it = info.ip_address_prefix_lengths.find(local_ip_address);
        if (it != info.ip_address_prefix_lengths.end()) {
            return it->second;
        }
    }
    return (uint32_t)-1;
}


/**
 *  Query the local hostname from the operating system
 */
const std::string LocalHost::queryHostname(void) {
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) != 0) {
        perror("gethostname() failure");
        return std::string();
    }
    return std::string(buffer);
}


/**
 *  Query the local ip addresses from the operating system
 */
std::vector<std::string> LocalHost::queryLocalIPAddresses(void) {
    std::vector<std::string> interfaces;
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        return interfaces;
    }
    struct addrinfo *info = NULL;
    if (getaddrinfo(hostname, NULL, NULL, &info) != 0) {
        perror("getaddrinfo");
        return interfaces;
    }
    while (info != NULL) {
        if (info->ai_protocol == 0 &&
            (info->ai_family == AF_INET ||
             info->ai_family == AF_INET6)) {
            interfaces.push_back(AddressConversion::toString(*info->ai_addr));
        }
        info = info->ai_next;
    }
    freeaddrinfo(info);
    return interfaces;
}


/**
 *  Query information related to local interfaces from the operating system
 */
std::vector<LocalHost::InterfaceInfo> LocalHost::queryLocalInterfaceInfos(void) {
    std::vector<LocalHost::InterfaceInfo> addresses;
#ifdef _WIN32
    PIP_ADAPTER_ADDRESSES AdapterAdresses;
    DWORD dwBufLen = sizeof(PIP_ADAPTER_ADDRESSES);

    AdapterAdresses = (IP_ADAPTER_ADDRESSES *)malloc(sizeof(IP_ADAPTER_ADDRESSES));
    if (AdapterAdresses == NULL) {
        perror("Error allocating memory needed to call GetAdaptersAddresses\n");
        return addresses;
    }

    // Make an initial call to GetAdaptersAddresses to get the necessary size into the dwBufLen variable
    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, AdapterAdresses, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterAdresses);
        AdapterAdresses = (IP_ADAPTER_ADDRESSES *)malloc(dwBufLen);
        if (AdapterAdresses == NULL) {
            perror("Error allocating memory needed to call GetAdaptersAddresses\n");
            return addresses;
        }
    }

    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, AdapterAdresses, &dwBufLen) == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pAdapterAddresses = AdapterAdresses;
        while (pAdapterAddresses != NULL) {
            if (pAdapterAddresses->OperStatus == IF_OPER_STATUS::IfOperStatusUp && pAdapterAddresses->PhysicalAddressLength == 6) {  // PhysicalAddressLength == 0 for loopback interfaces
                char mac_addr[18];
                snprintf(mac_addr, sizeof(mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
                    pAdapterAddresses->PhysicalAddress[0], pAdapterAddresses->PhysicalAddress[1],
                    pAdapterAddresses->PhysicalAddress[2], pAdapterAddresses->PhysicalAddress[3],
                    pAdapterAddresses->PhysicalAddress[4], pAdapterAddresses->PhysicalAddress[5]);
                LocalHost::InterfaceInfo info;
                char friendly[256];
                snprintf(friendly, sizeof(friendly), "%S", pAdapterAddresses->FriendlyName);
                info.if_name = std::string(friendly);
                info.if_index = pAdapterAddresses->Ipv6IfIndex;
                info.mac_address = mac_addr;

                PIP_ADAPTER_UNICAST_ADDRESS_LH unicast_address = pAdapterAddresses->FirstUnicastAddress;
                do {
                    std::string ip_name = AddressConversion::toString(*(sockaddr*)(unicast_address->Address.lpSockaddr));
                    info.ip_addresses.push_back(ip_name);
                    info.ip_address_prefix_lengths[ip_name] = unicast_address->OnLinkPrefixLength;
                    fprintf(stdout, "address: %28.*s  prefixlength: %d  mac: %s  name: \"%s\"\n", (int)ip_name.length(), ip_name.c_str(), unicast_address->OnLinkPrefixLength, mac_addr, info.if_name.c_str());
                    unicast_address = unicast_address->Next;
                } while (unicast_address != NULL);

                addresses.push_back(info);
            }
            pAdapterAddresses = pAdapterAddresses->Next;
        }
    }
    free(AdapterAdresses);
#else
    int s = socket(PF_INET, SOCK_DGRAM, 0);
    struct ifconf ifc;
    struct ifreq* ifr;
    char buf[16384];
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(s, SIOCGIFCONF, &ifc) == 0) {
        ifr = ifc.ifc_req;
        for (int i = 0; i < ifc.ifc_len;) {
            struct ifreq buffer;
            memset(&buffer, 0x00, sizeof(buffer));
            strcpy(buffer.ifr_name, ifr->ifr_name);
            InterfaceInfo info;
            info.if_name = std::string(buffer.ifr_name);
            info.if_index = if_nametoindex(buffer.ifr_name);
            std::string ip_name = AddressConversion::toString(ifr->ifr_ifru.ifru_addr);
            info.ip_addresses.push_back(ip_name);

#ifndef __APPLE__
            if (ioctl(s, SIOCGIFHWADDR, &buffer) == 0) {
                struct sockaddr saddr = buffer.ifr_ifru.ifru_hwaddr;
                char mac_addr[18];
                snprintf(mac_addr, sizeof(mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
                    (uint8_t)saddr.sa_data[0], (uint8_t)saddr.sa_data[1], (uint8_t)saddr.sa_data[2],
                    (uint8_t)saddr.sa_data[3], (uint8_t)saddr.sa_data[4], (uint8_t)saddr.sa_data[5]);
                info.mac_address = mac_addr;
            }
            size_t len = sizeof(struct ifreq);
#else
            size_t len = IFNAMSIZ + ifr->ifr_addr.sa_len;
#endif
            /* try to get network mask */
            uint32_t prefix = 0;
            if (ioctl(s, SIOCGIFNETMASK, &buffer) == 0) {
                struct sockaddr smask = buffer.ifr_ifru.ifru_netmask;
                if (smask.sa_family == AF_INET) {
                    struct sockaddr_in& smaskv4 = AddressConversion::toSockAddrIn(smask);
                    uint32_t saddr = smaskv4.sin_addr.s_addr;
                    for (int i = 0; i < 32; ++i) {
                        if ((saddr & (1u << i)) == 0) break;
                        ++prefix;
                    }
                }
                else if (smask.sa_family == AF_INET6) {
                    struct sockaddr_in6 smaskv6 = AddressConversion::toSockAddrIn6(smask);
                    for (int i = 0; i < sizeof(smaskv6.sin6_addr.s6_addr); ++i) {
                        uint8_t b = smaskv6.sin6_addr.s6_addr[i];
                        for (int j = 0; j < 8; ++j) {
                            if ((b & (1u << j)) == 0) break;
                            ++prefix;
                        }
                    }
                }
            }
            info.ip_address_prefix_lengths[ip_name] = prefix;
            fprintf(stdout, "address: %28.*s  prefixlength: %d  mac: %s  name: \"%s\"\n", (int)ip_name.length(), ip_name.c_str(), prefix, info.mac_address.c_str(), info.if_name.c_str());
            addresses.push_back(info);

            ifr = (struct ifreq*)((char*)ifr + len);
            i += len;
        }
    }
    close(s);
#endif
    return addresses;
}



/**
 *  Platform neutral sleep
 */
void LocalHost::sleep(uint32_t millis) {
#ifdef _WIN32
    Sleep(millis);
#else
    ::sleep(millis / 1000);
#endif
}


/**
 *  Platform neutral get tick count in ms ticks
 */
uint64_t LocalHost::getTickCountInMs(void) {  // return a tick counter with ms resolution
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec spec;
    if (clock_gettime(CLOCK_MONOTONIC, &spec) == -1) {
        abort();
    }
    return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
#endif
}


/**
 *  Platform neutral get unix epoch time in ms
 */
uint64_t LocalHost::getUnixEpochTimeInMs(void) {
#ifdef _WIN32
    std::chrono::system_clock::duration time = std::chrono::system_clock::now().time_since_epoch();
    std::chrono::system_clock::period period = std::chrono::system_clock::period();
    return (time.count() * period.num) / (period.den/1000);
#else
    struct timespec spec;
    if (clock_gettime(CLOCK_REALTIME, &spec) == -1) {
        abort();
    }
    return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
#endif
}

/*
 *  Match given ip address to longest matching local interface ip address
 */
static std::string::size_type findFirstDifference(const std::string& str1, const std::string& str2) {
    const std::string s1 = AddressConversion::stripIPAddress(str1);
    const std::string s2 = AddressConversion::stripIPAddress(str2);
    const char *c1 = s1.c_str(),  *c2 = s2.c_str();
    for (std::string::size_type i = 0; c1 != NULL && c2 != NULL; ++i) {
        if (*c1++ != *c2++)
            return i;
    }
    return std::string::npos;
}

const std::string LocalHost::getMatchingLocalIPAddress(std::string ip_address) const {
    std::string::size_type index = std::string::npos;
    std::string::size_type difference = 0;
    std::string match;
    for (auto& addr : local_ip_addresses) {
        std::string::size_type diff = findFirstDifference(ip_address, addr);
        if (diff > difference) {
            difference = diff;
            match = addr;
        }
    }
    return match;
}
