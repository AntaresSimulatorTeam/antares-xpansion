#include "antares-xpansion/benders/benders_core/IterationDataHttpSender.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

IterationDataHttpSender::IterationDataHttpSender(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

IterationDataHttpSender::~IterationDataHttpSender() { Disconnect(); }

bool IterationDataHttpSender::Connect() {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    std::cerr << "[IterationDataHttpSender] socket() failed: " << strerror(errno) << "\n";
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) {
    std::cerr << "[IterationDataHttpSender] invalid address: " << host_ << "\n";
    Disconnect();
    return false;
  }

  if (connect(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    std::cerr << "[IterationDataHttpSender] connect() failed: " << strerror(errno) << "\n";
    Disconnect();
    return false;
  }

  return true;
}

void IterationDataHttpSender::Disconnect() {
  if (socket_fd_ >= 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
}

bool IterationDataHttpSender::Send(int iteration, double lb, double ub, double best_ub) {
  std::ostringstream json;
  json << R"({"iteration":)" << iteration << R"(,"lb":)" << lb << R"(,"ub":)" << ub
       << R"(,"best_ub":)" << best_ub << "}";
  return PostJson("/get_data", json.str());
}

bool IterationDataHttpSender::SendSubproblemNames(const std::vector<std::string>& names) {
  std::ostringstream json;
  json << "[";
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0) {
      json << ",";
    }
    json << '"';
    for (const char c : names[i]) {
      if (c == '"' || c == '\\') {
        json << '\\';
      }
      json << c;
    }
    json << '"';
  }
  json << "]";
  return PostJson("/set", json.str());
}

bool IterationDataHttpSender::PostJson(const std::string& path, const std::string& body) {
  if (!Connect()) {
    return false;
  }

  std::ostringstream http;
  http << "POST " << path << " HTTP/1.1\r\n"
       << "Host: " << host_ << ":" << port_ << "\r\n"
       << "Content-Type: application/json\r\n"
       << "Content-Length: " << body.size() << "\r\n"
       << "Connection: close\r\n"
       << "\r\n"
       << body;
  std::string request = http.str();

  ssize_t sent = send(socket_fd_, request.c_str(), request.size(), 0);
  Disconnect();

  if (sent < 0) {
    std::cerr << "[IterationDataHttpSender] send() failed: " << strerror(errno) << "\n";
    return false;
  }

  return true;
}
