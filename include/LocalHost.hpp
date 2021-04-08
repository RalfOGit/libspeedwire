#ifndef __LOCALHOST_H__
#define __LOCALHOST_H__

#include <cstdint>
#include <vector>
#include <string>
#include <map>


/**
 *  Class implementing platform neutral abstractions for host related information
 */
class LocalHost {
public:

    typedef struct {
        std::string if_name;
        std::string mac_address;
        std::vector<std::string> ip_addresses;
        std::map<std::string, uint32_t> ip_address_prefix_lengths;
        uint32_t if_index;      // required for ipv6
    } InterfaceInfo;

private:

    std::string hostname;
    std::vector<std::string> local_ip_addresses;
    std::vector<std::string> local_ipv4_addresses;
    std::vector<std::string> local_ipv6_addresses;
    std::vector<InterfaceInfo> local_interface_infos;

public:

    LocalHost(void);
    ~LocalHost(void);

    // getter and setters for cached hostname
    const std::string &getHostname(void) const;
    void cacheHostname(const std::string &hostname);

    // getter and setters for cached interface names
    const std::vector<std::string> &getLocalIPAddresses(void) const;
    const std::vector<std::string> getLocalIPv4Addresses(void) const;
    const std::vector<std::string> getLocalIPv6Addresses(void) const;
    void cacheLocalIPAddresses(const std::vector<std::string> &interfaces);
    const std::string getMatchingLocalIPAddress(std::string ip_address) const;

    // getter and setters for cached interface informations
    const std::vector<InterfaceInfo> &getLocalInterfaceInfos(void) const;
    void cacheLocalInterfaceInfos(const std::vector<InterfaceInfo>  &addresses);
    const std::string getMacAddress(const std::string &local_ip_address) const;
    const std::string getInterfaceName(const std::string &local_ip_address) const;
    const uint32_t getInterfaceIndex(const std::string &local_ip_address) const;
    const uint32_t getInterfacePrefixLength(const std::string& local_ip_address) const;

    // query functions to obtain non-cached information from the operating system
    static const std::string queryHostname(void);
    static std::vector<std::string> queryLocalIPAddresses(void);
    static std::vector<InterfaceInfo> queryLocalInterfaceInfos(void);

    // platform neutral sleep
    static void sleep(uint32_t millis);

    // platform neutral get tick count method
    static uint64_t getTickCountInMs(void);

    // platform neutral get unix epoch time in ms
    static uint64_t getUnixEpochTimeInMs(void);
};

#endif
