#ifndef __LIBSPEEDWIRE_SPEEDWIREAUTHENTICATION_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREAUTHENTICATION_HPP__

#include <cstdint>
#include <string>
#include <SpeedwireCommand.hpp>
#include <SpeedwireDevice.hpp>
#include <SpeedwireAuthentication.hpp>

namespace libspeedwire {

    class SpeedwireAuthentication : public SpeedwireCommand {

    public:
        SpeedwireAuthentication(const LocalHost& localhost, const std::vector<SpeedwireDevice>& devices) : SpeedwireCommand(localhost, devices) {}
        ~SpeedwireAuthentication(void) {}

        // synchronous login command methods - send command requests and wait for the response
        bool login(const bool user, const std::string& password, const int timeout_in_ms);
        bool loginAnyToAny(const bool user, const std::string& password, const int timeout_in_ms);
        bool login(const SpeedwireDevice& dst_peer, const bool user, const std::string& password, const int timeout_in_ms = 1000);
        bool login(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src, const bool user, const std::string& password, const int timeout_in_ms = 1000);

        // synchronous logoff command methods - send command requests and wait for the response
        bool logoff(void);
        bool logoffAnyFromAny(void);
        bool logoff(const SpeedwireDevice& dst_peer);
        bool logoff(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src);

        // asynchronous send command methods - send command requests and return immediately
        void sendLogoffRequest(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src);
        SpeedwireCommandTokenIndex sendLoginRequest(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src, const bool user, const std::string& password);
    };

}   // namespace libspeedwire

#endif
