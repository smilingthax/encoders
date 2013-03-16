#ifndef _VIEWBASE_H
#define _VIEWBASE_H

#include "ranges.h"
#include <stddef.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  #define EE_CPP11
  #include <initializer_list>
#endif

namespace Ranges {

template <typename Range,bool CheckSize=true>
struct RangeView;

namespace detail {
template <typename T> struct is_view;

#ifdef EE_CPP11
// helper to execute('map') parameter pack:  pass({(f(args),0)...});
static inline void pass(const std::initializer_list<int> &) {}
#else
struct nil {};

#define FORWARD_WITH_NIL(rettype,name,constspec) \
  template <typename T0> \
  rettype name(T0 constspec &t0) { \
    return name<1,               T0,const detail::nil,const detail::nil,const detail::nil, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (               t0,    detail::nil(),    detail::nil(),    detail::nil(), \
                      detail::nil(),    detail::nil(),    detail::nil(),    detail::nil()); \
  } \
  template <typename T0,typename T1> \
  rettype name(T0 constspec &t0,T1 constspec &t1) { \
    return name<2,               T0,               T1,const detail::nil,const detail::nil, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (               t0,           t1,detail::nil(),detail::nil(), \
                      detail::nil(),detail::nil(),detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2) { \
    return name<3,               T0,               T1,               T2,const detail::nil, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (               t0,               t1,               t2,    detail::nil(), \
                      detail::nil(),    detail::nil(),    detail::nil(),    detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3) { \
    return name<4,               T0,               T1,               T2,               T3, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (               t0,               t1,               t2,               t3, \
                      detail::nil(),    detail::nil(),    detail::nil(),    detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4) { \
    return name<5,               T0,               T1,               T2,               T3, \
                                 T4,const detail::nil,const detail::nil,const detail::nil> \
                 (               t0,               t1,               t2,               t3, \
                                 t4,    detail::nil(),    detail::nil(),    detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4,T5 constspec &t5) { \
    return name<6,               T0,               T1,               T2,               T3, \
                                 T4,               T5,const detail::nil,const detail::nil> \
                 (               t0,               t1,               t2,               t3, \
                                 t4,               t5,    detail::nil(),    detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4,T5 constspec &t5,T6 constspec &t6) { \
    return name<7,               T0,               T1,               T2,               T3, \
                                 T4,               T5,               T6,const detail::nil> \
                 (               t0,               t1,               t2,               t3, \
                                 t4,               t5,               t6,    detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6,typename T7> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4,T5 constspec &t5,T6 constspec &t6,T7 constspec &t7) { \
    return name<8,               T0,               T1,               T2,               T3, \
                                 T4,               T5,               T6,               T7> \
                 (               t0,               t1,               t2,               t3, \
                                 t4,               t5,               t6,               t7); \
  }
#endif

} // namespace detail

template <template<typename,bool> class View_Impl,typename Range,bool CheckSize=true,bool Sized=range_traits<Range>::sized>
struct OneToMulti : View_Impl<Range,Sized> {
  typedef View_Impl<Range,Sized> impl_t;
  typedef typename impl_t::base_t base_t;

  // Needed from View_Impl:
  //   bool get_one(T &ret);
  //   bool put_one(const T &val);
  //   bool request(minlen); // at least stub returning false

  explicit OneToMulti(typename detail::remove_cv<typename base_t::ctor_t>::type &range) : impl_t(range) {}
  explicit OneToMulti(const typename base_t::ctor_t &range) : impl_t(range) {}

#ifdef EE_CPP11
  template <typename... Args>
  bool get(Args&... args) {
    return get_hlp(args...);
  }

  template <typename... Args>
  bool put(const Args&... args) {
    if (impl_t::expansible) {
      if (!impl_t::request(sizeof...(args))) {
        return false;
      }
    }
    return put_hlp(args...);
  }

private:
  template <typename Arg0,typename... Args>
  bool get_hlp(Arg0 &arg0,Args&... args) {
    return (impl_t::get_one(arg0))&&(get_hlp(args...));
  }
  bool get_hlp() {
    return true;
  }
  template <typename Arg0,typename... Args>
  bool put_hlp(Arg0 &arg0,Args&... args) {
    return (impl_t::put_one(arg0))&&(put_hlp(args...));
  }
  bool put_hlp() {
    return true;
  }
#else
  FORWARD_WITH_NIL(bool,get,)
  FORWARD_WITH_NIL(bool,put,const)
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool get(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    return do_nonnil_and_get_one(t0,t1,t2,t3,t4,t5,t6,t7);
  }
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool put(const T0 &t0,const T1 &t1,const T2 &t2,const T3 &t3,
           const T4 &t4,const T5 &t5,const T6 &t6,const T7 &t7) {
    if (impl_t::expansible) {
      if (!impl_t::request(N)) {
        return false;
      }
    }
    return do_nonnil_and_put_one(t0,t1,t2,t3,t4,t5,t6,t7);
  }
private:
#define DO_NONNIL_AND(fun) \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6,typename T7> \
  bool do_nonnil_and_##fun(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) { \
    if (!impl_t::fun(t0)) { \
      return false; \
    } \
    return do_nonnil_and_##fun(t1,t2,t3,t4,t5,t6,t7,(const detail::nil &)detail::nil()); \
  } \
  bool do_nonnil_and_##fun(detail::nil,detail::nil,detail::nil,detail::nil, \
                           detail::nil,detail::nil,detail::nil,detail::nil) { \
    return true; \
  }
  DO_NONNIL_AND(get_one);
  DO_NONNIL_AND(put_one);
#undef DO_NONNIL_AND
#endif
};

template <template<typename,bool> class View_Impl,typename Range,bool CheckSize>
struct OneToMulti<View_Impl,Range,CheckSize,true> : View_Impl<Range,true> {
  typedef View_Impl<Range,true> impl_t;
  typedef typename impl_t::base_t base_t;

  // Needed from View_Impl:
  //   size_t size() const;
  //   bool request(minlen);
  //   [void] get_one(T &ret);
  //   [void] put_one(const T &val);

  explicit OneToMulti(typename detail::remove_cv<typename base_t::ctor_t>::type &range) : impl_t(range) {}
  explicit OneToMulti(const typename base_t::ctor_t &range) : impl_t(range) {}

#ifdef EE_CPP11
  template <typename... Args>
  bool get(Args&&... args) {
    if ( (CheckSize)&&(impl_t::size()<sizeof...(args)) ) {
      return false;
    }
    detail::pass({(impl_t::get_one(args),0)...});
    return true;
  }
  template <typename... Args>
  bool put(Args&&... args) {
    if ( (CheckSize)&&(!impl_t::request(sizeof...(args))) ) {
      return false;
    }
    detail::pass({(impl_t::put_one(args),0)...});
    return true;
  }
#else
  FORWARD_WITH_NIL(bool,get,)
  FORWARD_WITH_NIL(bool,put,const)
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool get(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    if ( (CheckSize)&&(impl_t::size()<N) ) {
      return false;
    }
    do_nonnil_get_one(t0,t1,t2,t3,t4,t5,t6,t7);
    return true;
  }
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool put(const T0 &t0,const T1 &t1,const T2 &t2,const T3 &t3,
           const T4 &t4,const T5 &t5,const T6 &t6,const T7 &t7) {
    if ( (CheckSize)&&(!impl_t::request(N)) ) {
      return false;
    }
    do_nonnil_put_one(t0,t1,t2,t3,t4,t5,t6,t7);
    return true;
  }
private:
#define DO_NONNIL(fun) \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6,typename T7> \
  void do_nonnil_##fun(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) { \
    impl_t::fun(t0); \
    do_nonnil_##fun(t1,t2,t3,t4,t5,t6,t7,(const detail::nil&)detail::nil()); \
  } \
  void do_nonnil_##fun(detail::nil,detail::nil,detail::nil,detail::nil, \
                       detail::nil,detail::nil,detail::nil,detail::nil) { }

  DO_NONNIL(get_one);
  DO_NONNIL(put_one);
#undef DO_NONNIL
#endif
};


template <typename RangeOrView,bool B=detail::is_view<RangeOrView>::value>
struct View_base {
  static const bool parent_is_view=detail::is_view<RangeOrView>::value; // might differ from B, when RangeView<SomeView> was used (unsupported!)

  typedef RangeView<RangeOrView> view_t;
  typedef RangeOrView range_t;
  typedef typename detail::unmodified_range<RangeOrView>::type ctor_t;

  typedef RangeOrView parent_t;

  static const bool sized=range_traits<range_t>::sized &&
                          has_member<view_t,detail::check_size>::value;
  static const bool continuous=!parent_is_view &&
                               range_traits<range_t>::continuous;
  static const bool expansible=range_traits<range_t>::expansible;
};

template <typename RangeOrView>
struct View_base<RangeOrView,true> {
  static const bool parent_is_view=true;

  typedef RangeOrView view_t;
  typedef typename RangeOrView::range_t range_t;
  typedef typename RangeOrView::ctor_t ctor_t;

  typedef RangeOrView parent_t;

  static const bool sized=range_traits<range_t>::sized &&
                          has_member<view_t,detail::check_size>::value;
  static const bool continuous=false;
  static const bool expansible=range_traits<range_t>::expansible;
};

namespace detail {
template <typename T>
struct is_view {
  typedef char (&Yes)[1];
  typedef char (&No)[2];

  template <typename S>
  static Yes chk(View_base<S> *);
  static No chk(...);

  static const bool value=sizeof(chk((T *)0))==sizeof(Yes);
};
} // namespace detail

} // namespace Ranges

#endif
