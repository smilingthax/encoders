#ifndef _ENCODINGVIEW_H
#define _ENCODINGVIEW_H

#include "rangeview.h"

namespace Ranges {

namespace detail {

// Encoding_Impl has to provide  get_one(view,ret), put_one(view)
// and (maybe) factor[size_t] (>0)
template <template<typename> class Encoding_Impl>
struct EncodingView_Injector {

  template <typename RangeOrView,bool Sized>
  struct Impl : View_base<RangeOrView>, Encoding_Impl<Impl<RangeOrView,Sized> > {
    typedef View_base<RangeOrView> base_t;
    typedef typename base_t::view_t view_t;

    explicit Impl(typename remove_cv<typename base_t::ctor_t>::type &rov) : view(rov) {}
    explicit Impl(const typename base_t::ctor_t &rov) : view(rov) {}

  protected:
    bool request(size_t minlen) {
      return view.request(minlen);
    }
  private:
    friend struct Encoding_Impl<Impl<RangeOrView,Sized> >;
    view_t view;
  };

  template <typename RangeOrView>
  struct Impl<RangeOrView,true> : View_base<RangeOrView>, Encoding_Impl<Impl<RangeOrView,true> > {
    typedef View_base<RangeOrView> base_t;
    typedef typename base_t::view_t view_t;

    explicit Impl(typename remove_cv<typename base_t::ctor_t>::type &rov) : view(rov) {}
    explicit Impl(const typename base_t::ctor_t &rov) : view(rov) {}

  protected:
    bool request(size_t minlen) {
      return view.request(minlen);
    }
    size_t size() const {
      return view.size();
    }
  private:
    friend struct Encoding_Impl<Impl<RangeOrView,true> >;
    view_t view;
  };

};

#if 1
struct check_factor {
  template <int N>
  static void hlp(char (*)[N>0]=0) {}

  template <typename T,
            int X=sizeof((hlp<T::factor>(),0))
  > struct get {};
};
#endif

} // namespace detail


template <template<typename> class Encoding_Impl,
          typename RangeOrView,bool CheckSize,typename Enable=void>
struct EncodingView
  : OneToMulti<detail::EncodingView_Injector<Encoding_Impl>::template Impl,RangeOrView,CheckSize> {
  typedef OneToMulti<detail::EncodingView_Injector<Encoding_Impl>::template Impl,RangeOrView,CheckSize> otm_t;
  typedef typename otm_t::base_t base_t;

  explicit EncodingView(typename detail::remove_cv<typename base_t::ctor_t>::type &range) : otm_t(range) {}
  explicit EncodingView(const typename base_t::ctor_t &range) : otm_t(range) {}

  bool request(size_t minlen) {
    return false;
  }

};

#if 1
template <template<typename> class Encoding_Impl,
          typename RangeOrView,bool CheckSize>
struct EncodingView<Encoding_Impl,RangeOrView,CheckSize,
                    typename detail::enable_if<has_member<Encoding_Impl<RangeOrView>,detail::check_factor>::value>::type>
  : OneToMulti<detail::EncodingView_Injector<Encoding_Impl>::template Impl,RangeOrView,CheckSize> {
  typedef OneToMulti<detail::EncodingView_Injector<Encoding_Impl>::template Impl,RangeOrView,CheckSize> otm_t;
  typedef typename otm_t::base_t base_t;

  explicit EncodingView(typename detail::remove_cv<typename base_t::ctor_t>::type &range) : otm_t(range) {}
  explicit EncodingView(const typename base_t::ctor_t &range) : otm_t(range) {}

  bool request(size_t minlen) {
    return otm_t::request(minlen*otm_t::factor);
  }

  size_t size() const {
    return otm_t::size()/otm_t::factor;
  }

};
#endif


} // namespace Ranges

#endif
