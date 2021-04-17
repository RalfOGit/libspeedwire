#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <string>
#include <deque>

/*! \file */
/**
 *   Enumeration describing the defined log levels.
 */
enum class LogLevel {
    LOG_ERROR   = 0x01,     /**< error level     */
    LOG_WARNING = 0x02,     /**< warning level   */
    LOG_INFO_0  = 0x04,     /**< verbose level 0 */
    LOG_INFO_1  = 0x08,     /**< verbose level 1 */
    LOG_INFO_2  = 0x10,     /**< verbose level 2 */
    LOG_INFO_3  = 0x20,     /**< verbose level 3 */
};


/** Global scope operator for bitwise or'ing two LogLevel enum values */
inline LogLevel operator|(const LogLevel& op1, const LogLevel& op2) {
    return (LogLevel)((int)op1 | (int)op2);
}

/** Global scope operator for bitwise and'ing two LogLevel enum values */
inline LogLevel operator&(const LogLevel& op1, const LogLevel& op2) {
    return (LogLevel)((int)op1 & (int)op2);
}

/** Global scope operator for not equal comparison of two LogLevel enum values */
inline bool operator!=(const LogLevel& op1, const int& op2) {
    return ((int)op1 != (int)op2);
}


/**
 *  Interface for routing log messages created by class Logger.
 *  Classes implementing this interface can route loger output to e.g. stdout, stderr, files, ...
 */
class ILogListener {

public:
    /** Virtual destructor */
    virtual ~ILogListener(void) {}

    /**
     *  Method to output a single byte character message.
     *  @param msg The message string
     *  @param level The log level of the message string
     */
    virtual void log_msg(const std::string& msg, const LogLevel &level) = 0;

   /**
     *  Method to output a wide character message.
     *  @param msg The message string
     *  @param level The log level of the message string
     */
    virtual void log_msg_w(const std::wstring& msg, const LogLevel &level) = 0;
};


/**
 *  Logger class.
 *  An instance of this classed can be instanciated for each module, for instance by declaring a static instance with local scope inside
 *  the source code of the module. The class supports different log levels (see class LogLevel) and the registration of log listeners
 *  (see interface ILogListener). Log listeners can route log output to different output means and also limit the output to the 
 *  log levels defined during registration.
 */
class Logger {

public:
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
