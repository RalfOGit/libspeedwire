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
    enum class UserName : uint8_t {
        USER      = 0x7,    // 00000111
        INSTALLER = 0xa     // 00001010
    };


    /**
     *  Class encapsulating a speedwire credential consisting of a user name and password.
     */
    class Credentials : std::pair<UserName, std::string> {
    public:
        Credentials(UserName name, const std::string& password) : std::pair<UserName, std::string>(name, password) {}
        Credentials(void) : Credentials((UserName)0, "") {}
        UserName getUserName(void) const { return first; }
        std::string getPassWord(void) const { return second; }
        std::array<uint8_t, 12> getEncodedPassWord(void) const;
    };


    /**
     *  Class encapsulating a map of speedwire credentials.
     *  It holds pairs of user names and passwords in an std::map. A default user can be defined to simplify access.
     */
    class CredentialsMap : std::map<UserName, std::string> {
    protected:
        static UserName default_user_name;

    public:
        CredentialsMap(void) {
            add(UserName::USER,      "0000");  // standard sma user password
            add(UserName::INSTALLER, "1111");  // standard sma installer password
        }
        void add(UserName name, const std::string& password) {
            operator[](name) = password;
        }
        const Credentials get(UserName name) const {
            static const std::string empty;
            const auto it = find(name);
            if (it != end()) {
                return Credentials(it->first, it->second);
            }
            return Credentials(name, empty);
        }
        int readFromFile(const std::string& path);

        static UserName getDefaultUserName(void) { return default_user_name; }
        static void setDefaultUserName(UserName name) { default_user_name = name; }
        Credentials getDefaultCredentials(void) const { return get(default_user_name); }
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
