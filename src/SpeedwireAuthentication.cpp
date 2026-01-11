#define _CRT_SECURE_NO_WARNINGS (1)
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <LocalHost.hpp>
#include <AddressConversion.hpp>
#include <Logger.hpp>
#include <SpeedwireByteEncoding.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireTime.hpp>
#include <SpeedwireCommand.hpp>
#include <SpeedwireAuthentication.hpp>
using namespace libspeedwire;

static Logger logger("SpeedwireAuthentication");


/**
 *  Login this local device from to other devices. This is done by sending a broadcast login command for this device to each local interface.
 */
bool SpeedwireAuthentication::login(const Credentials& credentials, const int timeout_in_ms) {
    bool result = true;
    const SpeedwireAddress& local_address     = SpeedwireAddress::getLocalAddress();
    const SpeedwireAddress& broadcast_address = SpeedwireAddress::getBroadcastAddress();
    for (const auto& entry : socket_map) {
        result &= login(entry.first, broadcast_address, local_address, credentials, timeout_in_ms);
    }
    for (const auto& device : devices) {
        if (!AddressConversion::resideOnSameSubnet(device.deviceIpAddress, device.interfaceIpAddress, 24) && device.interfaceIpAddress.length() > 0) { // FIXME: hard coded prefix
            result &= login(device.interfaceIpAddress, device.deviceAddress, local_address, credentials, timeout_in_ms);
        }
    }
    return result;
}

/**
 *  Login all devices to all other devices. This is done by sending a broadcast login command for each device to each local interface.
 */
bool SpeedwireAuthentication::loginAnyToAny(const Credentials& credentials, const int timeout_in_ms) {
    bool result = true;
    const SpeedwireAddress& broadcast_address = SpeedwireAddress::getBroadcastAddress();
    for (const auto& device : devices) {
        for (const auto& entry : socket_map) {
            result &= login(entry.first, broadcast_address, device.deviceAddress, credentials, timeout_in_ms);
        }
    }
    return result;
}

/**
 *  synchronous login method - send inverter login command to the given destination peer, wait for the response and check for error codes
 */
bool SpeedwireAuthentication::login(const SpeedwireDevice& dst_peer, const Credentials& credentials, const int timeout_in_ms) {
    return login(dst_peer.interfaceIpAddress, dst_peer.deviceAddress, SpeedwireAddress::getLocalAddress(), credentials, timeout_in_ms);
}

/**
 *  synchronous login method - send inverter login command to the given peer, wait for the response and check for error codes
 */
bool SpeedwireAuthentication::login(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src, const Credentials& credentials, const int timeout_in_ms) {
    logger.print(LogLevel::LOG_INFO_0, "login susyid %u serial %lu => susyid %u serial %lu time 0x%016llx",
        src.susyID, src.serialNumber, dst.susyID, dst.serialNumber, localhost.getUnixEpochTimeInMs());

    // determine receive socket
    SocketIndex socket_index = socket_map[if_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return false;
    }
    SpeedwireSocket& socket = sockets[socket_index];

    // send login request to peer
    SpeedwireCommandTokenIndex token_index = sendLoginRequest(if_address, dst, src, credentials);
    if (token_index < 0) {
        return false;
    }

    // wait for the response
    unsigned char response_buffer[2048];
    int32_t nbytes = receiveResponse(token_index, socket, response_buffer, sizeof(response_buffer), timeout_in_ms);
    if (nbytes <= 0) {
        return false;
    }

    // check error code
    const SpeedwireHeader speedwire_packet(response_buffer, sizeof(response_buffer));
    if (speedwire_packet.isValidData2Packet()) {
        const SpeedwireData2Packet data2_packet(speedwire_packet);

        if (data2_packet.isInverterProtocolID()) {
            const SpeedwireInverterProtocol inverter_packet(data2_packet);
//          logger.print(LogLevel::LOG_INFO_1, "%s\n", inverter_packet.toString().c_str());


            uint16_t error_code = inverter_packet.getErrorCode();
            if (error_code != 0x0000) {
                if (error_code == 0x0017) {
                    logger.print(LogLevel::LOG_ERROR, "lost connection - not authenticated (error code 0x0017)");
                    token_repository.needs_login = true;
                }
                else if (error_code == 0x0100) {
                    logger.print(LogLevel::LOG_ERROR, "invalid password - not authenticated");
                }
                else if (token_repository.size() > token_index && token_repository.at(token_index).command == Command::LOGIN) { // login command
                    logger.print(LogLevel::LOG_ERROR, "login failure - not authenticated");
                }
                else {
                    logger.print(LogLevel::LOG_ERROR, "query error code received");
                }
                token_repository.remove(token_index);
                return false;
            }
        }
    }
    token_repository.remove(token_index);
    return true;
}


