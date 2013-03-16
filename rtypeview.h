#ifndef _RTYPEVIEW_H
#define _RTYPEVIEW_H

#include "encodingview.h" // includes "rangeview.h"
#include "swappingview.h"
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
struct LEfwd {
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

template <typename Type>
struct LEEncoder {
  template <typename Derived> // CRTP
  struct Impl {
    static const size_t factor=sizeof(Type);

  protected:
    bool get_one(Type &ret) {
      return LEfwd<typename Derived::view_t>::get(static_cast<Derived *>(this)->view,ret);
    }

    template <typename T>
    bool get_one(T &ret) {
      Type tmp;
      const bool res=get(tmp);
      if (res) {
        ret=tmp;
      }
      return res;
    }

    bool put_one(Type ch) {
      return LEfwd<typename Derived::view_t>::put(static_cast<Derived *>(this)->view,ch);
    }
  };

  template <typename RangeOrView,bool CheckSize=true>
  struct View {
    typedef EncodingView<Impl,RangeOrView,CheckSize> type;
  };
};

} // namespace detail

template <typename Type,typename RangeOrView,bool CheckSize=true>
struct LETypeView : detail::LEEncoder<Type>::template View<RangeOrView,CheckSize>::type {
  typedef typename detail::LEEncoder<Type>::template View<RangeOrView,CheckSize>::type enc_t;
  typedef typename enc_t::base_t base_t;

  explicit LETypeView(typename detail::remove_cv<typename base_t::ctor_t>::type &rov) : enc_t(rov) {}
  explicit LETypeView(const typename base_t::ctor_t &rov) : enc_t(rov) {}
};

#ifdef EE_CPP11
template <typename Type,typename RangeOrView,bool CheckSize=true>
using BETypeView = LETypeView<Type,SwappingView<RangeOrView>,CheckSize>;

#else    // no template typedef in C++03
template <typename Type,typename RangeOrView,bool CheckSize=true>
struct BETypeView : LETypeView<Type,SwappingView<RangeOrView>,CheckSize> {
  explicit BETypeView(typename detail::remove_cv<RangeOrView>::type &rov)
    : LETypeView<Type,SwappingView<RangeOrView>,CheckSize>(rov)
  {}
  explicit BETypeView(const RangeOrView &rov)
    : LETypeView<Type,SwappingView<RangeOrView>,CheckSize>(rov)
  {}
};
#endif

template <typename RangeOrView,bool CheckSize=true>
struct TypeView {
  // RangeView's char can be 'like' s8 or u8!
  typedef LETypeView< int8_t,RangeOrView,CheckSize> s8; // LE8 == BE8
  typedef LETypeView<uint8_t,RangeOrView,CheckSize> u8;

  typedef LETypeView< int16_t,RangeOrView,CheckSize> LEs16;
  typedef LETypeView<uint16_t,RangeOrView,CheckSize> LEu16;
  typedef LETypeView< int32_t,RangeOrView,CheckSize> LEs32;
  typedef LETypeView<uint32_t,RangeOrView,CheckSize> LEu32;

  typedef BETypeView< int16_t,RangeOrView,CheckSize> BEs16;
  typedef BETypeView<uint16_t,RangeOrView,CheckSize> BEu16;
  typedef BETypeView< int32_t,RangeOrView,CheckSize> BEs32;
  typedef BETypeView<uint32_t,RangeOrView,CheckSize> BEu32;
};

/*
// TODO? C++03:   #define TEMPLATE_USING(params,name,type)  ...  [ctor forwarding... ]
struct Types {
  template <typename RangeOrView,bool CheckSize=true>
  using s8 = detail::LEEncoder<int8_t>::template View<RangeOrView,CheckSize>::type;

  using BEs16 = detail::LEEncoder<int16_t>::template View<SwappingView<RangeOrView>,CheckSize>::type;
}
*/

} // namespace Ranges
#undef EE_CPP11

#endif
