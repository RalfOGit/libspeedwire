#ifndef __SPEEDWIRESOCKETFACTORY_HPP__
#define __SPEEDWIRESOCKETFACTORY_HPP__

#include <string>
#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>


/**
 *  Class implementing a platform neutral factory for sockets
 *  This is meant to deal with implementation incompatibilities between socket implementations
 *  in different operating systems
 */
class SpeedwireSocketFactory {

public:

    typedef enum {
        NONE = 0x0,
        SEND = 0x1,
        RECV = 0x2,
        ALL_DIRECTIONS = 0x3
    } Direction;

    typedef enum {
        NOCAST    = 0x0,
        UNICAST   = 0x1,
        MULTICAST = 0x2,
        ANYCAST   = 0x3
    } Type;

    typedef enum {
        ONE_SOCKET_FOR_EACH_INTERFACE = 1,
        ONE_SINGLE_SOCKET = 2,
        ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE = 3,
        ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE = 4
    } Strategy;

protected:

    class SocketEntry {
    public:
        Direction       direction;
        Type            type;
        std::string     interface_address;
        SpeedwireSocket socket;
        SocketEntry(const LocalHost& localhost) : direction(Direction::NONE), type(Type::NOCAST), interface_address(), socket(localhost) {};
    };

    static SpeedwireSocketFactory* instance;
    std::vector<SocketEntry> sockets;
    const LocalHost& localhost;
    Strategy strategy;

    SpeedwireSocketFactory(const LocalHost& localhost, const Strategy strategy);
    ~SpeedwireSocketFactory(void);

    bool openSocketForSingleInterface(const Direction direction, const Type type, const std::string& interface_address);
    bool openSocketForEachInterface(const Direction direction, const Type type);

public:
    static SpeedwireSocketFactory* getInstance(const LocalHost& localhost);
    static SpeedwireSocketFactory* getInstance(const LocalHost& localhost, const Strategy strategy);

    SpeedwireSocket& getSendSocket(const Type type, const std::string& if_addr);
    SpeedwireSocket& getRecvSocket(const Type type, const std::string& if_addr);
    std::vector<SpeedwireSocket> getRecvSockets(const Type type, const std::vector<std::string> &if_addresses);
};

#endif
