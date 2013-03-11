#ifndef _RANGEVIEW_TRAITS_H
#define _RANGEVIEW_TRAITS_H

#include "ranges.h"
#include <string>
#include <vector>

namespace Ranges {

/*
template <typename T>
struct extensible {
};
*/

template <typename T>
struct range_continuous_trait<T *> {
  static const bool value=true;
};

template <typename T>
struct range_continuous_trait<std::pair<T *,T *> > {
  static const bool value=true;
};

template <typename T>
struct range_continuous_trait<std::pair<const T *,const T *> > {
  static const bool value=true;
};

template <typename T>
struct range_continuous_trait<std::vector<T> > {
  static const bool value=true;
};

template <typename T>
struct range_continuous_trait<std::basic_string<T> > {
  static const bool value=true;
};

} // namespace Ranges

#endif
