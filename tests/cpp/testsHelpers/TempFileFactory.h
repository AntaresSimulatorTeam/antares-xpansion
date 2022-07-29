#ifndef _MKTEMPFACTORY_H
#define _MKTEMPFACTORY_H
#include <stdlib.h>
int mktemp_platform(char *nameTemplate, size_t sizeInChars = 0) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return _mktemp_s(nameTemplate, sizeInChars);

#else  // defined(__unix__) || (__APPLE__)
  return mkstemps(nameTemplate, sizeInChars);
#endif
}
#endif  //_MKTEMPFACTORY_H