/**
 *  Logoff this local device from all other devices. This is done by sending a broadcast logoff command for this device to each local interface.
 */
bool SpeedwireAuthentication::logoff(void) {
    bool result = true;
    const SpeedwireAddress &local_address     = SpeedwireAddress::getLocalAddress();
    const SpeedwireAddress &broadcast_address = SpeedwireAddress::getBroadcastAddress();
    for (const auto& entry : socket_map) {
        result &= logoff(entry.first, broadcast_address, local_address);
    }
    for (const auto& device : devices) {
        if (!AddressConversion::resideOnSameSubnet(device.deviceIpAddress, device.interfaceIpAddress, 24) && device.interfaceIpAddress.length() > 0) { // FIXME: hard coded prefix
            result &= logoff(device.interfaceIpAddress, device.deviceAddress, local_address);
        }
    }
    return result;
}

/**
 *  Logoff all devices from all other devices. This is done by sending a broadcast logoff command for each device to each local interface.
 */
bool SpeedwireAuthentication::logoffAnyFromAny(void) {
    bool result = true;
    const SpeedwireAddress& broadcast_address = SpeedwireAddress::getBroadcastAddress();
    for (const auto& device : devices) {
        for (const auto& entry : socket_map) {
            result &= logoff(entry.first, broadcast_address, device.deviceAddress);
        }
    }
    return result;
}

/**
 *  synchronous logoff method - send inverter logoff command to the given peer and return; there is no reply from the inverter for logoff commands
 */
bool SpeedwireAuthentication::logoff(const SpeedwireDevice& dst) {
    return logoff(dst.interfaceIpAddress, dst.deviceAddress, SpeedwireAddress::getLocalAddress());
}

/**
 *  Logoff - send inverter logoff command to the given peer and return; there is no reply from the inverter for logoff commands
 */
bool SpeedwireAuthentication::logoff(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src) {
    logger.print(LogLevel::LOG_INFO_0, "logoff susyid %u serial %lu => susyid %u serial %lu time 0x%016llx",
        src.susyID, src.serialNumber, dst.susyID, dst.serialNumber, localhost.getUnixEpochTimeInMs());

    return sendLogoffRequest(if_address, dst, src);
}


/**
 *  Send inverter login command to the given peer
 */
