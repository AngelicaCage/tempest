/* date = November 15th 2024 7:33 pm */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#define check(condition) if((condition) == NULL) { log_warning("Check failed at %s, %d, %s", __FILE__, __LINE__, __func__); ASSERT(false)};
#define check_vital(condition) if((condition) == NULL) { log_error("Check failed at %s, %d, %s", __FILE__, __LINE__, __func__); ASSERT(false)};

#endif //DIAGNOSTICS_H
