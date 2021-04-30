#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#define LOG_ON 1
#define LOG(fmt, ...) \
        do { if (LOG_ON) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#endif
