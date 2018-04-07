#ifndef INC_ALL_H
#define INC_ALL_H

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>
#include <vector>
#include <queue>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef signed int s32;

#define SIM_INFO 0
#define SIM_WARNING 1
#define SIM_ERROR 2
#define SIMLOG(LEVEL, fmt, ...)\
  do {\
    if (SIM_INFO == LEVEL)\
      fprintf(stdout, "INFO:[%s::%d::%s]:" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
    else if (SIM_WARNING == LEVEL)\
      fprintf(stdout, "INFO:[%s::%d::%s]:" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
    else if (SIM_ERROR == LEVEL)\
      fprintf(stderr, "INFO:[%s::%d::%s]:" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
      } while(0)

#endif
