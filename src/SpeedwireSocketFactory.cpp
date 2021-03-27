#include <SpeedwireSocketFactory.hpp>


// the static instance variable
SpeedwireSocketFactory* SpeedwireSocketFactory::instance = NULL;


/**
 *  Get instance
 */
SpeedwireSocketFactory* SpeedwireSocketFactory::getInstance(const LocalHost& localhost) {
    if (instance == NULL) {
        // choose a socket strategy depending on the host operating system
#ifdef _WIN32
        // for windows hosts, the following strategies will work
        Strategy strategy = Strategy::ONE_SOCKET_FOR_EACH_INTERFACE;
        //Strategy strategy = Strategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE;
        //Strategy strategy = Strategy::ONE_SINGLE_SOCKET;
#else 
        // for linux hosts, the following strategies will work
        Strategy strategy = Strategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE;
        //Strategy strategy = Strategy::ONE_SINGLE_SOCKET;
#endif
        instance = new SpeedwireSocketFactory(localhost, strategy);
    }
    return instance;
}


/**
 *  Constructor - depending on the strategy, a set of sockets is created and opened
 */
SpeedwireSocketFactory::SpeedwireSocketFactory(const LocalHost& _localhost, const Strategy _strategy) : localhost(_localhost), strategy(_strategy) {

    if (strategy == Strategy::ONE_SOCKET_FOR_EACH_INTERFACE) {

        // create one socket for each local interface address; this works for windows hosts
        const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
        for (auto& local_ip : localIPs) {

            // open socket for local ip address
            SocketEntry entry(localhost);
            if (entry.socket.openSocket(local_ip) < 0) {
                perror("cannot open recv socket instance");
            }
            else {
                entry.direction = (Direction)(Direction::SEND | Direction::RECV);
                entry.type = (Type)(Type::MULTICAST | Type::UNICAST);
                entry.interface_address = local_ip;
                sockets.push_back(entry);
            }
        }
    }
    else if (strategy == Strategy::ONE_SINGLE_SOCKET) {

        // create a single socket for all local interfaces
        SocketEntry entry(localhost);
        if (entry.socket.openSocket("0.0.0.0") < 0) {
            perror("cannot open recv socket instance");
        }
        else {
            entry.direction = (Direction)(Direction::SEND | Direction::RECV);
            entry.type = (Type)(Type::MULTICAST | Type::UNICAST);
            entry.interface_address = "0.0.0.0";
            sockets.push_back(entry);
        }
    }
    else if (strategy == Strategy::ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE) {

        // create a single socket for multicast reception and one unicast socket for each local interface address
        const std::vector<std::string>& localIPs = localhost.getLocalIPv4Addresses();
        for (auto& local_ip : localIPs) {

            // open socket for local ip address
            SocketEntry entry(localhost);
            if (entry.socket.openSocket(local_ip) < 0) {
                perror("cannot open recv socket instance");
            }
            else {
                entry.direction = (Direction)(Direction::SEND | Direction::RECV);
                entry.type = Type::UNICAST;
                entry.interface_address = local_ip;
                sockets.push_back(entry);
            }
            //SocketEntry entry2(localhost);
            //if (entry2.socket.openSocket(local_ip) < 0) {
            //    perror("cannot open recv socket instance");
            //}
            //else {
            //    entry2.direction = Direction::SEND;
            //    entry2.type = Type::MULTICAST;
            //    entry2.interface_address = local_ip;
            //    sockets.push_back(entry2);
            //}
        }
        SocketEntry entry(localhost);
        if (entry.socket.openSocket("0.0.0.0") < 0) {
            perror("cannot open recv socket instance");
        }
        else {
            entry.direction = (Direction)(Direction::SEND | Direction::RECV);
            entry.type = (Type)(Type::MULTICAST | Type::UNICAST);
            entry.interface_address = "0.0.0.0";
            sockets.push_back(entry);
        }
    }
}


/**
 *  Destructor - close all sockets
 */
SpeedwireSocketFactory::~SpeedwireSocketFactory(void) {
    for (auto& entry : sockets) {
        entry.socket.closeSocket();
    }
    sockets.clear();
}


/**
 *  Get a suitable socket for sending to the given interface ip address
 */
SpeedwireSocket& SpeedwireSocketFactory::getSendSocket(const Type type, const std::string& if_addr) {
    // first try to find an interface specific socket
    if (if_addr != "0.0.0.0") {
        for (auto& entry : sockets) {
            if ((entry.direction & Direction::SEND) != 0 && (entry.type & type) == type) {
                if (entry.interface_address == if_addr) {
                    return entry.socket;
                }
            }
        }
    }
    // try to find an INADDR_ANY socket
    for (auto& entry : sockets) {
        if ((entry.direction & Direction::SEND) != 0 && (entry.type & type) == type) {
            if (entry.interface_address == "0.0.0.0") {
                return entry.socket;
            }
        }
    }
    perror("cannot find any suitable socket");
    return sockets[0].socket;
}


/**
 *  Get a suitable socket for receiving from the given interface ip address
 */
SpeedwireSocket& SpeedwireSocketFactory::getRecvSocket(const Type type, const std::string& if_addr) {
    if (if_addr != "0.0.0.0") {
        // first try to find an interface and cast specific socket
        for (auto& entry : sockets) {
            if ((entry.direction & Direction::RECV) != 0 && (entry.type & type) == type) {
                if (entry.interface_address == if_addr) {
                    return entry.socket;
                }
            }
        }
        // then try to find an interface specific socket
        for (auto& entry : sockets) {
            if ((entry.direction & Direction::RECV) != 0 && (entry.type & type) != 0) {
                if (entry.interface_address == if_addr) {
                    return entry.socket;
                }
            }
        }
    }
    // try to find an INADDR_ANY socket
    for (auto& entry : sockets) {
        if ((entry.direction & Direction::RECV) != 0 && (entry.type & type) == type) {
            if (entry.interface_address == "0.0.0.0") {
                return entry.socket;
            }
        }
    }
    perror("cannot find any suitable socket");
    return sockets[0].socket;
}


/**
 *  Get a vector of suitable sockets for receiving from the given vector of interface ip addresses; this is useful
 * in combination with poll() calls
 */
std::vector<SpeedwireSocket> SpeedwireSocketFactory::getRecvSockets(const Type type, const std::vector<std::string>& if_addresses) {
    std::vector<SpeedwireSocket> recv_sockets;
    if ((type & Type::MULTICAST) == type && strategy == ONE_MULTICAST_SOCKET_AND_ONE_UNICAST_SOCKET_FOR_EACH_INTERFACE) {
        SpeedwireSocket& socket = getRecvSocket(Type::MULTICAST, "0.0.0.0");
        recv_sockets.push_back(socket);
        return recv_sockets;
    }
    if ((type & Type::UNICAST) != 0) {
        for (auto& addr : if_addresses) {
            SpeedwireSocket& socket = getRecvSocket(Type::UNICAST, addr);
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
    if ((type & Type::MULTICAST) != 0) {
        for (auto& addr : if_addresses) {
            SpeedwireSocket& socket = getRecvSocket(Type::MULTICAST, addr);
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
    if ((type & Type::ANYCAST) != 0) {
        for (auto& addr : if_addresses) {
            SpeedwireSocket& socket = getRecvSocket(Type::ANYCAST, addr);
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
