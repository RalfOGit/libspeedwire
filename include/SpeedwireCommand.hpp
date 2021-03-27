#ifndef __SPEEDWIRECOMMAND_HPP__
#define __SPEEDWIRECOMMAND_HPP__

#include <cstdint>
#include <string>
#include <map>
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireData.hpp>


enum Command : uint32_t {
    COMMAND_AC_QUERY     = 0x51000200,
    COMMAND_STATUS_QUERY = 0x51800200,
    COMMAND_DEVICE_QUERY = 0x58000200,
    COMMAND_DC_QUERY     = 0x53800200
};


class SpeedwireCommand {

protected:
    static const uint16_t local_susy_id;
    static const uint32_t local_serial_id;

    const LocalHost& localhost;
    const std::vector<SpeedwireInfo> &devices;
    std::vector<SpeedwireSocket> sockets;

    typedef int SocketIndex;
    typedef std::map<std::string, SocketIndex> SocketMap;
    SocketMap socket_map;

    uint16_t packet_id;

public:


    SpeedwireCommand(const LocalHost &localhost, const std::vector<SpeedwireInfo> &devices);
    ~SpeedwireCommand(void);

    // commands
    int32_t login(const SpeedwireInfo& peer, const bool user, const char* password);
    int32_t logoff(const SpeedwireInfo& peer);
    int32_t query(const SpeedwireInfo& peer, const Command command, const uint32_t first_register, const uint32_t last_register, std::vector<SpeedwireRawData> &data);

    // receive a reply packet from the given peer (or timeout)
    int recvReply(const SpeedwireInfo& peer, void* buff, const size_t buff_size, const unsigned long timeout_in_ms, struct sockaddr& recvfrom) const;

    // check reply packet for correctness
    bool checkReply(const SpeedwireInfo& peer, const uint16_t packet_id, const struct sockaddr& recvfrom, const void *const buff, const size_t buff_size) const;
};

#endif
