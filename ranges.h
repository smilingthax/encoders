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

// forward declarations  ... these can also be extended by the user
template <typename Range,typename Enable=void>
struct range_done;

template <typename Range,typename Enable=void>
struct range_continuous_trait;

template <typename Range,typename Enable=void>
struct range_traits;

template <typename Range>
struct expansible;

namespace detail {

using ::detail::remove_cv; // has_member.h

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

template <typename Tag>
struct check_iterator_tag {
  static void fn(Tag) {}
  template <typename T,
            int X=sizeof(check_iterator_tag<Tag>::fn(typename std::iterator_traits<T>::iterator_category()),0)
  > struct get {};
};

template <typename Iterator>
struct is_random_access {
  static const bool value=has_member<Iterator,check_iterator_tag<std::random_access_iterator_tag> >::value;
};

template <typename Iterator>
struct unbounded_iterator {
  typedef Iterator iterator;
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;
//  typedef ifelse<has_type<Iterator::reference>,typename Iterator::reference,value_type &> reference; // TODO?

  explicit unbounded_iterator(const Iterator &it) : it(it) {}
  // no update()

  bool done() const { return range_done<Iterator>::done(it); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  Iterator it;
};

template <typename Iterator>
struct falseterminated_iterator {
  typedef Iterator iterator;
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  explicit falseterminated_iterator(const Iterator &it) : it(it) {}
  // no update()

  bool done() const { return (!*it); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  Iterator it;
};

template <typename Iterator>
struct bounded_iterator {
  typedef Iterator iterator;
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  bounded_iterator(const Iterator &it,const Iterator &end) : it(it),end(end) {}
  void update(const Iterator &_end) { end=_end; }

  bool done() const { return (it==end); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  Iterator it,end;
};

template <typename Iterator>
struct emulated_sized_iterator {
  typedef Iterator iterator;
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  emulated_sized_iterator(const Iterator &it,const Iterator &end,size_t size) : it(it),end(end),ipos(0),isize(size) {}
  void update(const Iterator &_end,size_t _size) { end=_end; isize=_size; }

  bool done() const { return (it==end); }
  reference operator*() const { return *it; }
  void next() { ++it; ++ipos; }

  size_t size() const { return isize-ipos; }

  Iterator it,end;
  size_t ipos,isize;
};

template <typename Iterator>
struct sized_iterator {
  typedef Iterator iterator;
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  typedef typename std::iterator_traits<Iterator>::reference reference;

  sized_iterator(const Iterator &it,const Iterator &end) : it(it),end(end) {}
  void update(const Iterator &_end) { end=_end; }

  bool done() const { return (it==end); }
  reference operator*() const { return *it; }
  void next() { ++it; }

  size_t size() const { return (end-it); }

  Iterator it,end;
};

template <typename Range,typename BaseRIterator>
struct expansible_iterator { // wrapper
  typedef typename BaseRIterator::iterator iterator; // not: Range
  typedef typename BaseRIterator::value_type value_type;
  typedef typename BaseRIterator::reference reference;

  expansible_iterator(Range &container,BaseRIterator base) : container(container),base(base) {}
  // no update!  (no nesting!!)

  bool done() const { return base.done(); }
  reference operator*() const { return *base; }
  void next() { base.next(); }

  // only makes sense if base is sized
  size_t size() const { return base.size(); }

  bool request(size_t minlen) {
    // TODO?!?  no base.it... but request() has to restore the iterator, when invalidated, and may want current position
    const bool ret=expansible<Range>::request(container,base.it,minlen);
    if (ret) { // update end
      range_traits<Range>::update_iterator(container,base);
    }
    return ret;
  }

