#pragma once
namespace Outerloop {
class IMasterUpdate {
 public:
  virtual bool Update(double lambda_min, double lambda_max) = 0;
  [[nodiscard]] virtual double Rhs() const = 0;
  virtual ~IMasterUpdate() = default;
};
}  // namespace Outerloop
