#ifndef _RANGEVIEW_TRAITS_H
#define _RANGEVIEW_TRAITS_H

#include "ranges.h"
#include <string.h>
#include <string>
#include <vector>

namespace Ranges {

namespace detail {

using ::detail::remove_cv; // has_member.h

template <typename Range,bool Continuous>
struct rangeview_trait {
  static const bool continuous=Continuous;
  static const bool sized=is_same<typename range_traits<Range>::category,sized_range_tag>::value;
//  typedef Range range_t;
  typedef typename range_traits<Range>::iterator iterator;
  static iterator make_iterator(typename remove_cv<Range>::type &range) {
    return range_traits<Range>::make_iterator(range);
  }
  static iterator make_iterator(const Range &range) {
    return range_traits<Range>::make_iterator(range);
  }
};
} // namespace detail

template <typename Range,typename Enabled=void>
struct rangeview_traits
  : detail::rangeview_trait<Range,false> {};

template <typename Range>
struct rangeview_traits<const Range>
  : detail::rangeview_trait<const Range,
                            rangeview_traits<typename detail::remove_cv<Range>::type>::continuous> {};

// continuous storage with null-termination  (otherwise use std::pair<[const] char *,[const] char *>)
template <typename T>   // allow both  const char  and  [non-const] char
struct rangeview_traits<T *,typename detail::enable_if<
                              detail::is_same<
                                typename detail::remove_cv<T>::type,
                              char>::value
                            >::type>
  : detail::rangeview_trait<std::pair<T *,T *>,true> {
  typedef typename detail::rangeview_trait<std::pair<T *,T *>,true>::iterator iterator;
  static iterator make_iterator(T *range) {
    return range_traits<std::pair<T *,T *> >::make_iterator(std::make_pair(range,range+strlen(range)));
  }
};
//  unsigned char * ?  -->  user can add trait specialisation himself!

template <typename T>
struct rangeview_traits<std::pair<T *,T *> >
  : detail::rangeview_trait<std::pair<T *,T *>,true> {};

template <typename T>
struct rangeview_traits<std::pair<const T *,const T *> >
  : detail::rangeview_trait<std::pair<const T *,const T *>,true> {};

template <typename T>
struct rangeview_traits<std::vector<T> >
  : detail::rangeview_trait<std::vector<T>,true> {};

template <typename T>
struct rangeview_traits<std::basic_string<T> >
  : detail::rangeview_trait<std::basic_string<T>,true> {};

template <typename T>
struct rangeview_traits<T []>
  : detail::rangeview_trait<T [],true> {};

template <typename T,int N>
struct rangeview_traits<T [N]>
  : detail::rangeview_trait<T [N],true> {};

} // namespace Ranges

#endif
