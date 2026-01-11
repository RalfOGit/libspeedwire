#ifndef __LIBSPEEDWIRE_SPEEDWIREAUTHENTICATION_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREAUTHENTICATION_HPP__

#include <cstdint>
#include <string>
#include <map>
#include <array>
#include <SpeedwireCommand.hpp>
#include <SpeedwireDevice.hpp>
#include <SpeedwireAuthentication.hpp>

namespace libspeedwire {

    /**
     *  Speedwire user names to grant access to SMA devices with different permissions.
     */
    enum class UserCode : uint8_t {
        USER      = 0x7,    // 00000111
        INSTALLER = 0xa     // 00001010
      //SERVICE   = ??,
      //DEVELOPER = ??
    };


    /**
     *  Class encapsulating a speedwire credential consisting of a user name and password.
     */
    class Credentials : std::pair<UserCode, std::string> {
    public:
        Credentials(UserCode code, const std::string& password) : std::pair<UserCode, std::string>(code, password) {}
        Credentials(void) : Credentials((UserCode)0, "") {}
        UserCode getUserCode(void) const { return first; }
        const std::string& getPassword(void) const { return second; }
        std::array<uint8_t, 12> getEncodedPassword(void) const;
    };


    /**
     *  Class encapsulating a map of speedwire credentials.
     *  It holds pairs of user names and passwords in an std::map. A default user can be defined to simplify access.
     */
    class CredentialsMap : std::map<UserCode, std::string> {
    protected:
        static UserCode default_user;

    public:
        CredentialsMap(void) {
            add(UserCode::USER,      "0000");  // standard sma user password
            add(UserCode::INSTALLER, "1111");  // standard sma installer password
        }
        void add(UserCode name, const std::string& password) {
            operator[](name) = password;
        }
        const Credentials get(UserCode name) const {
            static const std::string empty;
            const auto it = find(name);
            if (it != end()) {
                return Credentials(it->first, it->second);
            }
            return Credentials(name, empty);
        }
        int readFromFile(const std::string& path);

        static UserCode getDefaultUserCode(void) { return default_user; }
        static void setDefaultUserCode(UserCode code) { default_user = code; }
        Credentials getDefaultCredentials(void) const { return get(default_user); }
    };


    /**
     *  Class encapsulating methods for speedwire device login and logoff.
     */
    class SpeedwireAuthentication : public SpeedwireCommand {

    public:
        SpeedwireAuthentication(const LocalHost& localhost, const std::vector<SpeedwireDevice>& devices) : SpeedwireCommand(localhost, devices) {}
        ~SpeedwireAuthentication(void) {}

        // synchronous login command methods - send command requests and wait for the response
        bool login(const Credentials& credentials, const int timeout_in_ms);
        bool loginAnyToAny(const Credentials& credentials, const int timeout_in_ms);
        bool login(const SpeedwireDevice& dst_peer, const Credentials& credentials, const int timeout_in_ms = 1000);
        bool login(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src, const Credentials& credentials, const int timeout_in_ms = 1000);

        // synchronous logoff command methods - send command requests and wait for the response
        bool logoff(void);
        bool logoffAnyFromAny(void);
        bool logoff(const SpeedwireDevice& dst_peer);
        bool logoff(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src);

        // asynchronous send command methods - send command requests and return immediately
        SpeedwireCommandTokenIndex sendLoginRequest(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src, const Credentials& credentials);
        bool sendLogoffRequest(const std::string& if_address, const SpeedwireAddress& dst, const SpeedwireAddress& src);
    };

}   // namespace libspeedwire

#endif
