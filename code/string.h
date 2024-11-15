/* date = November 15th 2024 0:02 pm */

#ifndef STRING_H
#define STRING_H


struct
String
{
    Char *data;
    Int length;
    Int length_allocated;
    Bool allocated = false;
};

String
create_string(const Char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    Int length = vsnprintf(NULL, 0, format, args);
    
    String result;
    result.length = length;
    result.length_allocated = length;
    result.data = (Char *)alloc(length);
    result.allocated = true;
    
    vsprintf(result.data, format, args);
    
    return result;
}

String
create_string(const Char *format, va_list args)
{
    Int length = vsnprintf(NULL, 0, format, args);
    
    String result;
    result.length = length;
    result.length_allocated = length;
    result.data = (Char *)alloc(length);
    result.allocated = true;
    
    vsprintf(result.data, format, args);
    
    return result;
}

#endif //STRING_H
