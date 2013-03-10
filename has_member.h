#ifndef _HAS_MEMBER_H
#define _HAS_MEMBER_H

/*
Idea from http://www.gockelhut.com/c++/articles/has_member

Usage:
  struct check_mymethods {
    template <typename T,
              // this also detects private members, but these are not (necessarily) accessible:
              int (T::*)(int) const = &T::method1, // signature must match exactly
              int X1=sizeof(&T::method2)           // data member or method
              int X2=sizeof(((T *)0)->foo(4),0)    // publicly callable with (4)
//            typename X1=decltype(&((T *)0)->foo) // CPP11
              [... more members]
    > struct get {};
  };

  ... has_member<T,check_mymethods>

*/

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  #include <type_traits>
#endif

namespace detail {

template <typename T,typename NameGetter>
struct has_member_impl {
  typedef char (&Yes)[1];
  typedef char (&No)[2];

  template <typename S>
  static Yes chk(typename NameGetter::template get<S>*);
  template <typename S>
  static No chk(...);

  static const bool value=sizeof(chk<T>(0))==sizeof(Yes);
};

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  using std::remove_cv;
#else
  template <typename T> struct remove_cv { typedef T type; };
  template <typename T> struct remove_cv<T const> { typedef T type; };
  template <typename T> struct remove_cv<T volatile> { typedef T type; };
  template <typename T> struct remove_cv<T const volatile> { typedef T type; };
#endif

} // namespace detail

template <typename T,typename NameGetter>
struct has_member {
  static const bool value=detail::has_member_impl<typename detail::remove_cv<T>::type,NameGetter>::value;
};

#endif
