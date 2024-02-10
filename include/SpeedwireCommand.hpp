#ifndef __LIBSPEEDWIRE_SPEEDWIRECOMMAND_HPP__
#define __LIBSPEEDWIRE_SPEEDWIRECOMMAND_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireSocket.hpp>

namespace libspeedwire {

    enum class Command : uint32_t {
        NONE                  = 0x00000000,

        ID_MASK               = 0xfffc0000,  // just a guess
        COMPONENT_MASK        = 0x00030000,  // just a guess
        RW_MASK               = 0x0000ff00,  // just a guess
        REQUEST_TYPE_MASK     = 0x000000ff,  // just a guess

        DISCOVERY             = 0x00000000,
        AC                    = 0x51000000,
        STATUS                = 0x51800000,
        TEMPERATURE           = 0x52000000,
        ID_UNKNOWN            = 0x53400000,
        DC                    = 0x53800000,
        ENERGY                = 0x54000000,
        DEVICE                = 0x58000000,
        YIELD_BY_MINUTE       = 0x70000000,
        EVENT                 = 0x70100000,
        YIELD_BY_DAY          = 0x70200000,
        AUTHENTICATION        = 0xfffc0000,

        COMPONENT_0           = 0x00000000,
        COMPONENT_1           = 0x00010000,
        COMPONENT_2           = 0x00020000,
        COMPONENT_3           = 0x00030000,

        WRITE                 = 0x00000100,
        READ                  = 0x00000200,
        RW_LOGIN              = 0x00000400,     // used for login

        QUERY_REQUEST         = 0x00000000,     // 0x00 <> 00000000
        QUERY_RESPONSE        = 0x00000001,     // 0x01 <> 00000001
        UPDATE_RESPONSE       = 0x0000000a,     // 0x0a <> 00001010
        LOGIN_REQUEST         = 0x0000000c,     // 0x0c <> 00001100
        LOGIN_RESPONSE        = 0x0000000d,     // 0x0c <> 00001101
        UPDATE_REQUEST        = 0x0000000e,     // 0x0e <> 00001110
        LOGOFF_REQUEST        = 0x000000e0,     // 0xe0 <> 11100000

        AC_QUERY              = AC              | COMPONENT_0 | READ,   // 0x51000200
        STATUS_QUERY          = STATUS          | COMPONENT_0 | READ,   // 0x51800200
        TEMPERATURE_QUERY     = TEMPERATURE     | COMPONENT_0 | READ,   // 0x52000200
        DC_QUERY              = DC              | COMPONENT_0 | READ,   // 0x53800200
        UNKNOWN               = ID_UNKNOWN      | COMPONENT_0 | READ,   // 0x53400200
        ENERGY_QUERY          = ENERGY          | COMPONENT_0 | READ,   // 0x54000200
        DEVICE_QUERY          = DEVICE          | COMPONENT_0 | READ,   // 0x58000200
        YIELD_BY_MINUTE_QUERY = YIELD_BY_MINUTE | COMPONENT_0 | READ,   // 0x70000200 - query yield in 5 minute intervals
        YIELD_BY_DAY_QUERY    = YIELD_BY_DAY    | COMPONENT_0 | READ,   // 0x70200200 - query yield in 24 hour intervals
        EVENT_QUERY           = EVENT           | COMPONENT_0 | READ,   // 0x70100200 - query events

        LOGIN                 = AUTHENTICATION  | COMPONENT_1 | RW_LOGIN | 0x0c,    // 0xfffd040c
        LOGOFF                = AUTHENTICATION  | COMPONENT_1 | WRITE    | 0xe0,    // 0xfffd01e0

        DEVICE_WRITE          = DEVICE          | COMPONENT_0 | WRITE,  // 0x58000100
    };

    static Command operator|(Command lhs, Command rhs) { return (Command)(((uint32_t)lhs) | ((uint32_t)rhs)); }
    static Command operator&(Command lhs, Command rhs) { return (Command)(((uint32_t)lhs) & ((uint32_t)rhs)); }
    static Command operator~(Command rhs) { return (Command)~((uint32_t)rhs); }
    static bool   operator==(Command lhs, Command rhs) { return (((uint32_t)lhs) == ((uint32_t)rhs)); }


