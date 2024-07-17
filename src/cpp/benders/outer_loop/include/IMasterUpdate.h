#pragma once
namespace Outerloop {
class IMasterUpdate {
 public:
  virtual bool Update(double lambda_min, double lambda_max) = 0;
  virtual void Init() = 0;
  [[nodiscard]] virtual double Rhs() const = 0;
};
}  // namespace Outerloop
