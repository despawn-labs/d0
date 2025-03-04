#pragma once

#include <string>

namespace d0 {

class Error {
public:
  virtual ~Error();

  [[nodiscard]] virtual const std::string &GetMessage() const;
};

}
