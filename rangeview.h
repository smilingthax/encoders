#ifndef _RANGEVIEW_H
#define _RANGEVIEW_H

#include "viewbase.h" // includes "ranges.h"

// HINT: subrange of std::vector?  =>  std::pair<>, std::vector::iterator is random access

namespace Ranges {

namespace detail {

template <bool Expansible,typename Iterator>
inline bool call_request(Iterator &it,typename enable_if<!Expansible,size_t>::type minlen) {
  return false;
}

template <bool Expansible,typename Iterator>
inline bool call_request(Iterator &it,typename enable_if<Expansible,size_t>::type minlen) {
  return it.request(minlen);
}

template <typename Range,bool Sized>
struct RangeView_Impl : View_base<Range,false> {
  typedef View_base<Range,false> base_t;
  typedef typename range_traits<Range>::iterator::value_type value_type;

  explicit RangeView_Impl(typename remove_cv<typename base_t::ctor_t>::type &range) : it(range_traits<Range>::make_iterator(range)) {}
  explicit RangeView_Impl(const typename base_t::ctor_t &range) : it(range_traits<Range>::make_iterator(range)) {}

  bool request(size_t minlen) {
    return call_request<base_t::expansible>(it,minlen);
  }

protected:
  template <typename T>
  inline bool get_one(T &ret) {
    if (it.done()) return false;
    ret=*it;
    it.next();
    return true;
  }
  template <typename T>
  inline bool put_one(T &val) {
    if (it.done()) return false;
    *it=val;
    it.next();
    return true;
  }
private:
  typename range_traits<Range>::iterator it;
};

template <typename Range>
struct RangeView_Impl<Range,true> : View_base<Range,false> {
  typedef View_base<Range,false> base_t;
  typedef typename range_traits<Range>::iterator::value_type value_type;

  explicit RangeView_Impl(typename remove_cv<typename base_t::ctor_t>::type &range) : it(range_traits<Range>::make_iterator(range)) {}
  explicit RangeView_Impl(const typename base_t::ctor_t &range) : it(range_traits<Range>::make_iterator(range)) {}

  bool request(size_t minlen) {
    if (minlen<=it.size()) {
      return true;
    }
    return call_request<base_t::expansible>(it,minlen-it.size());
  }

  size_t size() const {
    return it.size();
  }

  value_type *ptr() {
    return (base_t::continuous)?&*it:0;
  }

  void next(size_t num) { // for ptr()-access
    for (;num>0;num--) {
      it.next();
    }
  }

protected:
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
  typename range_traits<Range>::iterator it;
};

} // namespace detail


// Range can be std::pair<Iterator,Iterator> or something
template <typename Range,bool CheckSize/*=true*/> // see viewbase.h for first declaration
struct RangeView : OneToMulti<detail::RangeView_Impl,Range,CheckSize> {
  typedef OneToMulti<detail::RangeView_Impl,Range,CheckSize> otm_t;
  typedef typename otm_t::base_t base_t;

  explicit RangeView(typename detail::remove_cv<typename base_t::ctor_t>::type &range) : otm_t(range) {}
  explicit RangeView(const typename base_t::ctor_t &range) : otm_t(range) {}

};

} // namespace Ranges
#undef EE_CPP11

#endif
