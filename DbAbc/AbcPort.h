#ifndef abcport_h
#define abcport_h

#ifdef _MSC_VER
#define snprintf _snprintf
#define _CRT_SECURE_NO_WARNINGS
#elif __APPLE__
#define strnicmp strncasecmp
#define stricmp strcasecmp
#elif __linux__
#define strnicmp strncasecmp
#else
    #error "Unknown platform"
#endif

#endif