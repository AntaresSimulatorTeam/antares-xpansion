#include "LogUtils.h"

#include <string.h>
#define INFO_BUFFER_SIZE 32767

std::string LogUtils::UserName() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  TCHAR user_name[INFO_BUFFER_SIZE];
  DWORD bufCharCount = INFO_BUFFER_SIZE;

  if (!GetUserName(user_name, &bufCharCount)) {
    strcpy_s(user_name, "Unidentified user");
  }
  std::string s;
  return user_name;
#else
  char user_name[LOGIN_NAME_MAX];
  if (!getlogin_r(user_name, LOGIN_NAME_MAX)) {
    strcpy_s(user_name, "Unidentified user");
  }
  return user_name;
#endif
}
std::string LogUtils::HostName() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  TCHAR host_name[INFO_BUFFER_SIZE];
  DWORD bufCharCount = INFO_BUFFER_SIZE;

  if (!GetComputerName(host_name, &bufCharCount)) {
    strcpy_s(host_name, "Unidentified host");
  }
  return host_name;
#else
  char host_name[HOST_NAME_MAX];
  if (!gethostname(host_name, HOST_NAME_MAX)) {
    strcpy_s(host_name, "Unidentified host");
  }
  return host_name;
#endif
}