#pragma once
class IMasterUpdate {
 public:
  virtual void Update() = 0;
  virtual void AddConstraints() = 0;
  virtual void AddCutsInMaster() = 0;
};

class MasterUpdateBase : public IMasterUpdate {
 public:
  void Update() override;
  void AddConstraints() override;
  void AddCutsInMaster() override;
};
