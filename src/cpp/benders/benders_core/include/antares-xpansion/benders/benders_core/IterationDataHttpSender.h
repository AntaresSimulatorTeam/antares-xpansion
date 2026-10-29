#pragma once

#include <string>
#include <vector>

class IterationDataHttpSender {
 public:
  IterationDataHttpSender(std::string host, int port);
  ~IterationDataHttpSender();

  IterationDataHttpSender(const IterationDataHttpSender&) = delete;
  IterationDataHttpSender& operator=(const IterationDataHttpSender&) = delete;

  bool Send(int iteration, double lb, double ub, double best_ub);
  bool SendSubproblemNames(const std::vector<std::string>& names);

 private:
  bool Connect();
  void Disconnect();
  bool PostJson(const std::string& path, const std::string& body);

  std::string host_;
  int port_;
  int socket_fd_ = -1;
};
