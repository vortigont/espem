// Default config options
// do NOT change anything here, copy 'config.h' into 'user_config.h' and change you options there

#pragma once

#if defined __has_include
#  if __has_include("user_config.h")
#    include "user_config.h"
#  endif
#endif


#define FW_NAME "espem"

// LOG macro's
#if defined(LOG)
#undef LOG
#endif

#if defined(ESPEM_DEBUG)
    #define LOG(func, ...) ESPEM_DEBUG.func(__VA_ARGS__)
#else
    #define LOG(func, ...) ;
#endif
