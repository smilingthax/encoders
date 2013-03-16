#ifndef _SWAPPINGVIEW_H
#define _SWAPPINGVIEW_H

#include "viewbase.h"

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  #define EE_CPP11
  #include <tuple>
#endif

namespace Ranges {

namespace detail {
#ifdef EE_CPP11
template<int... S>
struct seq { };

template<int N, int... S>
struct gens_rev : gens_rev<N-1, S..., N-1> { };

template<int... S>
struct gens_rev<0, S...> {
  typedef seq<S...> type;
};

#else
template <int N,
          typename T0,typename T1,typename T2,typename T3,
          typename T4,typename T5,typename T6,typename T7>
struct shift_by : shift_by<N-1,T1,T2,T3,T4,T5,T6,T7,const nil> {
  template <typename R,typename C,
            typename U0,typename U1,typename U2,typename U3,
            typename U4,typename U5,typename U6,typename U7>
  static inline R run(C &c,R (C::*method)(U0&,U1&,U2&,U3&,U4&,U5&,U6&,U7&),
                      T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    return shift_by<N-1,T1,T2,T3,T4,T5,T6,T7,const nil>::
             template run<R,C>(c,method,t1,t2,t3,t4,t5,t6,t7,nil());
  }
};

template <typename T0,typename T1,typename T2,typename T3,
          typename T4,typename T5,typename T6,typename T7>
struct shift_by<0,T0,T1,T2,T3,T4,T5,T6,T7> {
  typedef T0 t0; typedef T1 t1; typedef T2 t2; typedef T3 t3;
  typedef T4 t4; typedef T5 t5; typedef T6 t6; typedef T7 t7;

  template <typename R,typename C>
  static inline R run(C &c,R (C::*method)(T0&,T1&,T2&,T3&,T4&,T5&,T6&,T7&),
                      T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    return (c.*method)(t0,t1,t2,t3,t4,t5,t6,t7);
  }
};
#endif
} // namespace detail

template <typename RangeOrView>
struct SwappingView : View_base<RangeOrView> { // i.e. view adapter
  typedef View_base<RangeOrView> base_t;
  typedef typename base_t::view_t view_t;

  explicit SwappingView(typename detail::remove_cv<typename base_t::ctor_t>::type &rov) : view(rov) {}
  explicit SwappingView(const typename base_t::ctor_t &rov) : view(rov) {}

  bool request(size_t minlen) {
    return view.request(minlen);
  }

  size_t size() const { // base_t::sized
    return view.size();
  }

#ifdef EE_CPP11
  template <typename... Args>
  bool get(Args&&... args) {
    return get_hlp(std::forward_as_tuple(args...),typename detail::gens_rev<sizeof...(Args)>::type());
  }
  template <typename... Args>
  bool put(Args&&... args) {
    return put_hlp(std::forward_as_tuple(args...),typename detail::gens_rev<sizeof...(Args)>::type());
  }
private:
  template <typename... Args, int... S>
  inline bool get_hlp(std::tuple<Args...> t,detail::seq<S...>) {
    return view.get(std::forward<Args>(std::get<S>(t))...);
  }
  template <typename... Args, int... S>
  inline bool put_hlp(std::tuple<Args...> t,detail::seq<S...>) {
    return view.put(std::forward<Args>(std::get<S>(t))...);
  }
#else
  FORWARD_WITH_NIL(bool,get,)
  FORWARD_WITH_NIL(bool,put,const)
private:
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool get(T0 &t0,T1 &t1,T2 &t2,T3 &t3,T4 &t4,T5 &t5,T6 &t6,T7 &t7) {
    typedef detail::shift_by<8-N,T7,T6,T5,T4,T3,T2,T1,T0> rev_t;
    return rev_t::template run<bool,view_t,
                               typename rev_t::t0,typename rev_t::t1,typename rev_t::t2,typename rev_t::t3,
                               typename rev_t::t4,typename rev_t::t5,typename rev_t::t6,typename rev_t::t7>
        (view,&view_t::template get<N>,t7,t6,t5,t4,t3,t2,t1,t0);
  }
  template <int N,typename T0,typename T1,typename T2,typename T3,
                  typename T4,typename T5,typename T6,typename T7>
  bool put(const T0 &t0,const T1 &t1,const T2 &t2,const T3 &t3,
           const T4 &t4,const T5 &t5,const T6 &t6,const T7 &t7) {
    typedef detail::shift_by<8-N,const T7,const T6,const T5,const T4,const T3,const T2,const T1,const T0> rev_t;
    return rev_t::template run<bool,view_t,
                               typename rev_t::t0,typename rev_t::t1,typename rev_t::t2,typename rev_t::t3,
                               typename rev_t::t4,typename rev_t::t5,typename rev_t::t6,typename rev_t::t7>
        (view,&view_t::template put<N>,t7,t6,t5,t4,t3,t2,t1,t0);
  }
#endif
private:
  view_t view;
};

} // namespace Ranges
#undef EE_CPP11

#endif
