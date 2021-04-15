#include <string>
#include <cstdarg>
#include <Logger.hpp>
#include <LocalHost.hpp>

//#define PRINT_TIMESTAMP


// static initializer
Logger::ListenerEntry* Logger::s_listener = NULL;


/**
 * Constructor for logger instances
 * @param moduleName The name of the related modul - which is printed together with all log messages
 */
Logger::Logger(const char *moduleName)
: m_module_name( moduleName )
{
    m_module_name_w.resize(m_module_name.length());
    for ( unsigned int i = 0 ; i < m_module_name.length() ; ++i ) {
        m_module_name_w[i] = m_module_name[i];
    }
}


/**
 * Constructor for logger instances
 * @param moduleName The name of the related modul - which is printed together with all log messages
 */
Logger::Logger(const std::string &moduleName)
: m_module_name( moduleName )
{
    m_module_name_w.resize(m_module_name.length());
    for ( unsigned int i = 0 ; i < m_module_name.length() ; ++i ) {
        m_module_name_w[i] = m_module_name[i];
    }
}

/**
 * Add a log listener to the Logger.
 * The log listener is added globally and affects the output of all locally declared Logger instances.
 * @param listener A pointer to an object instance implementing the interface ILogListener
 * @param level The log levels that are sent to the log listener
 */
void Logger::setLogListener(ILogListener *listener, LogLevel level) {
    ListenerEntry *entry = new ListenerEntry();
    entry->listener = listener;
    entry->level    = level;
    if( s_listener != NULL ) delete s_listener;
    s_listener = entry;
}


/**
 * Print a log message, where the output is in strings of char's.
 * @param level The log levels that are sent to the log listener
 * @param format The standard printf format string
 * @param ... A variable argument list
 */
void Logger::print(LogLevel level, const char* format, ... )
{
    std::string text;

  #ifdef PRINT_TIMESTAMP
    uint64_t time = LocalHost::getUnixEpochTimeInMs();
    char strTime[32];
    snprintf(strTime, sizeof(strTime), "%015llu.%03u ", time/1000, (unsigned)(time%1000));
    text.append(strTime);
  #endif

    switch (level) {
    case LogLevel::LOG_ERROR:
        text.append("ERROR:   ");
        break;
    case LogLevel::LOG_WARNING:
        text.append("WARNING: ");
        break;
    case LogLevel::LOG_INFO_0:
    case LogLevel::LOG_INFO_1:
    case LogLevel::LOG_INFO_2:
    case LogLevel::LOG_INFO_3:
        text.append("INFO:    ");
        break;
    default:
        text.append("UNKNOWN: ");
        break;
    }
    
    text.append(m_module_name);
    text.append(": ");

    char cbuf[8000];
    va_list list;
    va_start(list, format);
    vsnprintf(cbuf, sizeof(cbuf), format, list);
    va_end(list);
    
    text.append(cbuf);
    if ( text[text.size()-1] != '\n' ) {
        text.append("\n");
    }

    if( s_listener != NULL && s_listener->listener != NULL ) {
        if( (level & s_listener->level) != 0 ) {
            s_listener->listener->log_msg(text, level);
        }
    }
    else {
        fputs(text.c_str(), stderr);
    }
}


/**
 * Print a log message, where the output is in strings of wchar's.
 * @param level The log levels that are sent to the log listener
 * @param format The standard printf format string
 * @param ... A variable argument list
 */
void Logger::print(LogLevel level, const wchar_t* format, ... )
{
    std::wstring text;

  #ifdef PRINT_TIMESTAMP
    uint64_t time = LocalHost::getUnixEpochTimeInMs();
    wchar_t wstrTime[32];
    swprintf(wstrTime, sizeof(wstrTime)/sizeof(wchar_t), L"%015llu.%03u ", time/1000, (unsigned)(time%1000));
    text.append(wstrTime);
  #endif

    switch (level) {
    case LogLevel::LOG_ERROR:
        text.append(L"ERROR:   ");
        break;
    case LogLevel::LOG_WARNING:
        text.append(L"WARNING: ");
        break;
    case LogLevel::LOG_INFO_0:
    case LogLevel::LOG_INFO_1:
    case LogLevel::LOG_INFO_2:
    case LogLevel::LOG_INFO_3:
        text.append(L"INFO:    ");
        break;
    default:
        text.append(L"UNKNOWN: ");
        break;
    }
    
    text.append(m_module_name_w);
    text.append(L": ");

    wchar_t cbuf[8000];
    va_list list;
    va_start(list, format);
    vswprintf(cbuf, sizeof(cbuf)/sizeof(cbuf[0]), format, list);
    va_end(list);
    
    text.append(cbuf);
    if ( text[text.size()-1] != '\n' ) {
        text.append(L"\n");
    }

    if( s_listener != NULL && s_listener->listener != NULL ) {
        if( (level & s_listener->level) != 0 ) {
            s_listener->listener->log_msg_w(text, level);
        }
    }
    else {
        fwprintf(stderr, text.c_str());
    }

}
