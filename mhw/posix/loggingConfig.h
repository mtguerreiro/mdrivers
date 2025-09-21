#ifndef MHW_LOGGING_CONFIG_H_
#define MHW_LOGGING_CONFIG_H_

#include "clogging/logging_levels.h"

/* Logging configuration for the Demo. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "MHW"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "clogging/logging_stack.h"


#endif /* MHW_LOGGING_CONFIG_H_ */
