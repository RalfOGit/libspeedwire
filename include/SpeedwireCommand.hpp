#ifndef __LIBSPEEDWIRE_SPEEDWIRECOMMAND_HPP__
#define __LIBSPEEDWIRE_SPEEDWIRECOMMAND_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireData.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireSocket.hpp>

namespace libspeedwire {

    enum Command : uint32_t {
        COMMAND_AC_QUERY              = 0x51000200,
        COMMAND_AC_QUERY_2            = 0x51020200,
        COMMAND_STATUS_QUERY          = 0x51800200,
        COMMAND_TEMPERATURE_QUERY     = 0x52000200,
        COMMAND_DC_QUERY              = 0x53800200,
        COMMAND_DC_QUERY_2            = 0x53820200,
        COMMAND_UNKNOWN               = 0x53400200,
        COMMAND_UNKNOWN_2             = 0x53420200,
        COMMAND_ENERGY_QUERY          = 0x54000200,
        COMMAND_ENERGY_QUERY_2        = 0x54020200,
        COMMAND_DEVICE_QUERY          = 0x58000200,
        COMMAND_DEVICE_QUERY_1        = 0x58010200,
        COMMAND_DEVICE_QUERY_2        = 0x58020200,
        COMMAND_DEVICE_QUERY_3        = 0x58030200,
        COMMAND_YIELD_BY_MINUTE_QUERY = 0x70000200,  // query yield in 5 minute intervals
        COMMAND_YIELD_BY_DAY_QUERY    = 0x70200200,  // query yield in 24 hour intervals
        COMMAND_EVENT_QUERY           = 0x70100200   // query events
    };


    /**
     *  Struct SpeedwireCommandToken is used to match command replies with their corresponding command queries.
     */
    typedef struct {
        uint16_t    susyid;             //!< Susyid of the speedwire device the query was send to
        uint32_t    serialnumber;       //!< Serial number of the speedwire device the query was send to
        uint16_t    packetid;           //!< Packet identifier of the query packet
        std::string peer_ip_address;    //!< IP address of the speedwire device the query was send to
        uint32_t    command;            //!< Command identifier of the query
        uint32_t    create_time;        //!< Creation time of the query as lower 32-bit of unix epoch timestamp
    } SpeedwireCommandToken;


    /**
     *  Class SpeedwireCommandTokenRepository holds SpeedwireCommandTokens from when the command is send
     *  to the peer until the corresponding reply is received.
     */
    typedef int SpeedwireCommandTokenIndex;

    class SpeedwireCommandTokenRepository {
    public:
        SpeedwireCommandTokenIndex add(const uint16_t susyid, const uint32_t serialnumber, const uint16_t packetid, const std::string& peer_ip_address, const uint32_t command);
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
