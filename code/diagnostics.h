/* date = November 15th 2024 7:33 pm */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

inline Void
_check(Bool condition, const Char *file, Int line, const Char *function)
{
    if(!condition)
    {
        log_warning("Check failed at %s, %d, %s", file, line, function);
        ASSERT(false);
    }
}

inline Void
_check_vital(Bool condition, const Char *message_if_false)
{
    if(!condition)
    {
        log_warning(message_if_false);
        ASSERT(false);
    }
}


#define check(condition) _check((condition) != NULL, __FILE__, __LINE__, __func__);



#endif //DIAGNOSTICS_H
