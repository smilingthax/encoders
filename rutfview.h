#ifndef _RUTFVIEW_H
#define _RUTFVIEW_H

// TODO?? type erasure / interface(virtuals)

// TODO?! remove 'advanced interface' ?!

/*     UTF16LE  =^=  UTFView<UTF16,TypeView<Range>::LE16>
    [continuous]      [non-continuous, but (todo) sized]
*/

// NOTE: only for value_type=char*  (or short*/int* with UTF16/UTF32)

#include "rangeview.h"
#include "utfbase.h"
#include "utfbase.tcc"

// FIXME? #define EE_CPP11

namespace Ranges {

struct UTF8 {
  static const size_t MAXLEN=4; // limit range to utf-16 addressible U+10FFFF (e.g. RFC3629)
};
struct CESU8 : UTF8 {
  static const size_t MAXLEN=6;
};
struct ModifiedUTF8 : CESU8 {
};
struct UTF16 {
  static const size_t MAXLEN=2;
};
struct UTF32 {
  static const size_t MAXLEN=1;
};

namespace detail {

using UTFEncoding::CharInfo;
using namespace UTFEncoding::detail;

template <typename BaseView>
struct UTFfwd {

  static bool get(BaseView &view,CharInfo &ret,UTF8 *) { // {{{
    unsigned char c0;
    if (!view.get(c0)) return false;
    ret=getFirstUTF8(c0);
    if (!getMoreUTF8(view,ret)) return false;
    return (!isSurrogate(ret.ch));
  }
  // }}}

  static bool get(BaseView &view,CharInfo &ret,CESU8 *) { // {{{
    unsigned char c0;
    if (!view.get(c0)) return false;
    ret=getFirstUTF8(c0);
    if (!getMoreUTF8(view,ret)) return false;
    return (getMoreCESU8(view,ret));
  }
  // }}}

  static bool get(BaseView &view,CharInfo &ret,ModifiedUTF8 *) { // {{{
    unsigned char c0,c1;
    if (!view.get(c0)) return false;
    ret=getFirstUTF8(c0);
    if (c0==0xc0) { // certainly overlong
      if (!view.get(c1)) return false;
      return (c1==0x80);
    }
    if (!getMoreUTF8(view,ret)) return false;
    return (getMoreCESU8(view,ret));
  }
  // }}}

  static bool get(BaseView &view,CharInfo &ret,UTF16 *) { // {{{
    unsigned short c0;
    if (!view.get(c0)) return false;
    if (getFirstUTF16(ret,c0,1)) {
      unsigned short c1;
      if (!view.get(c1)) return false;
      getSecondUTF16(ret,c1);
      if (ret.len<0) return false;
    }
    return true;
  }
  // }}}

  static bool get(BaseView &view,CharInfo &ret,UTF32 *) { // {{{
    if (!view.get(ret.ch)) return false;
    ret.len=1;
    return (detail::isInRange(ret.ch))&&(!detail::isSurrogate(ret.ch));
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,UTF8 *) { // {{{
    if (isSurrogate(ch)) return false;
    return rawputUTF8(view,ch);
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,CESU8 *) { // {{{
    if (!isInRange(ch)) return false;
    if (ch>0xffff) {
      unsigned short c0=leadSurrogate(ch),
                     c1=trailSurrogate(ch);
      if ( (!rawputUTF8(view,c0))||
           (!rawputUTF8(view,c1)) ) {
        return false;
      }
      return true;
    }
    return put(view,ch,(UTF8 *)0);
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,ModifiedUTF8 *) { // {{{
    if (ch==0) {
      return view.put(0xc0,0x80);
    }
    return put(view,ch,(CESU8 *)0);
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,UTF16 *) { // {{{
    if ( (!detail::isInRange(ch))||
         (detail::isSurrogate(ch)) ) return -1;
    if (ch>0xffff) {
      return view.put(leadSurrogate(ch),trailSurrogate(ch));
    }
    return view.put(ch);
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,UTF32 *) { // {{{
    if ( (!detail::isInRange(ch))||
         (detail::isSurrogate(ch)) ) return -1;
    return view.put(ch);
  }
  // }}}

private:
  static bool getMoreUTF8(BaseView &view,CharInfo &ret) { // {{{
    unsigned char c1,c2,c3;
    switch (ret.len) {
    case 1:
      break;
    case 2:
      if ( (!view.get(c1))||
           ((c1&0xc0)!=0x80) ) {
        return false;
      }
      ret.ch=(ret.ch<<6) | (c1&0x3f);
      break;
    case 3:
      if ( (!view.get(c1,c2))||
           ((c1&0xc0)!=0x80)||
           ((c2&0xc0)!=0x80) ) {
        return false;
      }
      ret.ch=(ret.ch<<12) | ((c1&0x3f)<<6) | (c2&0x3f);
      break;
    case 4:
      if ( (!view.get(c1,c2,c3))||
           ((c1&0xc0)!=0x80)||
           ((c2&0xc0)!=0x80)||
           ((c3&0xc0)!=0x80) ) {
        return false;
      }
      ret.ch=(ret.ch<<18) | ((c1&0x3f)<<12) | ((c2&0x3f)<<6) | (c3&0x3f);
      break;
// TODO? 5 and 6 encoding
    default:
    case -1:
      return false;
    }
    if ( (ret.ch==0xfffe)||(ret.ch==0xffff) ) {
      return false; // invalid unicode (e.g. reverse bom)
    }
    if (ret.len!=enclenUTF8(ret.ch)) {
       return false; // overlong encoding, or !isInRange
    }
    return true;
  }
  // }}}

