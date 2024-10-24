#pragma once
namespace Outerloop {
class OuterLoop {
 public:
  virtual void Run();
  virtual void OuterLoopCheckFeasibility() = 0;
  virtual void OuterLoopBilevelChecks() = 0;
  virtual void RunAttachedAlgo() = 0;
  virtual bool UpdateMaster() = 0;
  virtual void PrintLog() = 0;
  virtual void init_data() = 0;
  virtual bool isExceptionRaised() = 0;
  virtual double OuterLoopLambdaMin() const = 0;
  virtual double OuterLoopLambdaMax() const = 0;

  virtual ~OuterLoop() = default;

 public:
  double Runtime() const;

 protected:
  double runtime_ = 0.;
};

}  // namespace Outerloop