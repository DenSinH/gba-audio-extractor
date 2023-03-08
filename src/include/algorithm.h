#pragma once

#include <algorithm>
#include <iterator>

namespace util {

template<typename Cont, typename Pred>
Cont filter(const Cont &container, Pred predicate) {
  Cont result;
  std::copy_if(container.begin(), container.end(), std::back_inserter(result), predicate);
  return result;
}

}