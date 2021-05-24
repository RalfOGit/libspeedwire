#ifndef __LIBSPEEDWIRE_SPEEDWIRESOCKETFACTORY_HPP__
#define __LIBSPEEDWIRE_SPEEDWIRESOCKETFACTORY_HPP__

#include <string>
#include <vector>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>

namespace libspeedwire {

    /**
     *  Class implementing a platform neutral factory for sockets.
     *  This is meant to deal with implementation incompatibilities between socket implementations
     *  in different operating systems.
     */
    class SpeedwireSocketFactory {

    public:

        //! Enumeration of send or receive direction that the socket is to be used for.
        enum class SocketDirection {
            NONE = 0x0,                     //!< Direction is unspecified.
            SEND = 0x1,                     //!< Send direction only.
            RECV = 0x2,                     //!< Receive direction only.
            ALL_DIRECTIONS = SEND | RECV    //!< Both send and receive direction.
        };

        //! Enumeration of packet type that the socket is to be used for.
        enum class SocketType {
            NOCAST = 0x0,                //!< Packet type is not specified.
            UNICAST = 0x1,                //!< Unicast packets only.
            MULTICAST = 0x2,                //!< Multicast packets only.
            ANYCAST = UNICAST | MULTICAST //!< Both unicast and multicast packets.
        };

        //! Enumeration of the socket creation strategies.
        enum class SocketStrategy {
            //! One socket is created for each local interface; it is then used for both directions and for both unicast and multicast.
            ONE_SOCKET_FOR_EACH_INTERFACE,
            //! One single socket is created to be used for all local interfaces, both directions and both unicast and multicast.
            ONE_SINGLE_SOCKET,
            //! One single multicasts socket socket is created for all local interfaces and one unicast socket is created for each local interface and for both unicast and multicast.
            ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE,
            //! One unicast socket is created for each local interface; it is used for both directions.
            ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE
        };

    protected:

        //! Object holding the properties of a single socket created by the constructor.
        class SocketEntry {
        public:
            SocketDirection direction;                  //!< Send or receive direction that the socket is to be used for.
            SocketType      type;                       //!< Packet type that the socket is to be used for.
            std::string     interface_address;          //!< Local interface address that the socket is opened on.
            SpeedwireSocket socket;                     //!< SpeedwireSocket instance.
            SocketEntry(const LocalHost& localhost) : direction(SocketDirection::NONE), type(SocketType::NOCAST), interface_address(), socket(localhost) {};
        };

        static SpeedwireSocketFactory* instance;        //!< The static singleton instance.
        std::vector<SocketEntry> sockets;               //!< Vector of SocketEntry instances created by the constructor.
        const LocalHost& localhost;                     //!< Reference to LocalHost instance.
        SocketStrategy strategy;                        //!< Socket creation strategy provided to the getInstance method.

        SpeedwireSocketFactory(const LocalHost& localhost, const SocketStrategy strategy);
        ~SpeedwireSocketFactory(void);

        bool openSocketForSingleInterface(const SocketDirection direction, const SocketType type, const std::string& interface_address);
        bool openSocketForEachInterface(const SocketDirection direction, const SocketType type);

    public:
        static SpeedwireSocketFactory* getInstance(const LocalHost& localhost);
        static SpeedwireSocketFactory* getInstance(const LocalHost& localhost, const SocketStrategy strategy);

        SpeedwireSocket& getSendSocket(const SocketType type, const std::string& if_addr);
        SpeedwireSocket& getRecvSocket(const SocketType type, const std::string& if_addr);
        std::vector<SpeedwireSocket> getRecvSockets(const SocketType type, const std::vector<std::string>& if_addresses);
    };


    //! Bitwise or operator for SpeedwireSocketFactory::SocketDirection.
    inline SpeedwireSocketFactory::SocketDirection operator|(const SpeedwireSocketFactory::SocketDirection& op1, const SpeedwireSocketFactory::SocketDirection& op2) {
        return (SpeedwireSocketFactory::SocketDirection)((int)op1 | (int)op2);
    }

    //! Bitwise and operator for SpeedwireSocketFactory::SocketDirection.
    inline SpeedwireSocketFactory::SocketDirection operator&(const SpeedwireSocketFactory::SocketDirection& op1, const SpeedwireSocketFactory::SocketDirection& op2) {
        return (SpeedwireSocketFactory::SocketDirection)((int)op1 & (int)op2);
    }

    //! Not equal operator taking a SpeedwireSocketFactory::SocketDirection and an integer.
    inline bool operator!=(const SpeedwireSocketFactory::SocketDirection& op1, const int op2) {
        return ((int)op1 != op2);
    }

    //! Bitwise or operator for SpeedwireSocketFactory::SocketType.
    inline SpeedwireSocketFactory::SocketType operator|(const SpeedwireSocketFactory::SocketType& op1, const SpeedwireSocketFactory::SocketType& op2) {
        return (SpeedwireSocketFactory::SocketType)((int)op1 | (int)op2);
    }

    //! Bitwise and operator for SpeedwireSocketFactory::SocketType.
    inline SpeedwireSocketFactory::SocketType operator&(const SpeedwireSocketFactory::SocketType& op1, const SpeedwireSocketFactory::SocketType& op2) {
        return (SpeedwireSocketFactory::SocketType)((int)op1 & (int)op2);
    }

    //! Not equal operator taking a SpeedwireSocketFactory::SocketType and an integer.
    inline bool operator!=(const SpeedwireSocketFactory::SocketType& op1, const int op2) {
        return ((int)op1 != op2);
    }

}   // namespace libspeedwire

#endif