  static bool getMoreCESU8(BaseView &view,CharInfo &ret) { // {{{
    if (isLeadSurrogate(ret.ch)) {
      unsigned char c3;
      if (!view.get(c3)) return false;
      CharInfo r2=getFirstUTF8(c3);
      if (!getMoreUTF8(view,r2)) return false;
      ret.len+=r2.len;
      getSecondUTF16(ret,r2.ch); // will -1, if not a trail surrogate
      if (ret.len<=0) return false;
    } else if (isTrailSurrogate(ret.ch)) {
      return false;
    }
    return true;
  }
  // }}}

  static bool rawputUTF8(BaseView &view,unsigned int ch) { // {{{
    const signed char len=enclenUTF8(ch);
    switch (len) {
    case 1:
      return view.put(ch);
    case 2:
      return view.put(0xc0 | (ch>>6),
                      0x80 | (ch&0x3f));
    case 3:
      return view.put(0xe0 | (ch>>12),
                      0x80 | ((ch>>6)&0x3f),
                      0x80 | (ch&0x3f));
    case 4:
      return view.put(0xf0 | (ch>>18),
                      0x80 | ((ch>>12)&0x3f),
                      0x80 | ((ch>>6)&0x3f),
                      0x80 | (ch&0x3f));
// TODO? 5 and 6 encoding
    case -1:
    default:
      break;
    }
    return false;
  }
  // }}}

};

template <typename Encoding>
struct UTFEncoder {
  template <typename Derived>
  struct Impl {

    bool get(UTFEncoding::CharInfo &ret) { // advanced interface // NOTE: ret.len might be wrong when false is returned!
      return UTFfwd<typename Derived::view_t>::get(static_cast<Derived *>(this)->view,ret,(Encoding *)0);
    }

  protected:
    bool get_one(unsigned int &ret) {
      UTFEncoding::CharInfo tmp;
      const bool res=get(tmp);
      if (res) {
        ret=tmp.ch;
      }
      return res;
    }

    bool put_one(unsigned int ch) {
      return UTFfwd<typename Derived::view_t>::put(static_cast<Derived *>(this)->view,ch,(Encoding *)0);
    }
  };

  template <typename RangeOrView,bool CheckSize=true>
  struct View {
    typedef EncodingView<Impl,RangeOrView,CheckSize> type;
  };
};

} // namespace detail

template <typename Encoding,typename RangeOrView,bool CheckSize=true>
struct UTFView : detail::UTFEncoder<Encoding>::template View<RangeOrView,CheckSize>::type {
  typedef typename detail::UTFEncoder<Encoding>::template View<RangeOrView,CheckSize>::type enc_t;
  typedef typename enc_t::base_t base_t;

  explicit UTFView(typename detail::remove_cv<typename base_t::ctor_t>::type &rov) : enc_t(rov) {}
  explicit UTFView(const typename base_t::ctor_t &rov) : enc_t(rov) {}
};

} // namespace Ranges

#endif