SpeedwireCommandTokenIndex SpeedwireAuthentication::sendLoginRequest(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src, const Credentials& credentials) {
    // Request  534d4100000402a000000001003a0010 60650ea0 7a01842a71b30001 7d0042be283a0001 000000000280 0c04fdff 07000000 84030000 00d8e85f 00000000 c1c1c1c18888888888888888 00000000   => login command = 0xfffd040c, first = 0x00000007 (user 7, installer a), last = 0x00000384 (hier timeout), time = 0x5fdf9ae8, 0x00000000, pw 12 bytes
    // Response 534d4100000402a000000001002e0010 60650be0 7d0042be283a0001 7a01842a71b30001 000000000280 0d04fdff 07000000 84030000 00d8e85f 00000000 00000000 => login OK
    // Response 534d4100000402a000000001002e0010 60650be0 7d0042be283a0001 7a01842a71b30001 000100000280 0d04fdff 07000000 84030000 fddbe85f 00000000 00000000 => login INVALID PASSWORD
    // command  0xfffd040c => 0x400 set?  0x00c bytecount=12?
    // assemble unicast device login packet
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4 + 4 + 4 + 12 + 4];
    memset(request_buffer, 0, sizeof(request_buffer));

    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer) - 20, SpeedwireData2Packet::sma_inverter_protocol_id);   // 4 + 8 + 4 => es fehlen 4 byte an der Länge, prüfen of tagLength erst nach dem control word zählt
    //LocalHost::hexdump(request_buffer, sizeof(request_buffer));

    SpeedwireData2Packet data2_packet(request_header);
    data2_packet.setControl(0xa0);

    const uint16_t packet_id = getIncrementedPacketID();

    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(dst.susyID);
    request.setDstSerialNumber(dst.serialNumber);
    request.setDstControl(0x0100);
    request.setSrcSusyID(src.susyID);
    request.setSrcSerialNumber(src.serialNumber);
    request.setSrcControl(0x0100);
    request.setErrorCode(0);
    request.setFragmentCounter(0);
    request.setPacketID(packet_id);
    request.setCommandID(Command::LOGIN);
    request.setFirstRegisterID((uint32_t)credentials.getUserCode());    // user: 0x7  installer: 0xa
    request.setLastRegisterID(0x00000384);     // timeout
    request.setDataUint32(0, SpeedwireTime::getInverterTimeNow());
    request.setDataUint32(4, 0x00000000);
    std::array<uint8_t, 12> encoded_password = credentials.getEncodedPassword();
    request.setDataUint8Array(8, encoded_password.data(), (unsigned long)encoded_password.size());

    // identify the socket to be used
    SocketIndex socket_index = socket_map[if_address];
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return -1;
    }
    SpeedwireSocket& socket = sockets[socket_index];

    // identify the destination ip address to be used
    std::string dst_ip_address = AddressConversion::toString(socket.getSpeedwireMulticastIn4Address().sin_addr);
    if (dst.isBroadcast() == false) {
        for (const auto& device : devices) {
            if (device.deviceAddress == dst) {
                dst_ip_address = device.deviceIpAddress;
            }
        }
    }

    // send login request packet to peer
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), dst_ip_address);
    if (nsent <= 0) {
        logger.print(LogLevel::LOG_ERROR, "cannot send data to socket");
        return -1;
    }

    // add a query token; this is used to match reply packets to this request packet
    SpeedwireCommandTokenIndex index = token_repository.add(dst.susyID, dst.serialNumber, packet_id, dst_ip_address, Command::LOGIN);

    return index;
}


/**
 *  Send inverter logoff command to the given peer
 */
