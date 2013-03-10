#ifndef _RANGES_H
#define _RANGES_H

#include "has_member.h"
#include <iterator>

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||(__cplusplus>=201103L)
  #define EE_CPP11
  #include <type_traits>
#endif

namespace Ranges {

struct unbounded_range_tag {};    // needs value_type, done() fn, might have has()
struct bounded_range_tag {};      // e.g. std::pair<InputIterator,>, but no size() [e.g. not RandomAccessIterator]
struct sized_range_tag : bounded_range_tag {};   // begin(Range), [end(Range),] size()

template <typename Range,typename Enable=void>
struct range_done;

namespace detail {

#ifdef EE_CPP11
  using std::enable_if;
  using std::is_same;
#else
  template<bool B, class T = void>
  struct enable_if {};

  template<class T>
  struct enable_if<true, T> { typedef T type; };

  template<typename S, typename T>
  struct is_same {
    static const bool value=false;
  };

  template<typename T>
  struct is_same<T,T> {
    static const bool value=true;
  };
#endif

struct check_size {
  template <typename T,
            int X=sizeof(((const T *)0)->size(),0)
  > struct get {};
};

struct check_done {
  template <typename T,
            int X=sizeof(((const T *)0)->done(),0)
  > struct get {};
};

struct check_beginend {
  template <typename T,
            typename U=typename T::iterator,
            int X=sizeof(((T *)0)->begin(),0),
            int Y=sizeof(((T *)0)->end(),0)
  > struct get {};
};

template <typename Iterator>
struct is_random_access {
  static const bool value=is_same<typename std::iterator_traits<Iterator>::iterator_category,
                                  std::random_access_iterator_tag>::value;
};

template <typename Iterator>
struct unbounded_iterator {
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;
//  typedef ifelse<has_type<Iterator::reference>,typename Iterator::reference,value_type &> reference; // TODO?

  unbounded_iterator(const Iterator &it) : it(it) {}

  bool done() const { return range_done<Iterator>::done(it); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  Iterator it;
};

template <typename Iterator>
struct bounded_iterator {
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  bounded_iterator(const Iterator &it,const Iterator &end) : it(it),end(end) {}

  bool done() const { return (it==end); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  Iterator it,end;
};

template <typename Iterator>
struct emulated_sized_iterator {
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  emulated_sized_iterator(const Iterator &it,const Iterator &end,size_t size) : it(it),end(end),ipos(0),isize(size) {}

  bool done() const { return (it==end); }
  reference operator*() const { return *it; }
  void next() { ++it; ++ipos; }
  size_t size() const { return isize-ipos; }

  Iterator it,end;
  size_t ipos,isize;
};

template <typename Iterator>
struct sized_iterator {
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  sized_iterator(const Iterator &it,const Iterator &end) : it(it),end(end) {}

  bool done() const { return (it==end); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  size_t size() const { return (end-it); }

  Iterator it,end;
};

} // namespace detail

// can be extended by user
template <typename Range,typename Enable>
struct range_done {
  static bool done(const Range &range) { // really unbounded
    return false;
  }
};

template <typename Iterator>
struct range_done<Iterator,typename detail::enable_if<has_member<Iterator,detail::check_done>::value>::type>
{
  static bool done(const Iterator &it) {
    return it.done();
  }
};

template <typename Range,typename Enable=void>
struct range_traits {
  typedef unbounded_range_tag category;
  typedef detail::unbounded_iterator<Range> iterator;
  static iterator make_iterator(Range &range) {
    return iterator(range);
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range);
  }
};

template <typename Range>
struct range_traits<Range,typename detail::enable_if<has_member<Range,detail::check_beginend>::value &&
                                                     !has_member<Range,detail::check_size>::value &&
                                                     !detail::is_random_access<typename Range::iterator>::value
                                                    >::type> {
  typedef bounded_range_tag category;
  typedef detail::bounded_iterator<typename Range::iterator> iterator;
  static iterator make_iterator(Range &range) {
    return iterator(range.begin(),range.end());
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range.begin(),range.end());
  }
};

template <typename Range>
struct range_traits<Range,typename detail::enable_if<has_member<Range,detail::check_beginend>::value &&
                                                     has_member<Range,detail::check_size>::value &&
                                                     !detail::is_random_access<typename Range::iterator>::value
                                                    >::type> {
  typedef sized_range_tag category;
  typedef detail::emulated_sized_iterator<typename Range::iterator> iterator;
  static iterator make_iterator(Range &range) {
    return iterator(range.begin(),range.end(),range.size());
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range.begin(),range.end(),range.size());
  }
};

template <typename Range>
struct range_traits<Range,typename detail::enable_if<has_member<Range,detail::check_beginend>::value &&
                                                     detail::is_random_access<typename Range::iterator>::value
                                                    >::type> {
  typedef sized_range_tag category;
  typedef detail::sized_iterator<typename Range::iterator> iterator;
  static iterator make_iterator(Range &range) {
    return iterator(range.begin(),range.end());
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range.begin(),range.end());
  }
};

// Pair types
template <typename Iterator>
struct range_traits<std::pair<Iterator,Iterator>,
                    typename detail::enable_if<!detail::is_random_access<Iterator>::value
                                              >::type> {
  typedef bounded_range_tag category;
  typedef detail::bounded_iterator<Iterator> iterator;
  static iterator make_iterator(const std::pair<Iterator,Iterator> &range) {
    return iterator(range.first,range.second);
  }
};

template <typename Iterator>
struct range_traits<std::pair<Iterator,Iterator>,
                    typename detail::enable_if<detail::is_random_access<Iterator>::value
                                              >::type> {
  typedef sized_range_tag category;
  typedef detail::sized_iterator<Iterator> iterator;
  static iterator make_iterator(const std::pair<Iterator,Iterator> &range) {
    return iterator(range.first,range.second);
  }
};

template <typename Iterator>
struct range_traits<const std::pair<Iterator,Iterator> >
  : range_traits<std::pair<Iterator,Iterator> > { };

// Array types
template <typename T>
struct range_traits<T []> {
  typedef unbounded_range_tag category;
  typedef detail::unbounded_iterator<T *> iterator;
  static iterator make_iterator(T range[]) {
    return iterator(range);
  }
};

template <typename T,int N>
struct range_traits<T [N]> {
  typedef sized_range_tag category;
  typedef detail::sized_iterator<T *> iterator;
  static iterator make_iterator(T range[N]) {
    return iterator(range,range+N);
  }
};

} // namespace Ranges
#undef EE_CPP11

#endif
