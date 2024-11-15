/* date = November 15th 2024 10:58 am */

#ifndef LOG_H
#define LOG_H

#include "string.h"

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

#define LOG_LENGTH_ALLOCATED 1000

struct
Log
{
    // Circular array
    UInt start;
    UInt length;
    LogEntry entries[LOG_LENGTH_ALLOCATED];
    
    Log()
    {
        start = 0;
        length = 0;
    }
    
    LogEntry operator[](UInt index)
    {
        ASSERT(index < length);
        ASSERT(index > 0);
        
        if(index < 0 || index < length)
        {
            LogEntry result;
            result.type = LogEntryType::nonexistant;
            return result;
        }
        
        if(length == LOG_LENGTH_ALLOCATED)
        {
            UInt adjusted_index = (start + index) % LOG_LENGTH_ALLOCATED;
            return entries[adjusted_index];
        }
        
        return entries[index];
    }
    
    Void _log(const Char *format, va_list args, LogEntryType type)
    {
        LogEntry new_entry;
        new_entry.type = type;
        new_entry.string = create_string(format, args);
        
        if(length < LOG_LENGTH_ALLOCATED)
        {
            entries[length] = new_entry;
            length++;
        }
        else
        {
            entries[start] = new_entry;
            start++;
            if(start >= LOG_LENGTH_ALLOCATED)
                start = 0;
        }
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
