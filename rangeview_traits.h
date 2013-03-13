#ifndef _RANGEVIEW_TRAITS_H
#define _RANGEVIEW_TRAITS_H

#include "ranges.h"
#include <string>
#include <vector>

namespace Ranges {

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

#if 1
template <typename T>  // TODO .. const T  vs T  needs different instances of expansible...
struct expansible<std::vector<T> > {
  static bool request(typename std::vector<T> &range,typename std::vector<T>::iterator &pos,int minlen) {
    // NOTE: pos+minlen > size() already checked in RangeView, as we're sized
    size_t num=pos-range.begin(); // store iterator position
    range.resize(range.size()+minlen); // invalidates pos!
    pos=range.begin()+num; // reconstruct iterator
    return true;
  }
  typedef std::vector<T> real_range_t;
};
#else
namespace detail {

struct check_resize {
  template <typename T,
            int X=sizeof(((const T *)0)->size(),0),
            int Y=sizeof(((const T *)0)->resize(0),0)
  > struct get {};
};

} // namespace detail

  // if expansible<
          detail::enable_if<
            has_member<T,detail::check_resize>::value
          >::type
#endif

} // namespace Ranges

#endif
