#ifndef debug_h__included
#define debug_h__included

#include <c_types.h>
#include <stdio.h>

// Debug levels
#define TRACE 0
#define DEBUG 1
#define INFO 2
#define WARN 3
#define ERROR 4
#define FATAL 5

// minimum level of messages to print,
// may be overridden by defining it before including the header
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL TRACE
#endif

// Debug logging macro:
// Usage: LOG(INFO, "Hello %s", name)
//
// You may leave out any parameters after _format to just log a string
// a newline is appended automatically
#define LOG(_level, _format, args...) { \
    if (_level >= DEBUG_LEVEL) { \
        static const char flash_str[] ICACHE_RODATA_ATTR STORE_ATTR = _format "\n";  \
        printf(flash_str, ## args); \
    } \
}


// Log a JSON object
// Usage: LOG_JSON(INFO, "MyObject", myObject)
//
// this pretty prints the json object and logs it
#define LOG_JSON(_level, _msg, _json_obj, args...) { \
    if (_level >= DEBUG_LEVEL) { \
        static const char flash_str[] ICACHE_RODATA_ATTR STORE_ATTR = _msg ": %s\n"; \
        char *out = cJSON_Print(_json_obj); \
        printf(flash_str, ## args, out); \
        free(out); \
    } \
}

#if DEBUG_LEVEL <= DEBUG
#define LOG_HEAP(_msg) { \
    static const char flash_str[] ICACHE_RODATA_ATTR STORE_ATTR = _msg " - Free heap: %d\n"; \
    printf(flash_str, system_get_free_heap_size()); \
}   
#else
#define LOG_HEAP(_msg) {}
#endif

#define HEXDUMP(_level, _title, _data, _len) { \
    if (_level >= DEBUG_LEVEL) { \
        os_printf(_title "\n"); \
        for (uint16_t i = 0; i < (_len); i++) { \
            os_printf("%02x ", (_data)[i]); \
            if ((i + 1) % 16 == 0) { \
                os_printf("\n"); \
            } \
        } \
        if ((_len) % 16 != 0) { \
            os_printf("\n"); \
        } \
    } \
}

#endif