bool SpeedwireAuthentication::sendLogoffRequest(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src) {
    // Request 534d4100000402a00000000100220010 606508a0 ffffffffffff0003 7d0052be283a0003 000000000280 0e01fdff ffffffff 00000000   => logoff command = 0xfffd01e0 (fehlt hier last?)
    // Request 534d4100000402a00000000100220010 606508a0 ffffffffffff0003 7d0042be283a0003 000000000180 e001fdff ffffffff 00000000
    // assemble unicast device logoff packet
    unsigned char request_buffer[24 + 8 + 8 + 6 + 4 + 4];
    memset(request_buffer, 0, sizeof(request_buffer));

    SpeedwireHeader request_header(request_buffer, sizeof(request_buffer));
    request_header.setDefaultHeader(1, sizeof(request_buffer) - 20, SpeedwireData2Packet::sma_inverter_protocol_id);

    SpeedwireData2Packet data2_packet(request_header);
    data2_packet.setControl(0xa0);

    const uint16_t packet_id = getIncrementedPacketID();

    SpeedwireInverterProtocol request(request_header);
    request.setDstSusyID(dst.susyID);
    request.setDstSerialNumber(dst.serialNumber);
    request.setDstControl(0x0300);
    request.setSrcSusyID(src.susyID);
    request.setSrcSerialNumber(src.serialNumber);
    request.setSrcControl(0x0300);
    request.setErrorCode(0);
    request.setFragmentCounter(0);
    request.setPacketID(packet_id);
    request.setCommandID(Command::LOGOFF);
    request.setFirstRegisterID(0xffffffff);
    request.setLastRegisterID(0x00000000);

    // identify the destination ip address to be used
    SocketIndex socket_index = socket_map.at(if_address);
    if (socket_index < 0) {
        logger.print(LogLevel::LOG_ERROR, "invalid socket_index");
        return false;
    }
    SpeedwireSocket& socket = sockets[socket_index];

    // identify the destination ip address to be used
    std::string dst_ip_address = AddressConversion::toString(socket.getSpeedwireMulticastIn4Address().sin_addr);
    if (dst.isBroadcast() == false) {
        for (const auto& device : devices) {
            if (device.deviceAddress == dst) {
                dst_ip_address = device.deviceIpAddress;
            }
        }
    }

    // send login request packet to peer
    int nsent = socket.sendto(request_buffer, sizeof(request_buffer), dst_ip_address);
    if (nsent <= 0) {
        logger.print(LogLevel::LOG_ERROR, "cannot send data to socket");
        return false;
    }

    return true;
}


/**
 *  Encode password string into its 12 byte binary line encoding.
 */
std::array<uint8_t, 12> Credentials::getEncodedPassword(void) const {
    uint8_t pattern = 0x88;
    switch (getUserCode()) {
    case UserCode::USER:      pattern = 0x88; break;
    case UserCode::INSTALLER: pattern = 0xBB; break;
    }
    const std::string& password = getPassword();
    std::array<uint8_t, 12> encoded_password;
    for (size_t i = 0; i < encoded_password.size(); ++i) {
        encoded_password[i] = (i < password.length() ? password[i] + pattern : pattern);
    }
    return encoded_password;
}


// initialize default user code to UserCode::USER
UserCode CredentialsMap::default_user = UserCode::USER;


/**
 *  Read credentials from input file.
 *  Credentials are encoded similar to http basic authentication: user=password, installer=password.
 *  @param path input file path
 *  @return number of credentials in credentials map after the read operation.
 */
int CredentialsMap::readFromFile(const std::string& path) {

    FILE* inp = fopen(path.c_str(), "r");
    if (inp == NULL) {
        printf("Could not open credentials input file \"%s\"\n", path.c_str());
        return -1;
    }

    char buffer[1024] = { 0 };

    while (fgets(buffer, sizeof(buffer), inp) != NULL) {
        std::string line(buffer);

        // remove comments, empty lines and trailing \r or \n
        size_t hashpos = line.find_first_of("#\r\n");
        if (hashpos != std::string::npos) {
            line.erase(hashpos);
        }

        if (line.length() > 0) {
            std::string username;
            std::string password;

            // split line at first '='
            std::string::size_type pos = line.find('=');
            if (pos != std::string::npos) {
                username = line.substr(0, pos);
                password = line.substr(pos + 1);
                if (username == "user") {
                    add(UserCode::USER, password);
                }
                else if (username == "installer") {
                    add(UserCode::INSTALLER, password);
                }
                else {
                    unsigned int code;
                    if (sscanf(username.c_str(), " %u", &code) > 0) {
                        add((UserCode)code, password);
                    }
                    else{
                        printf("Unknown username \"%s\" in credentials input file \"%s\"\n", username.c_str(), path.c_str());
                    }
                }
            }
        }
    }
    fclose(inp);

    return (int)size();
}
