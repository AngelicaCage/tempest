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
    
    Char operator[](U64 index)
    {
        return data[index];
    }
};


String
create_string(const Char *format, va_list args)
{
    Int length = vsnprintf(NULL, 0, format, args);
    
    String result;
    result.length = length;
    result.length_allocated = length + 1;
    result.data = (Char *)mem_alloc(result.length_allocated);
    result.allocated = true;
    
    vsprintf(result.data, format, args);
    
    return result;
}

String
create_string(const Char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    return create_string(format, args);
}

Void
free_string(String *str)
{
    if(str->allocated)
        mem_free(str->data);
}


#endif //STRING_H
