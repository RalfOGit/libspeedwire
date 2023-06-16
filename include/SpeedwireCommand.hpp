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
        COMMAND_AC_QUERY = 0x51000200,
        COMMAND_STATUS_QUERY = 0x51800200,
        COMMAND_DEVICE_QUERY = 0x58000200,
        COMMAND_DC_QUERY = 0x53800200
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

    protected:
        static constexpr uint16_t local_susy_id = 0x007d;         // arbitrarily choosen local susy id
        static constexpr uint32_t local_serial_id = 0x3a28be42;     // arbitrarily choosen local serial number

        const LocalHost& localhost;
        const std::vector<SpeedwireInfo>& devices;
        std::vector<SpeedwireSocket> sockets;

        typedef int SocketIndex;
        typedef std::map<std::string, SocketIndex> SocketMap;
        SocketMap socket_map;

        uint16_t packet_id;

        // query tokens are used to match inverter command requests with their responses
        SpeedwireCommandTokenRepository token_repository;

    public:
        SpeedwireCommand(const LocalHost& localhost, const std::vector<SpeedwireInfo>& devices);
        ~SpeedwireCommand(void);

        // synchronous command methods - send command requests and wait for the response
        bool    login(const SpeedwireInfo& peer, const bool user, const char* password, const int timeout_in_ms = 1000);
        bool    logoff(const SpeedwireInfo& peer);
        int32_t query(const SpeedwireInfo& peer, const Command command, const uint32_t first_register, const uint32_t last_register, void* udp_buffer, const size_t udp_buffer_size, const int timeout_in_ms = 1000);

        // asynchronous send command methods - send command requests and return immediately
        void sendLogoffRequest(const SpeedwireInfo& peer);
        SpeedwireCommandTokenIndex sendLoginRequest(const SpeedwireInfo& peer, const bool user, const char* password);
        SpeedwireCommandTokenIndex sendQueryRequest(const SpeedwireInfo& peer, const Command command, const uint32_t first_register, const uint32_t last_register);

        void queryDeviceType(const SpeedwireInfo& peer);

        // synchronous receive method - receive command reply packet for the given command token; this method will block until the packet is received or it times out
        // (for asynchronous receive handling, see class SpeedwireReceiveDispatcher)
        int32_t receiveResponse(const SpeedwireCommandTokenIndex index, SpeedwireSocket& socket, void* udp_buffer, const size_t udp_buffer_size, const int poll_timeout_in_ms);

        // find SpeedwireCommandToken for the reply packet
        int findCommandToken(const SpeedwireHeader& speedwire_packet) const;   // convenience method => returns index

        // check reply packet for correctness
        static bool checkReply(const SpeedwireHeader& speedwire_packet, const struct sockaddr& recvfrom, const SpeedwireCommandToken& token);
        bool checkReply(const SpeedwireHeader& speedwire_packet, const struct sockaddr& recvfrom) const;

        // getter methods for local susy id, serial number and token repository
        uint16_t getLocalSusyID(void) const { return local_susy_id; }
        uint32_t getLocalSerialNumber(void) const { return local_serial_id; }
        SpeedwireCommandTokenRepository& getTokenRepository(void);
    };

}   // namespace libspeedwire

#endif
