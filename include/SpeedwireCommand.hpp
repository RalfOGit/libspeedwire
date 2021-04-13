#ifndef __SPEEDWIRECOMMAND_HPP__
#define __SPEEDWIRECOMMAND_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <SpeedwireHeader.hpp>
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireData.hpp>


enum Command : uint32_t {
    COMMAND_AC_QUERY     = 0x51000200,
    COMMAND_STATUS_QUERY = 0x51800200,
    COMMAND_DEVICE_QUERY = 0x58000200,
    COMMAND_DC_QUERY     = 0x53800200
};


/**
 *  Struct SpeedwireCommandToken is used to match command replies with their corresponding command queries.
 */
typedef struct {
    uint16_t    susyid;
    uint32_t    serialnumber;
    uint16_t    packetid;
    std::string peer_ip_address;
    uint32_t    command;
    uint32_t    create_time;
} SpeedwireCommandToken;


/**
 *  Class SpeedwireCommandTokenRepository holds SpeedwireCommandTokens from when the command is send 
 *  to the peer until the corresponding reply is received.
 */
class SpeedwireCommandTokenRepository {
public:
    void add (const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid, const std::string& peer_ip_address, const uint32_t command);
    int  find(const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid);
    void remove(const int index);
    void clear(void);
    int  expire(const int timeout_in_ms);
    SpeedwireCommandToken& at(const int index);
    int  size(void);
    bool needs_login;

    SpeedwireCommandTokenRepository(void) { needs_login = false;  }

protected:
    std::vector<SpeedwireCommandToken> token;
};


/**
 *  Class SpeedwireCommand holds functionality to send commands to peers and to check a reply packet for validity
 */
class SpeedwireCommand {

protected:
    static constexpr uint16_t local_susy_id   = 0x007d;         // arbitrarily choosen susy id
    static constexpr uint32_t local_serial_id = 0x3a28be42;     // arbitrarily choosen serial number

    const LocalHost& localhost;
    const std::vector<SpeedwireInfo> &devices;
    std::vector<SpeedwireSocket> sockets;

    typedef int SocketIndex;
    typedef std::map<std::string, SocketIndex> SocketMap;
    SocketMap socket_map;

    uint16_t packet_id;

    // query tokens are used to match inverter command requests with their responses
    SpeedwireCommandTokenRepository token_repository;

public:
    SpeedwireCommand(const LocalHost &localhost, const std::vector<SpeedwireInfo> &devices);
    ~SpeedwireCommand(void);

    // commands
    int32_t login (const SpeedwireInfo& peer, const bool user, const char* password);
    int32_t logoff(const SpeedwireInfo& peer);
    int32_t query (const SpeedwireInfo& peer, const Command command, const uint32_t first_register, const uint32_t last_register);

    // find SpeedwireCommandToken for the reply packet
    int findCommandToken(SpeedwireHeader& speedwire_packet);   // convenience method returns index
    SpeedwireCommandTokenRepository& getTokenRepository(void);

    // check reply packet for correctness
    static bool checkReply(SpeedwireHeader& speedwire_packet, const struct sockaddr& recvfrom, const SpeedwireCommandToken& token);
    bool checkReply(SpeedwireHeader& speedwire_packet, const struct sockaddr& recvfrom);

    // get local susy id and serial number
    uint16_t getLocalSusyID      (void) const { return local_susy_id; }
    uint32_t getLocalSerialNumber(void) const { return local_serial_id; }
};

#endif
