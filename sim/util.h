#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>     /* atexit */

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

#endif