    /**
     *  Struct SpeedwireCommandToken is used to match command replies with their corresponding command queries.
     */
    typedef struct {
        uint16_t    susyid;             //!< Susyid of the speedwire device the query was send to
        uint32_t    serialnumber;       //!< Serial number of the speedwire device the query was send to
        uint16_t    packetid;           //!< Packet identifier of the query packet
        std::string peer_ip_address;    //!< IP address of the speedwire device the query was send to
        Command     command;            //!< Command identifier of the query
        uint32_t    create_time;        //!< Creation time of the query as lower 32-bit of unix epoch timestamp
    } SpeedwireCommandToken;


    /**
     *  Class SpeedwireCommandTokenRepository holds SpeedwireCommandTokens from when the command is send
     *  to the peer until the corresponding reply is received.
     */
    typedef int SpeedwireCommandTokenIndex;

    class SpeedwireCommandTokenRepository {
    public:
        SpeedwireCommandTokenIndex add(const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid, const std::string& peer_ip_address, const Command command);
        int  find(const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid) const;
        void remove(const SpeedwireCommandTokenIndex index);
        void clear(void);
        int  expire(const int timeout_in_ms);
        const SpeedwireCommandToken& at(const SpeedwireCommandTokenIndex index) const;
        int  size(void) const;
        bool needs_login;

        SpeedwireCommandTokenRepository(void) { needs_login = false; }

    protected:
        std::vector<SpeedwireCommandToken> token;
    };



    /**
     *  Class SpeedwireCommand holds functionality to send commands to peers and to check a reply packet for validity
     */
    class SpeedwireCommand {
    public:
        typedef int SocketIndex;
        typedef std::map<std::string, SocketIndex> SocketMap;

    protected:
        const LocalHost& localhost;
        const std::vector<SpeedwireDevice>& devices;
        std::vector<SpeedwireSocket> sockets;
        SocketMap socket_map;

        static uint16_t packet_id;

        // query tokens are used to match inverter command requests with their responses
        SpeedwireCommandTokenRepository token_repository;

    public:
        SpeedwireCommand(const LocalHost& localhost, const std::vector<SpeedwireDevice>& devices);
        ~SpeedwireCommand(void);

        // synchronous command methods - send command requests and wait for the response
        int32_t query(const SpeedwireDevice& peer, const Command command, const uint32_t first_register, const uint32_t last_register, void* udp_buffer, const size_t udp_buffer_size, const int timeout_in_ms = 1000);
        SpeedwireDevice queryDeviceType(const SpeedwireDevice& peer, const int timeout_in_ms = 1000);

        // asynchronous send command method - send command requests and return immediately
        SpeedwireCommandTokenIndex sendQueryRequest(const SpeedwireDevice& peer, const Command command, const uint32_t first_register, const uint32_t last_register);

        // synchronous receive method - receive command reply packet for the given command token; this method will block until the packet is received or it times out
        // (for asynchronous receive handling, see class SpeedwireReceiveDispatcher)
        int32_t receiveResponse(const SpeedwireCommandTokenIndex index, SpeedwireSocket& socket, void* udp_buffer, const size_t udp_buffer_size, const int poll_timeout_in_ms);

        // find SpeedwireCommandToken for the reply packet
        int findCommandToken(const SpeedwireHeader& speedwire_packet) const;   // convenience method => returns index

        // check reply packet for correctness
        bool checkReply(const SpeedwireHeader& speedwire_packet, const struct sockaddr& recvfrom, const SpeedwireCommandToken& token) const;
        bool checkReply(const SpeedwireHeader& speedwire_packet, const struct sockaddr& recvfrom) const;

        // get token repository
        SpeedwireCommandTokenRepository& getTokenRepository(void);

        // get socket map
        const SocketMap& getSocketMap(void) const { return socket_map; }

        // increment packet id and return it
        static uint16_t getIncrementedPacketID(void) {
            packet_id = (packet_id + 1) | 0x8000;
            return packet_id;
        }
    };

}   // namespace libspeedwire

#endif
