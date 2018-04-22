#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>     /* atexit */
#include <cassert>

typedef signed long long s64;
typedef unsigned long long u64;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u8;

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

template <typename T> class Singleton {
public:
 static T* &get_instance() {
   if (_instance == 0) {
     _instance = new T;
     atexit(del);
   }

   return _instance;
 }

private:
 static void del() {
   delete _instance;
 }

 Singleton(const Singleton &other);
 Singleton &operator=(const Singleton &other);
 Singleton();
 ~Singleton();

 static T* _instance;
};

template <typename T>
T* Singleton <T>::_instance = NULL;

extern const u64 MACHINE_WORD_SIZE;
extern const u64 MAX_SETS_SIZE;
extern const u64 MAX_BLOCK_SIZE;

inline bool check_addr_valid(u64 addr) {
  assert(MACHINE_WORD_SIZE > 0);
  // addr >> 64 is not permitted, so do the shift twice
  addr = addr >> 1;
  return (addr >> (MACHINE_WORD_SIZE-1)) == 0;
}

inline bool is_power_of_two(u64 num) {
  return (num & (num - 1)) == 0;
}

inline u32 len_of_binary(u64 num) {
  assert(is_power_of_two(num));
  int ret = 0;
  while (num) {
    num = num >> 1;
    ret++;
  }
  return ret-1;
}

#endif
