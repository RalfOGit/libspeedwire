#include <SpeedwireSocketFactory.hpp>


//! The static instance variable
SpeedwireSocketFactory* SpeedwireSocketFactory::instance = NULL;


/**
 * Singleton get instance method using the default strategy for obtaining sockets from the operating system.
 * The implementation may use different strategies depending on the underlying operating system.
 * @param localhost Reference to a LocalHost instance.
 */
SpeedwireSocketFactory* SpeedwireSocketFactory::getInstance(const LocalHost& localhost) {
    // choose a socket strategy depending on the host operating system
#ifdef _WIN32
    // for windows hosts, the following strategies will work
    //Strategy strategy = SocketStrategy::ONE_SOCKET_FOR_EACH_INTERFACE;
    SocketStrategy strategy = SocketStrategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE;
    //Strategy strategy = SocketStrategy::ONE_SINGLE_SOCKET;
#else 
    // for linux hosts, the following strategies will work
    Strategy strategy = SocketStrategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE;
    //Strategy strategy = SocketStrategy::ONE_SINGLE_SOCKET;
#endif
    return getInstance(localhost, strategy);
}


/**
 * Singleton get instance method using the given strategy for obtaining sockets from the operating system.
 * @param localhost Reference to a LocalHost instance.
 * @param strategy The strategy to use for obtaining sockets from the OS.
 */
SpeedwireSocketFactory* SpeedwireSocketFactory::getInstance(const LocalHost& localhost, const SocketStrategy strategy) {
    if (instance == NULL) {
        instance = new SpeedwireSocketFactory(localhost, strategy);
    }
    return instance;
}


/**
 * Non-public constructor - depending on the strategy, a set of sockets is created and opened.
 */
SpeedwireSocketFactory::SpeedwireSocketFactory(const LocalHost& _localhost, const SocketStrategy _strategy) : localhost(_localhost), strategy(_strategy) {

    if (strategy == SocketStrategy::ONE_SOCKET_FOR_EACH_INTERFACE) {
        // create one socket for each local interface address; this works for windows hosts
        openSocketForEachInterface((SocketDirection::SEND | SocketDirection::RECV), (SocketType::MULTICAST | SocketType::UNICAST));
    }
    else if (strategy == SocketStrategy::ONE_SINGLE_SOCKET) {
        // create a single socket for all local interfaces
        openSocketForSingleInterface((SocketDirection::SEND | SocketDirection::RECV), (SocketType::MULTICAST | SocketType::UNICAST), "0.0.0.0");
    }
    else if (strategy == SocketStrategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE) {
        // create one unicast socket for each local interface address
        openSocketForEachInterface((SocketDirection::SEND | SocketDirection::RECV), SocketType::UNICAST);
        // create a single socket for multicast
        openSocketForSingleInterface((SocketDirection::SEND | SocketDirection::RECV), (SocketType::MULTICAST | SocketType::UNICAST), "0.0.0.0");
    }
    else if (strategy == SocketStrategy::ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE) {
        // create one unicast socket for each local interface address
        openSocketForEachInterface((SocketDirection::SEND | SocketDirection::RECV), SocketType::UNICAST);
    }
}


/**
 * Destructor - close all sockets.
 */
SpeedwireSocketFactory::~SpeedwireSocketFactory(void) {
    for (auto& entry : sockets) {
        entry.socket.closeSocket();
    }
    sockets.clear();
}


/**
 *  Open a socket with the given characteristics for for the given local interface.
 */
bool SpeedwireSocketFactory::openSocketForSingleInterface(const SocketDirection direction, const SocketType type, const std::string& interface_address) {
    // create a single socket for multicast
    SocketEntry entry(localhost);
    if (entry.socket.openSocket(interface_address, (type & SocketType::MULTICAST) != 0) < 0) {
        perror("cannot open recv socket instance");
        return false;
    }
    entry.direction = direction;
    entry.type = type;
    entry.interface_address = interface_address;
    sockets.push_back(entry);
    return true;
}


/**
 *  Open a socket with the given characteristics for each local interface.
 */
