#ifndef LAS_C_MACRO_H
#define LAS_C_MACRO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LAS_DEBUG_ASSERTIONS

#define LAS_DEBUG_ASSERTIONS 1

#endif

#define LAS_ASSERT(condition)                                                                      \
    if (!(condition))                                                                              \
    {                                                                                              \
        fprintf(stderr,                                                                            \
                "%s::%d::%s: condition `%s` failed\n",                                             \
                __FILE__,                                                                          \
                __LINE__,                                                                          \
                __func__,                                                                          \
                #condition);                                                                       \
        abort();                                                                                   \
    }

#define LAS_ASSERT_M(condition, format, ...)                                                       \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            fprintf(stderr,                                                                        \
                    "%s::%d::%s: condition `%s` failed.\n" format "\n",                            \
                    __FILE__,                                                                      \
                    __LINE__,                                                                      \
                    __func__,                                                                      \
                    #condition,                                                                    \
                    ##__VA_ARGS__);                                                                \
            abort();                                                                               \
        }                                                                                          \
    } while (0)

#if LAS_DEBUG_ASSERTIONS == 1

#define LAS_DEBUG_ASSERT(condition) LAS_ASSERT(condition)
#define LAS_DEBUG_ASSERT_NOT_NULL(ptr) LAS_ASSERT((ptr) != NULL)
#define LAS_DEBUG_ASSERT_M(condition, message, ...) LAS_ASSERT_M(condition, message, ##__VA_ARGS__)

#else

#define LAS_DEBUG_ASSERT(condition)
#define LAS_DEBUG_ASSERT_M(condition, message)
#define LAS_DEBUG_ASSERT_NOT_NULL

#endif

#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

#endif // LAS_C_MACRO_H
