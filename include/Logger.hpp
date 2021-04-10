#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <string>
#include <deque>

enum class LogLevel {
    LOG_ERROR   = 0x01,
    LOG_WARNING = 0x02,
    LOG_INFO_0  = 0x04,     // verbose level 0
    LOG_INFO_1  = 0x08,     // verbose level 1
    LOG_INFO_2  = 0x10,     // verbose level 2
    LOG_INFO_3  = 0x20      // verbose level 3
};

inline LogLevel operator|(const LogLevel& op1, const LogLevel& op2) {
    return (LogLevel)((int)op1 | (int)op2);
}

inline LogLevel operator&(const LogLevel& op1, const LogLevel& op2) {
    return (LogLevel)((int)op1 & (int)op2);
}

inline bool operator!=(const LogLevel& op1, const int& op2) {
    return ((int)op1 != (int)op2);
}


class ILogListener
{
public:
    virtual ~ILogListener() {}
    virtual void log_msg(const std::string& msg, const LogLevel &level) = 0;
    virtual void log_msg_w(const std::wstring& msg, const LogLevel &level) = 0;
};


class Logger
{
public:
    // the different possible logging levels

    Logger(const char *moduleName);
    Logger(const std::string &moduleName);

    static void setLogListener(ILogListener *listener, LogLevel level);

    void print(LogLevel level, const char   * format, ... );
    void print(LogLevel level, const wchar_t* format, ... );

private:
    typedef struct {
        ILogListener   *listener;
        LogLevel        level;
    } ListenerEntry;

    static ListenerEntry* s_listener;
    std::string           m_module_name;
    std::wstring          m_module_name_w;
};


#endif
