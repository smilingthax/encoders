#ifndef _RUTFVIEW_H
#define _RUTFVIEW_H

// TODO?? type erasure / interface(virtuals)

// TODO?! remove 'advanced interface' ?!

// TODO... actually an Encoder.
// ... insert LETypeView[problem: really an Encoder -> Range trait needed] and/or SwappingView for BE16/BE32  ...

/*     UTF16LE  =^=  UTFView<UTF16,TypeView<Range>::LE16>   // FIXME: TypeView<Range> is a RangeContainer, not a Range
    [continuous]      [non-continuous, but (todo) sized]
*/

// NOTE: only for value_type=char*  (or short*/int* with UTF16/UTF32)

#include "rangeview.h"
#include "utfbase.h"
#include "utfbase.tcc"

// FIXME: #define EE_CPP11

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
struct fwd_all {

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
    return (detail::isInRange(ret.ch));
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,UTF16 *) { // {{{
    if (!isInRange(ch)) return false;
    if (ch>0xffff) {
      return view.put(leadSurrogate(ch),trailSurrogate(ch));
    }
    return view.put(ch);
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,UTF32 *) { // {{{
    if (!isInRange(ch)) return false;
    return view.put(ch);
  }
  // }}}

};

template <typename BaseView,bool Continuous>
struct fwd : fwd_all<BaseView> {
  using fwd_all<BaseView>::get;
  using fwd_all<BaseView>::put;

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
      getSecondUTF16(ret,r2.ch);
      if (ret.len<0) return false;
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

template <typename BaseView>
struct fwd<BaseView,true> : fwd_all<BaseView> { // continuous
  using fwd_all<BaseView>::get;
  using fwd_all<BaseView>::put;

  static bool get(BaseView &view,CharInfo &ret,UTF8 *) { // {{{
    ret=rawgetUTF8(view.ptr(),view.size());
    if (ret.len<=0) {
      return false;
    }
    view.next(ret.len);
    return (!isSurrogate(ret.ch));
  }
  // }}}

  static bool get(BaseView &view,CharInfo &ret,CESU8 *) { // {{{
    ret=rawgetUTF8(view.ptr(),view.size());
    if (ret.len>0) {
      view.next(ret.len);
      if (isLeadSurrogate(ret.ch)) {
        CharInfo r2=rawgetUTF8(view.ptr(),view.size());
        if (r2.len<0) return false;
        view.next(r2.len);
        ret.len+=r2.len;
        getSecondUTF16(ret,r2.ch); // will -1, if not a trail surrogate
        if (ret.len<0) return false;
      } else if (isTrailSurrogate(ret.ch)) {
        return false;
      }
    }
    return true;
  }
  // }}}

  static bool get(BaseView &view,CharInfo &ret,ModifiedUTF8 *) { // {{{
    if ( (view.size()>=2)&&
         (view.ptr()[0]==0xc0)&&(view.ptr()[1]==0x80) ) {
      ret.ch=0;
      ret.len=2;
      view.next(2);
      return true;
    }
    return get(view,ret,(CESU8 *)0);
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,UTF8 *) { // {{{
    if (isSurrogate(ch)) return false;
    const signed char len=enclenUTF8(ch);
    if (len<=0) return false;
    if (view.size()<(size_t)len) {
      return false;
    } else if (!rawputUTF8(view.ptr(),ch,len)) {
      return false;
    }
    view.next(len);
    return true;
  }
  // }}}

  static bool put(BaseView &view,unsigned int ch,CESU8 *) { // {{{
    if (!isInRange(ch)) return false;
    if (ch>0xffff) {
      if (view.size()<6) return false;
      unsigned short c0=leadSurrogate(ch),
                     c1=trailSurrogate(ch);
      if ( (!rawputUTF8(view.ptr(),c0,3))||
           (!rawputUTF8(view.ptr()+3,c1,3)) ) {
        return false;
      }
      view.next(6);
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

};

} // namespace detail

template <typename Encoding,typename RangeOrView>
struct UTFView : View_base<RangeOrView> {
  typedef View_base<RangeOrView> base_t;
  typedef typename base_t::view_t view_t;

  UTFView(typename detail::remove_cv<RangeOrView>::type &rov) : view(rov) {}
  UTFView(const RangeOrView &rov) : view(rov) {}

  bool get(UTFEncoding::CharInfo &ret) { // advanced interface // NOTE: ret.len might be wrong when false is returned!
    return detail::fwd<view_t,base_t::continuous>::get(view,ret,(Encoding *)0);
  }

  bool get(unsigned int &ret) {
    UTFEncoding::CharInfo tmp;
    const bool res=get(tmp);
    if (res) {
      ret=tmp.ch;
    }
    return res;
  }

  bool put(unsigned int ch) {
    return detail::fwd<view_t,base_t::continuous>::put(view,ch,(Encoding *)0);
  }

private:
  view_t view;
};

} // namespace Ranges

#endif