bool SpeedwireSocketFactory::openSocketForEachInterface(const SocketDirection direction, const SocketType type) {
    bool result = true;
    // loop across all local interfaces
    const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
    for (auto& local_ip : localIPs) {
        // open socket for local ip address
        if (openSocketForSingleInterface(direction, type, local_ip) == false) {
            result = false;
        }
    }
    return result;
}


/**
 *  Get a suitable socket for sending to the given interface ip address.
 */
SpeedwireSocket& SpeedwireSocketFactory::getSendSocket(const SocketType type, const std::string& if_addr) {
    // first try to find an interface specific socket
    if (if_addr != "0.0.0.0") {
        for (auto& entry : sockets) {
            if ((entry.direction & SocketDirection::SEND) != 0 && (entry.type & type) == type) {
                if (entry.interface_address == if_addr) {
                    return entry.socket;
                }
            }
        }
    }
    // try to find an INADDR_ANY socket
    for (auto& entry : sockets) {
        if ((entry.direction & SocketDirection::SEND) != 0 && (entry.type & type) == type) {
            if (entry.interface_address == "0.0.0.0") {
                return entry.socket;
            }
        }
    }
    perror("cannot find any suitable socket");
    return sockets[0].socket;
}


/**
 *  Get a suitable socket for receiving from the given interface ip address.
 */
SpeedwireSocket& SpeedwireSocketFactory::getRecvSocket(const SocketType type, const std::string& if_addr) {
    if (if_addr != "0.0.0.0") {
        // first try to find an interface and cast specific socket
        for (auto& entry : sockets) {
            if ((entry.direction & SocketDirection::RECV) != 0 && (entry.type & type) == type) {
                if (entry.interface_address == if_addr) {
                    return entry.socket;
                }
            }
        }
        // then try to find an interface specific socket
        for (auto& entry : sockets) {
            if ((entry.direction & SocketDirection::RECV) != 0 && (entry.type & type) != 0) {
                if (entry.interface_address == if_addr) {
                    return entry.socket;
                }
            }
        }
    }
    // try to find an INADDR_ANY socket
    for (auto& entry : sockets) {
        if ((entry.direction & SocketDirection::RECV) != 0 && (entry.type & type) == type) {
            if (entry.interface_address == "0.0.0.0") {
                return entry.socket;
            }
        }
    }
    perror("cannot find any suitable socket");
    return sockets[0].socket;
}


/**
 * Get a vector of suitable sockets for receiving from the given vector of interface ip addresses; this is useful
 * in combination with poll() calls.
 */
std::vector<SpeedwireSocket> SpeedwireSocketFactory::getRecvSockets(const SocketType type, const std::vector<std::string>& if_addresses) {
    std::vector<SpeedwireSocket> recv_sockets;
    if ((type & SocketType::MULTICAST) == type && strategy == SocketStrategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE) {
        SpeedwireSocket& socket = getRecvSocket(SocketType::MULTICAST, "0.0.0.0");
        recv_sockets.push_back(socket);
        return recv_sockets;
    }
    if ((type & SocketType::UNICAST) != 0) {
        for (auto& addr : if_addresses) {
            SpeedwireSocket& socket = getRecvSocket(SocketType::UNICAST, addr);
            bool duplicate = false;
            for (auto& recv_socket : recv_sockets) {
                if (socket.getSocketFd() == recv_socket.getSocketFd()) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate == false) {
                recv_sockets.push_back(socket);
            }
        }
    }
    if ((type & SocketType::MULTICAST) != 0) {
        for (auto& addr : if_addresses) {
            SpeedwireSocket& socket = getRecvSocket(SocketType::MULTICAST, addr);
            bool duplicate = false;
            for (auto& recv_socket : recv_sockets) {
                if (socket.getSocketFd() == recv_socket.getSocketFd()) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate == false) {
                recv_sockets.push_back(socket);
            }
        }
    }
    if ((type & SocketType::ANYCAST) != 0) {
        for (auto& addr : if_addresses) {
            SpeedwireSocket& socket = getRecvSocket(SocketType::ANYCAST, addr);
            bool duplicate = false;
            for (auto& recv_socket : recv_sockets) {
                if (socket.getSocketFd() == recv_socket.getSocketFd()) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate == false) {
                recv_sockets.push_back(socket);
            }
        }
    }
    return recv_sockets;
}