  Range &container;
  BaseRIterator base;
};

template <typename Range,typename Tag,
          bool Continuous=range_continuous_trait<Range>::value >
struct range_trait {
  typedef Tag category;
  static const bool continuous=Continuous;
  static const bool sized=is_same<Tag,sized_range_tag>::value;
  static const bool expansible=false;
  // iterator, make_iterator
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

// can be extended by user (or rangeview_traits.h)
template <typename Range,typename Enable>
struct range_continuous_trait {
  static const bool value=false;
};

template <typename Range,typename Enable>
struct range_traits
  : detail::range_trait<Range,unbounded_range_tag> {

  typedef detail::unbounded_iterator<Range> iterator;
  static iterator make_iterator(typename detail::remove_cv<Range>::type &range) {
    return iterator(range);
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range);
  }
  static void update_iterator(const Range &range,iterator &it) {}
};

template <typename Range>
struct range_traits<Range,typename detail::enable_if<has_member<Range,detail::check_beginend>::value &&
                                                     !has_member<Range,detail::check_size>::value &&
                                                     !detail::is_random_access<typename Range::iterator>::value
                                                    >::type>
  : detail::range_trait<Range,bounded_range_tag> {

  typedef detail::bounded_iterator<typename Range::iterator> iterator;
  static iterator make_iterator(typename detail::remove_cv<Range>::type &range) {
    return iterator(range.begin(),range.end());
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range.begin(),range.end());
  }
  static void update_iterator(typename detail::remove_cv<Range>::type &range,iterator &it) {
    it.update(range.end());
  }
  static void update_iterator(const Range &range,iterator &it) {
    it.update(range.end());
  }
};

template <typename Range>
struct range_traits<Range,typename detail::enable_if<has_member<Range,detail::check_beginend>::value &&
                                                     has_member<Range,detail::check_size>::value &&
                                                     !detail::is_random_access<typename Range::iterator>::value
                                                    >::type>
  : detail::range_trait<Range,sized_range_tag> {

  typedef detail::emulated_sized_iterator<typename Range::iterator> iterator;
  static iterator make_iterator(typename detail::remove_cv<Range>::type &range) {
    return iterator(range.begin(),range.end(),range.size());
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range.begin(),range.end(),range.size());
  }
  static void update_iterator(typename detail::remove_cv<Range>::type &range,iterator &it) {
    it.update(range.end(),range.size());
  }
  static void update_iterator(const Range &range,iterator &it) {
    it.update(range.end(),range.size());
  }
};

// TODO? kind-of continuous
template <typename Range>
struct range_traits<Range,typename detail::enable_if<has_member<Range,detail::check_beginend>::value &&
                                                     detail::is_random_access<typename Range::iterator>::value
                                                    >::type>
  : detail::range_trait<Range,sized_range_tag> {

  typedef detail::sized_iterator<typename Range::iterator> iterator;
  static iterator make_iterator(typename detail::remove_cv<Range>::type &range) {
    return iterator(range.begin(),range.end());
  }
  static iterator make_iterator(const Range &range) {
    return iterator(range.begin(),range.end());
  }
  static void update_iterator(typename detail::remove_cv<Range>::type &range,iterator &it) {
    it.update(range.end());
  }
  static void update_iterator(const Range &range,iterator &it) {
    it.update(range.end());
  }
};

// Pair types
template <typename Iterator>
struct range_traits<std::pair<Iterator,Iterator>,
                    typename detail::enable_if<!detail::is_random_access<Iterator>::value
                                              >::type>
  : detail::range_trait<std::pair<Iterator,Iterator>,bounded_range_tag> {

  typedef detail::bounded_iterator<Iterator> iterator;
  static iterator make_iterator(const std::pair<Iterator,Iterator> &range) {
    return iterator(range.first,range.second);
  }
  // TODO? pair types are not resizeable without special help... (falseterminated<> does it right, though)
  static void update_iterator(const std::pair<Iterator,Iterator> &range,iterator &it) {
    it.update(range.second);
  }
};

template <typename Iterator>
struct range_traits<std::pair<Iterator,Iterator>,
                    typename detail::enable_if<detail::is_random_access<Iterator>::value
                                              >::type>
  : detail::range_trait<std::pair<Iterator,Iterator>,sized_range_tag> {

  typedef detail::sized_iterator<Iterator> iterator;
  static iterator make_iterator(const std::pair<Iterator,Iterator> &range) {
    return iterator(range.first,range.second);
  }
  static void update_iterator(const std::pair<Iterator,Iterator> &range,iterator &it) {
    it.update(range.second);
  }
};

template <typename Iterator>
struct range_traits<const std::pair<Iterator,Iterator> >
  : range_traits<std::pair<Iterator,Iterator> > { };

// Array types
template <typename T>
struct range_traits<T []>
  : detail::range_trait<T [],unbounded_range_tag,true> {

  typedef detail::unbounded_iterator<T *> iterator;
  static iterator make_iterator(T range[]) {
    return iterator(range);
  }
  static void update_iterator(const T &range,iterator &it) {}
};

template <typename T,int N>
struct range_traits<T [N]>
  : detail::range_trait<T [N],sized_range_tag,true> {

  typedef detail::sized_iterator<T *> iterator;
  static iterator make_iterator(T range[N]) {
    return iterator(range,range+N);
  }
  static void update_iterator(const T &range,iterator &it) {}
};

// 'modified types':  falseterminate<Range>, expansible<Range>, ...
namespace detail {

template <typename Range,typename Enable=void>
struct unmodified_range {
  typedef Range type;
};

template <typename Range>
struct unmodified_range<Range,typename detail::enable_if<sizeof(typename Range::real_range_t)>::type> {
  typedef typename Range::real_range_t type;
};

} // namespace detail

// user can add specialization for different peekBefore behavior
template <typename Range>
struct falseterminated { // NOTE: could be continuous or not
  // is Range at least be iterable twice (e.g. forward iterator)?
  static const bool peekBefore=has_member<Range,detail::check_iterator_tag<std::forward_iterator_tag> >::value ||
                                                range_continuous_trait<Range>::value;
  typedef typename detail::unmodified_range<Range>::type real_range_t; // to support nesting, otherwise 'typedef Range real_range_t' would be enough
};

template <typename Iterator>
struct range_traits<falseterminated<Iterator>,
                    typename detail::enable_if<!falseterminated<Iterator>::peekBefore
                   >::type>
  : detail::range_trait<Iterator,unbounded_range_tag> {

  typedef detail::falseterminated_iterator<Iterator> iterator;

  static iterator make_iterator(typename detail::remove_cv<Iterator>::type &range) {
    return iterator(range);
  }
  static iterator make_iterator(const Iterator &range) {
    return iterator(range);
  }
  static void update_iterator(const Iterator &range,iterator &it) {}
};

template <typename Iterator>
struct range_traits<falseterminated<Iterator>,
                    typename detail::enable_if<falseterminated<Iterator>::peekBefore
                   >::type>
  : range_traits<std::pair<Iterator,Iterator> > {

  typedef range_traits<std::pair<Iterator,Iterator> > base_t;
  typedef typename base_t::iterator iterator;

  static iterator make_iterator(typename detail::remove_cv<Iterator>::type &range) {
    typename detail::remove_cv<Iterator>::type end=range;
    while (*end) {
      ++end;
    }
    return base_t::make_iterator(std::make_pair(range,end));
  }
  static iterator make_iterator(const Iterator &range) {
    typename detail::remove_cv<Iterator>::type end=range;
    while (*end) {
      ++end;
    }
    return base_t::make_iterator(std::make_pair(range,end));
  }
  static void update_iterator(typename detail::remove_cv<Iterator>::type &range,iterator &it) {
    typename detail::remove_cv<Iterator>::type end=it.first;
    while (*end) {
      ++end;
    }
    base_t::update_iterator(std::make_pair(it.first,end),it);
  }
  static void update_iterator(const Iterator &range,iterator &it) {
    typename detail::remove_cv<Iterator>::type end=range;
    while (*end) {
      ++end;
    }
    base_t::update_iterator(std::make_pair(it.first,end),it);
  }
};

template <typename Range>
struct expansible; // no default impl!
/*
{
  typedef Range real_range_t;

  static bool request(Range &range,range_traits<Range>::iterator &it,int minlen); // NOTE: must restore the iterator, if invalidated
};
*/

// falseterminated<expansible<T> >   is not possible  (iterator.request() is not forwarded in iterators)
// expansible<expansible<T> >        is also forbidden
// expansible<falseterminated<T> >   can work (but think about it!)

template <typename Range>
struct range_traits<expansible<expansible<Range> > >; // no impl!  // better: static_assert?!
template <typename Range>
struct range_traits<falseterminated<expansible<Range> > >;  // no impl!
// CPP11?  static_assert(sizeof(Range)==0,"Do not instantiate this template");

template <typename Range>
struct range_traits<expansible<Range> >
  : range_traits<Range> {

  static const bool expansible=true;

  typedef range_traits<Range> base_t;
  typedef typename detail::expansible_iterator<Range,typename base_t::iterator> iterator;

  static iterator make_iterator(typename detail::remove_cv<Range>::type &range) {
    return iterator(range,base_t::make_iterator(range));
  }
  // does not make sense without special expansible::request override:
  static iterator make_iterator(const Range &range) {
    return iterator(range,base_t::make_iterator(range));
  }
  // no static
};

} // namespace Ranges
#undef EE_CPP11

#endif
