#include "LogUtils.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif
#define INFO_BUFFER_SIZE 32767

std::string LogUtils::UserName() {
  std::string user_name("Unidentified user");
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  TCHAR tchar_user_name[INFO_BUFFER_SIZE];
  DWORD bufCharCount = INFO_BUFFER_SIZE;

  if (GetUserName(tchar_user_name, &bufCharCount)) {
    user_name = tchar_user_name;
  }
#else
  char char_user_name[LOGIN_NAME_MAX];
  if (getlogin_r(char_user_name, LOGIN_NAME_MAX)) {
    user_name = char_user_name;
  }
#endif
  return user_name;
}
std::string LogUtils::HostName() {
  std::string host_name("Unidentified host");
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  TCHAR tchar_host_name[INFO_BUFFER_SIZE];
  DWORD bufCharCount = INFO_BUFFER_SIZE;

  if (GetComputerName(tchar_host_name, &bufCharCount)) {
    host_name = tchar_host_name;
  }
#else
  char char_host_name[INFO_BUFFER_SIZE];
  if (gethostname(char_host_name, HOST_NAME_MAX)) {
    host_name = char_host_name;
  }
#endif
  return host_name;
}