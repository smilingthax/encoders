#ifndef _RANGEVIEW_H
#define _RANGEVIEW_H

#include "rangeview_traits.h" // includes "ranges.h"

// TODO? specialize RangeView<SomeView> ???

// HINT: subrange of std::vector?  =>  std::pair<>, std::vector::iterator is random access

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  #define EE_CPP11
  #include <initializer_list>
#endif

namespace Ranges {

namespace detail {
#ifdef EE_CPP11
// helper to execute('map') parameter pack:  pass({(f(args),0)...});
inline void pass(const std::initializer_list<int> &) {}
#else
struct nil {};

#define FORWARD_WITH_NIL(rettype,name,constspec) \
  template <typename T0> \
  rettype name(T0 constspec &t0) { \
    return name<1,               T0,const detail::nil,const detail::nil,const detail::nil, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (           t0,detail::nil(),detail::nil(),detail::nil(), \
                  detail::nil(),detail::nil(),detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1> \
  rettype name(T0 constspec &t0,T1 constspec &t1) { \
    return name<2,               T0,               T1,const detail::nil,const detail::nil, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (           t0,           t1,detail::nil(),detail::nil(), \
                  detail::nil(),detail::nil(),detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2) { \
    return name<3,               T0,               T1,               T2,const detail::nil, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (           t0,           t1,           t2,detail::nil(), \
                  detail::nil(),detail::nil(),detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3) { \
    return name<4,               T0,               T1,               T2,               T3, \
                  const detail::nil,const detail::nil,const detail::nil,const detail::nil> \
                 (           t0,           t1,           t2,           t3, \
                  detail::nil(),detail::nil(),detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4) { \
    return name<5,               T0,               T1,               T2,               T3, \
                                 T4,const detail::nil,const detail::nil,const detail::nil> \
                 (           t0,           t1,           t2,           t3, \
                             t4,detail::nil(),detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4,T5 constspec &t5) { \
    return name<6,               T0,               T1,               T2,               T3, \
                                 T4,               T5,const detail::nil,const detail::nil> \
                 (           t0,           t1,           t2,           t3, \
                             t4,           t5,detail::nil(),detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4,T5 constspec &t5,T6 constspec &t6) { \
    return name<7,               T0,               T1,               T2,               T3, \
                                 T4,               T5,               T6,const detail::nil> \
                 (           t0,           t1,           t2,           t3, \
                             t4,           t5,           t6,detail::nil()); \
  } \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6,typename T7> \
  rettype name(T0 constspec &t0,T1 constspec &t1,T2 constspec &t2,T3 constspec &t3, \
               T4 constspec &t4,T5 constspec &t5,T6 constspec &t6,T7 constspec &t7) { \
    return name<8,               T0,               T1,               T2,               T3, \
                                 T4,               T5,               T6,               T7> \
                 (           t0,           t1,           t2,           t3, \
                             t4,           t5,           t6,           t7); \
  }
#endif

} // namespace detail

template <typename RangeOrView,bool B>
struct View_base;

// Range can be std::pair<Iterator,Iterator> or something
template <typename Range,bool Sized=rangeview_traits<Range>::sized>
struct RangeView : View_base<Range,false> {
  typedef typename rangeview_traits<Range>::iterator::value_type value_type;

  RangeView(typename detail::remove_cv<Range>::type &range) : it(rangeview_traits<Range>::make_iterator(range)) {}
  RangeView(const Range &range) : it(rangeview_traits<Range>::make_iterator(range)) {}

#ifdef EE_CPP11
  template <typename Arg0,typename... Args>
  bool get(Arg0 &arg0,Args&... args) {
    return (get_safe(arg0))&&(get(args...));
  }
  bool get() {
    return true;
  }

  template <typename Arg0,typename... Args>
  bool put(Arg0 &arg0,Args&... args) {
    return (put_safe(arg0))&&(put(args...));
  }
  bool put() {
    return true;
  }
#else
  FORWARD_WITH_NIL(bool,get,)
  FORWARD_WITH_NIL(bool,put,)
  FORWARD_WITH_NIL(bool,put,const)
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool get(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    return do_nonnil_and_get_safe(t0,t1,t2,t3,t4,t5,t6,t7);
  }
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool put(const T0 &t0,const T1 &t1,const T2 &t2,const T3 &t3,
           const T4 &t4,const T5 &t5,const T6 &t6,const T7 &t7) {
    return do_nonnil_and_put_safe(t0,t1,t2,t3,t4,t5,t6,t7);
  }
private:
#define DO_NONNIL_AND(fun) \
  template <typename T0,typename T1,typename T2,typename T3, \
            typename T4,typename T5,typename T6,typename T7> \
  bool do_nonnil_and_##fun(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) { \
    if (!fun(t0)) { \
      return false; \
    } \
    return do_nonnil_and_##fun(t1,t2,t3,t4,t5,t6,t7,(const detail::nil &)detail::nil()); \
  } \
  bool do_nonnil_and_##fun(detail::nil,detail::nil,detail::nil,detail::nil, \
                           detail::nil,detail::nil,detail::nil,detail::nil) { \
    return true; \
  }
  DO_NONNIL_AND(get_safe);
  DO_NONNIL_AND(put_safe);
#undef DO_NONNIL_AND
#endif
private:
  template <typename T>
  inline bool get_safe(T &ret) {
    if (it.done()) return false;
    ret=*it;
    it.next();
    return true;
  }
  template <typename T>
  inline bool put_safe(T &val) {
    if (it.done()) return false;
    *it=val;
    it.next();
    return true;
  }
private:
  typename rangeview_traits<Range>::iterator it;
};

template <typename Range>
struct RangeView<Range,true> : View_base<Range,false> {
  typedef typename rangeview_traits<Range>::iterator::value_type value_type;

  RangeView(typename detail::remove_cv<Range>::type &range) : it(rangeview_traits<Range>::make_iterator(range)) {}
  RangeView(const Range &range) : it(rangeview_traits<Range>::make_iterator(range)) {}

  size_t size() const {
    return it.size();
  }

  value_type *ptr() {
    const bool Continuous=rangeview_traits<Range>::continuous;
    return (Continuous)?&*it:0;
  }

  void next(size_t num) { // for ptr()-access
    for (;num>0;num--) {
      it.next();
    }
  }

#ifdef EE_CPP11
  template <typename... Args>
  bool get(Args&&... args) {
    if (it.size()<sizeof...(args)) {
      return false;
    }
    detail::pass({(get_one(args),0)...});
    return true;
  }
  template <typename... Args>
  bool put(Args&&... args) {
    if (it.size()<sizeof...(args)) {
      return false;
    }
    detail::pass({(put_one(args),0)...});
    return true;
  }
#else
  FORWARD_WITH_NIL(bool,get,)
  FORWARD_WITH_NIL(bool,put,)
  FORWARD_WITH_NIL(bool,put,const)
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool get(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    if (it.size()<N) {
      return false;
    }
    do_nonnil_get_one(t0,t1,t2,t3,t4,t5,t6,t7);
    return true;
  }
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool put(const T0 &t0,const T1 &t1,const T2 &t2,const T3 &t3,
           const T4 &t4,const T5 &t5,const T6 &t6,const T7 &t7) {
    if (it.size()<N) {
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
    fun(t0); \
    do_nonnil_##fun(t1,t2,t3,t4,t5,t6,t7,(const detail::nil&)detail::nil()); \
  } \
  void do_nonnil_##fun(detail::nil,detail::nil,detail::nil,detail::nil, \
                       detail::nil,detail::nil,detail::nil,detail::nil) { }

  DO_NONNIL(get_one);
  DO_NONNIL(put_one);
#undef DO_NONNIL
#endif
private:
  template <typename T>
  inline void get_one(T &ret) {
    ret=*it;
    it.next();
  }
  template <typename T>
  inline void put_one(T &val) {
    *it=val;
    it.next();
  }
private:
  typename rangeview_traits<Range>::iterator it;
};

namespace detail {
template <typename T> struct is_view;
} // namespace detail

template <typename RangeOrView,bool B=detail::is_view<RangeOrView>::value>
struct View_base {
  static const bool parent_is_view=detail::is_view<RangeOrView>::value; // might differ from B, when RangeView<SomeView> was used (unsupported!)

  typedef RangeView<RangeOrView> view_t;
  typedef RangeOrView range_t;

  typedef RangeOrView parent_t;

  static const bool sized=rangeview_traits<range_t>::sized &&
                          has_member<view_t,detail::check_size>::value;
  static const bool continuous=!parent_is_view &&
                               rangeview_traits<RangeOrView>::continuous;
};

template <typename RangeOrView>
struct View_base<RangeOrView,true> {
  static const bool parent_is_view=true;

  typedef RangeOrView view_t;
  typedef typename RangeOrView::range_t range_t;

  typedef RangeOrView parent_t;

  static const bool sized=rangeview_traits<range_t>::sized &&
                          has_member<view_t,detail::check_size>::value;
  static const bool continuous=false;
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
#undef EE_CPP11

#endif
