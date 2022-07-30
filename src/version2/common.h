#ifndef __COMMON_FUNCTIONS_H__
#define __COMMON_FUNCTIONS_H__

#define CREATE_LOG_FN(SELF_TYPEDEF, FN_PREFIX, LOG_PREFIX)\
static inline void FN_PREFIX##_Log(SELF_TYPEDEF, const char *fmt, ...){\
    char buffer[2048];\
    va_list args;\
    va_start(args, fmt);\
    vsnprintf(buffer, 2048, fmt, args);\
    printf("[%s] %s\n", LOG_PREFIX, buffer);\
    va_end(args);\
}\

// Thank you Falco Girgis for this excellent idea.
#define C_PRIVATE_BEGIN(N)  struct N##_private {
#define C_PRIVATE_END       } private;

#endif // __COMMON_FUNCTIONS_H__