#ifndef __LIBSPEEDWIRE_LOCALHOST_HPP__
#define __LIBSPEEDWIRE_LOCALHOST_HPP__

#ifdef ARDUINO
#include <Arduino.h>
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#else
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#endif

namespace libspeedwire {

    /**
     *  Class implementing platform neutral abstractions for host related information
     *
     *  This class provices platform neutral methods to get information about the local host from the operating system.
     *  The information is internally cached within the class. Public getter methods provide access to this information.
     */
    class LocalHost {
    public:

        /// data structure holding information related to one network interface
        typedef struct {
            std::string if_name;                                        /**< name of the network interface */
            std::string mac_address;                                    /**< mac address of the network interface */
            std::vector<std::string> ip_addresses;                      /**< list of ip addresses associated with the network interface */
            std::map<std::string, uint32_t> ip_address_prefix_lengths;  /**< map of network prefixes of the network interface using its ip addresses as key */
            uint32_t if_index;                                          /**< ipv6 interface index of the network interface*/
        } InterfaceInfo;

    protected:

        std::string hostname;
        std::vector<std::string> local_ip_addresses;
        std::vector<std::string> local_ipv4_addresses;
        std::vector<std::string> local_ipv6_addresses;
        std::vector<InterfaceInfo> local_interface_infos;

        LocalHost(void);
        ~LocalHost(void);

        // query functions to obtain non-cached information from the operating system
        static const std::string queryHostname(void);
        static std::vector<std::string> queryLocalIPAddresses(void);
        static std::vector<InterfaceInfo> queryLocalInterfaceInfos(void);

        // setters to cache queried host information into class private storage
        void cacheHostname(const std::string& hostname);
        void cacheLocalIPAddresses(const std::vector<std::string>& interfaces);
        void cacheLocalInterfaceInfos(const std::vector<InterfaceInfo>& addresses);

        static LocalHost* instance;

    public:

        static LocalHost& getInstance(void);

        // getter for hostname
        const std::string& getHostname(void) const;

        // getters for interface names
        const std::vector<std::string>& getLocalIPAddresses(void) const;
        const std::vector<std::string>& getLocalIPv4Addresses(void) const;
        const std::vector<std::string>& getLocalIPv6Addresses(void) const;
        const std::string getMatchingLocalIPAddress(std::string ip_address) const;

        // getters for interface informations
        const std::vector<InterfaceInfo>& getLocalInterfaceInfos(void) const;
        const std::string getMacAddress(const std::string& local_ip_address) const;
        const std::string getInterfaceName(const std::string& local_ip_address) const;
        uint32_t getInterfaceIndex(const std::string& local_ip_address) const;
        uint32_t getInterfacePrefixLength(const std::string& local_ip_address) const;

        // platform neutral sleep
        static void sleep(uint32_t millis);

        // platform neutral get tick count method
        static uint64_t getTickCountInMs(void);

        // platform neutral get unix epoch time in ms
        static uint64_t getUnixEpochTimeInMs(void);

        // platform neutral conversion of unix epoch time in ms to a formatted string
        static std::string unixEpochTimeInMsToString(uint64_t epoch);

        // Calculate the absolute time difference between time1 and time2
        static uint64_t calculateAbsTimeDifference(uint64_t time1, uint64_t time2);

        // hexdump utility method
        static void hexdump(const void* const buff, const unsigned long size);
    };

}   // namespace libspeedwire

#endif
