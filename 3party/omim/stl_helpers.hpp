#pragma once

#include <string>

namespace base
{
struct IdFunctor
{
  template <typename T>
  T operator()(T const & x) const
  {
    return x;
  }
};

inline void ToLower(std::string & s) { transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); }); }
}  // namespace base
