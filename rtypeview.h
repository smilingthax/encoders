#ifndef _RTYPEVIEW_H
#define _RTYPEVIEW_H

#include "swappingview.h" // includes "rangeview.h"
#include <stdint.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  #define EE_CPP11
#else
  #include <limits>
#endif

namespace Ranges {

namespace detail {

#ifdef EE_CPP11
  using std::is_signed;
  using std::make_unsigned;
#else
  template <typename T>
  struct is_signed {
    // not always the same as std::is_signed, but good enough here
    static const bool value=std::numeric_limits<T>::is_signed;
  };

  template <typename T>
  struct make_unsigned;
  template <> struct make_unsigned<int8_t> { typedef uint8_t type; };
  template <> struct make_unsigned<int16_t> { typedef uint16_t type; };
  template <> struct make_unsigned<int32_t> { typedef uint32_t type; };
#endif

template <typename BaseView>
struct LEEncoder {
  static bool get(BaseView &view,uint8_t &ret) { // {{{
    unsigned char c0;
    const bool res=view.get(c0); // increments
    if (res) {
      ret=c0;
    }
    return res;
  }
  // }}}

  static bool get(BaseView &view,uint16_t &ret) { // {{{
    unsigned char c0,c1;
    const bool res=view.get(c0,c1); // increments
    if (res) {
      ret=(c1<<8) | c0;
    }
    return res;
  }
  // }}}

  static bool get(BaseView &view,uint32_t &ret) { // {{{
    unsigned char c0,c1,c2,c3;
    const bool res=view.get(c0,c1,c2,c3);
    if (res) {
      ret=(c3<<24) | (c2<<16) | (c1<<8) | c0;
    }
    return res;
  }
  // }}}

  static bool put(BaseView &view,uint8_t ch) { // {{{
    return view.put(ch);
  }
  // }}}

  static bool put(BaseView &view,uint16_t ch) { // {{{
    return view.put(ch&0xff,(ch>>8)&0xff);
  }
  // }}}

  static bool put(BaseView &view,uint32_t ch) { // {{{
    return view.put(ch&0xff,(ch>>8)&0xff,(ch>>16)&0xff,(ch>>24)&0xff);
  }
  // }}}

  template <typename T>
  typename enable_if<is_signed<T>::value,bool>::type get(BaseView &view,T &ret) { // {{{
    typename make_unsigned<T>::type tmp;
    const bool res=get(view,tmp);
    if (res) {
      ret=tmp;
    }
    return res;
  }
  // }}}

};

} // namespace detail

template <typename Type,typename RangeOrView>
struct LETypeView : View_base<RangeOrView> {
  typedef typename View_base<RangeOrView>::view_t view_t;

  LETypeView(typename detail::remove_cv<RangeOrView>::type &rov) : view(rov) {}
  LETypeView(const RangeOrView &rov) : view(rov) {}

  size_t size() const { // only if base_t::sized
    return view.size()/sizeof(Type);
  }

  bool get(Type &ret) {
    return detail::LEEncoder<view_t>::get(view,ret);
  }

  template <typename T>
  bool get(T &ret) {
    Type tmp;
    const bool res=get(tmp);
    if (res) {
      ret=tmp;
    }
    return res;
  }

  bool put(Type ch) {
    return detail::LEEncoder<view_t>::put(view,ch);
  }

private:
  view_t view;
};

#ifdef EE_CPP11
template <typename Type,typename RangeOrView>
using BETypeView = LETypeView<Type,SwappingView<RangeOrView>>;

#else    // no template typedef in C++03
template <typename Type,typename RangeOrView>
struct BETypeView : LETypeView<Type,SwappingView<RangeOrView> > {
  BETypeView(typename detail::remove_cv<RangeOrView>::type &rov)
    : LETypeView<Type,SwappingView<RangeOrView> >(rov)
  {}
  BETypeView(const RangeOrView &rov)
    : LETypeView<Type,SwappingView<RangeOrView> >(rov)
  {}
};
#endif

template <typename RangeOrView>
struct TypeView {
  // RangeView's char can be 'like' s8 or u8!
  typedef LETypeView< int8_t,RangeOrView> s8; // LE8 == BE8
  typedef LETypeView<uint8_t,RangeOrView> u8;

  typedef LETypeView< int16_t,RangeOrView> LEs16;
  typedef LETypeView<uint16_t,RangeOrView> LEu16;
  typedef LETypeView< int32_t,RangeOrView> LEs32;
  typedef LETypeView<uint32_t,RangeOrView> LEu32;

  typedef BETypeView< int16_t,RangeOrView> BEs16;
  typedef BETypeView<uint16_t,RangeOrView> BEu16;
  typedef BETypeView< int32_t,RangeOrView> BEs32;
  typedef BETypeView<uint32_t,RangeOrView> BEu32;
};

} // namespace Ranges
#undef EE_CPP11

#endif
