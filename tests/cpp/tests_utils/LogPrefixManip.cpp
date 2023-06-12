#include "LogPrefixManip.h"

std::string RemovePrefixFromLineMessage(const std::string& msg) {
  // format of prefix :  "[XXXX][XXXX]"

  auto to_search = ']';
  auto pos = msg.find(to_search);
  if (pos != std::string::npos) {
    pos = msg.find(to_search, pos + 1);
  }
  if (pos != std::string::npos) {
    return msg.substr(pos + 1);
  } else {
    return msg;
  }
}

std::string RemovePrefixFromMessage(std::stringstream& message) {
  std::string message_without_prefix = "";
  std::string line;
  while (std::getline(message, line)) {
    message_without_prefix += RemovePrefixFromLineMessage(line) + "\n";
  }
  return message_without_prefix;
}