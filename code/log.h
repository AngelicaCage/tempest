/* date = November 15th 2024 10:58 am */

#ifndef LOG_H
#define LOG_H

#include "string.h"
#include "list.h"

enum class LogEntryType
{
    nonexistant, // error code, for if the log entry cannot be found
    info,
    warning,
    error,
    user,
};

struct LogEntry
{
    LogEntryType type;
    String string;
};

struct
Log
{
    InplaceCircularArray<LogEntry, 1000> entries;
    // Circular array
#if 0
    UInt start;
    UInt length;
    LogEntry entries[LOG_LENGTH_ALLOCATED];
#endif
    
#if 0
    LogEntry entry_at(UInt index)
    {
        if(index < 0 || index >= entries.length)
        {
            LogEntry result;
            result.type = LogEntryType::nonexistant;
            return result;
        }
        
        return entries[index];
    }
#endif
    
    LogEntry entry_at(UInt index)
    {
        if(index < 0 || index >= entries.length)
        {
            LogEntry result;
            result.type = LogEntryType::nonexistant;
            return result;
        }
        
        return entries[index];
    }
    
    Void _log(const Char *format, va_list args, LogEntryType type)
    {
        LogEntry new_entry;
        new_entry.type = type;
        new_entry.string = create_string(format, args);
        
        entries.add(new_entry);
    }
    
    Void log(const Char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _log(format, args, LogEntryType::info);
    }
    
    Void log_warning(const Char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _log(format, args, LogEntryType::warning);
    }
    
    Void log_error(const Char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _log(format, args, LogEntryType::error);
    }
    
};

Log *global_log; // This must be set to reference a log

Void log(const Char *format, ...)
{
    va_list args;
    va_start(args, format);
    global_log->_log(format, args, LogEntryType::info);
}

Void log_warning(const Char *format, ...)
{
    va_list args;
    va_start(args, format);
    global_log->_log(format, args, LogEntryType::warning);
}

Void log_error(const Char *format, ...)
{
    va_list args;
    va_start(args, format);
    global_log->_log(format, args, LogEntryType::error);
}

#endif //LOG_H
