#ifndef _RBACKSLASHENCVIEW_H
#define _RBACKSLASHENCVIEW_H

#include "encodingview.h"
#include <assert.h>

namespace Ranges {

namespace detail {

// NOTE: to find out when a shortened escaped number ends, we need lookahead
// i.e. (e.g.) put after get does not have the 'right' position

// inject 'must_escape'/'ascii_escape'/... into tmpl, 'simple'...  --> function pointer

template <typename EscapeSpec>
struct BackslashEncoder {
  template <typename Derived>
  struct Impl : EscapeSpec {
  protected:
    bool get_one(unsigned int &ret) { // {{{
      if (!EscapeSpec::get_saved(ret)) { // could already have gotten false, but get() will return it again.
        if (!static_cast<Derived *>(this)->view.get(ret)) {
          return false;
        }
      }
      if (ret==EscapeSpec::backslash) {
        return EscapeSpec::unescape(static_cast<Derived *>(this)->view,ret);
      }
      return true;
    }
    // }}}

    // TODO? could use 'lookbehind' (set some 'conditional-filler' variable)
    // to be able to use 'shortest' encoding, e.g. for '\0'
    bool put_one(unsigned int ch) { // {{{
      if (EscapeSpec::please_escape(ch)) {
        const unsigned int esc=EscapeSpec::pretty_escape(ch);
        if (esc) {
          return static_cast<Derived *>(this)->view.put(EscapeSpec::backslash,esc);
        } else {
          return EscapeSpec::escape(static_cast<Derived *>(this)->view,ch);
        }
      } else {
        return static_cast<Derived *>(this)->view.put(ch);
      }
    }
    // }}}
  };

  template <typename RangeOrView,bool CheckSize=true>
  struct View {
    typedef EncodingView<Impl,RangeOrView,CheckSize> type;
  };
};

} // namespace detail

// TODO: declare static const char hex[] outside ?! (need c/c++ file)
struct BEV_Escapers {
  static const unsigned char backslash='\\';

  template <typename View>
  static bool hex_escape(View &view,unsigned int ch) { // {{{
    if (ch>255) {
      assert(0);
      return false;
    }
    return view.put(backslash,'x',as_hex(ch>>4),as_hex(ch));
  }
  // }}}

  template <typename View>
  static bool octal_escape(View &view,unsigned int ch) { // {{{
    if (ch>255) {
      assert(0);
      return false;
    }
    return view.put(backslash,'0'+((ch>>6)&0x02),'0'+((ch>>3)&0x03),'0'+(ch&0x07));
  }
  // }}}

  template <typename View>
  static bool unihex4_escape(View &view,unsigned int ch) { // {{{
    if (ch>0xffff) {
      assert(0);
      return false;
    }
    return view.put(backslash,'u',as_hex(ch>>12),as_hex(ch>>8),
                                  as_hex(ch>>4),as_hex(ch));
  }
  // }}}

  static bool get_hex(unsigned int ch,unsigned int &ret) { // {{{
    if ( (ch>='0')&&(ch<='9') ) {
      ret=ch-'0';
    } else if ( (ch>='a')&&(ch<='f') ) {
      ret=ch-'a'+0x10;
    } else if ( (ch>='A')&&(ch<='F') ) {
      ret=ch-'A'+0x10;
    } else {
      return false;
    }
    return true;
  }
  // }}}

  static bool get_octal(unsigned int ch,unsigned int &ret) { // {{{
    if ( (ch>='0')&&(ch<='7') ) {
      ret=ch-'0';
      return true;
    }
    return false;
  }
  // }}}

  // work around static-init problem for hex[]
  static char as_hex(unsigned int nibble) { // {{{
    static const char hex[]="0123456789ABCDEF";
    return hex[nibble&0x0f];
  }
  // }}}
};

/* TODO: real fsm impl of hex-unescape, etc.
  problem: hex-unescape basically has multiple variants: \u[[:xdigit:]]{4}, \x[[:xdigit:]]{1,2}, \x[[:xdigit:]]{1,}
  also: handling of different 'error' conditions, e.g. for "\x":  error(C), just "\\x"(PHP), ...
problem: output-value is not really 'state-machine'
*/

struct BEV_CEscapeSpec {
  BEV_CEscapeSpec() : saved(false) {}

  static const unsigned char backslash='\\';

  static bool please_escape(unsigned int ch) { // {{{
    return (ch=='\0')||(ch=='\a')||(ch=='\b')||(ch=='\t')||(ch=='\n')||
           (ch=='\v')||(ch=='\f')||(ch=='\r')||
           (ch=='\\')||(ch=='\"');
//           (ch=='\\')||(ch=='\'')||(ch=='\"'); // ||(ch=='\?')
  }
  // }}}

  static unsigned int pretty_escape(unsigned int ch) { // {{{
    switch (ch) {
//    case '\0': return '0';  // problem: only possible if following one/two characters are not '0'..'7'
    case '\a': return 'a';
    case '\b': return 'b';
    case '\t': return 't';
    case '\n': return 'n';
    case '\v': return 'v';
    case '\f': return 'f';
    case '\r': return 'r';
    case '"': return '"';
    case '\'': return '\'';
    case '\\': return '\\';
    default: return 0;
    }
  }
  // }}}

  static unsigned int pretty_unescape(unsigned int ch) { // {{{
    switch (ch) {
    // '0' is handled by octal
    case 'a': return '\a';
    case 'b': return '\b';
    case 't': return '\t';
    case 'n': return '\n';
    case 'v': return '\v';
    case 'f': return '\f';
    case 'r': return '\r';
    case '"': return '"';
    case '\'': return '\'';
    case '\\': return '\\';
    default: return 0;
    }
  }
  // }}}

  bool get_saved(unsigned int &ret) { // {{{
    if (saved) {
      ret=save;
      saved=false;
      return true;
    }
    return false;
  }
  // }}}

// PROBLEM: c eats until non-hex char is found and then (probably) says 'out-of-range'. This does not happen with octal.
  template <typename View>
  static bool escape(View &view,unsigned int ch) { // {{{
    return BEV_Escapers::octal_escape(view,ch);
//    return BEV_Escapers::hex_escape(view,ch);
  }
  // }}}

  template <typename View>
  bool unescape(View &view,unsigned int &ret) { // {{{
    unsigned int c1;
    if (!view.get(c1)) {
      return false;
    }

    // we don't support '\u...'
    unsigned int res;
    if (c1=='x') { // hex
      return unescape_hex(view,ret);
    } else if (BEV_Escapers::get_octal(c1,res)) { // octal
      ret=res; // first char already read
      return unescape_octal(view,ret);
    }
    ret=pretty_unescape(c1);
    // else 'unsupported escape'
    return (ret!=0);
  }
  // }}}

private:
  template <typename View>
  bool unescape_hex(View &view,unsigned int &ret) { // {{{
    unsigned int c2,c34,res;
    if ( (!view.get(c2))||
         (!BEV_Escapers::get_hex(c2,res)) ) {
      // 'expected digit after \x'
      return false;
    }
    ret=res;
    if (!view.get(c34)) { // next get() will return false again
      return true;
    }
    if (BEV_Escapers::get_hex(c34,res)) {
      ret=(ret<<4)|res;
      if (!view.get(c34)) {
        return true;
      }
      if (BEV_Escapers::get_hex(c34,res)) {
        // 'out-of-range'     // TODO? ... at least in C/C++
        return false;
      }
    }
    saved=true;
    save=c34;
    return true;
  }
  // }}}

  template <typename View>
  bool unescape_octal(View &view,unsigned int &ret) { // {{{
    // first char is already read
    unsigned int c234,res;
    if (!view.get(c234)) { // next get() will return false again
      return true;
    }
    if (BEV_Escapers::get_octal(c234,res)) {
      ret=(ret<<3)|res;
      if (!view.get(c234)) {
        return true;
      }
      if (BEV_Escapers::get_octal(c234,res)) {
        ret=(ret<<3)|res;
        if (ret>0xff) {
          // 'out-of-range'     // TODO? ... at least in C/C++
          return false;
        }
        // no more lookahead needed
        return true;
      }
    }
    saved=true;
    save=c234;
    return true;
  }
  // }}}

private:
  bool saved;
  unsigned int save;
};

struct BEV_JSEscapeSpec {
  static const unsigned char backslash='\\';

  static bool get_saved(unsigned int &ret) { return false; }

  // TODO: enable replacement (function pointer template argument... [but default?])  -- must not request escaping for ch>0xffff
  // idea: use as additional fn, then default/fallback can be as simple as {return false; }  [either by null-ptr sfinae or dummy method]
  static bool please_escape(unsigned int ch) { // {{{
    return (ch<0x20)||(ch=='"')||(ch=='\\'); // 0x22 0x5c
  }
  // }}}

  static unsigned int pretty_escape(unsigned int ch) { // {{{
    switch (ch) {
    case '\b': return 'b';
    case '\t': return 't';
    case '\n': return 'n';
    case '\f': return 'f';
    case '\r': return 'r';
    case '"': return '"';
    case '\\': return '\\';
    default: return 0;
    }
  }
  // }}}

  static unsigned int pretty_unescape(unsigned int ch) { // {{{
    switch (ch) {
    case 'b': return '\b';
    case 't': return '\t';
    case 'n': return '\n';
    case 'f': return '\f';
    case 'r': return '\r';
    case '"': return '"';
    case '\\': return '\\';
    default: return 0;
    }
  }
  // }}}

  template <typename View>
  static bool escape(View &view,unsigned int ch) { // {{{
    return BEV_Escapers::unihex4_escape(view,ch);
  }
  // }}}

  template <typename View>
  static bool unescape(View &view,unsigned int &ret) { // {{{
    unsigned int c1;
    if (!view.get(c1)) {
      return false;
    }

    if (c1=='u') { // hex
      return unescape_unihex4(view,ret);
    }
    ret=pretty_unescape(c1);
    // else 'unsupported escape'
    return (ret!=0);
  }
  // }}}

private:
  template <typename View>
  static bool unescape_unihex4(View &view,unsigned int &ret) { // {{{
    unsigned int c2,c3,c4,c5;
    if (!view.get(c2,c3,c4,c5)) {
      return false;
    }
    if ( (!BEV_Escapers::get_hex(c2,c2))||
         (!BEV_Escapers::get_hex(c3,c3))||
         (!BEV_Escapers::get_hex(c4,c4))||
         (!BEV_Escapers::get_hex(c5,c5)) ) {
      return false;
    }
    ret=(c2<<12)|(c3<<8)|(c4<<4)|c5;
    return true;
  }
  // }}}
};

template <typename EscapeSpec,typename RangeOrView,bool CheckSize=true>
struct BackslashEscapeView : detail::BackslashEncoder<EscapeSpec>::template View<RangeOrView,CheckSize>::type {
  typedef typename detail::BackslashEncoder<EscapeSpec>::template View<RangeOrView,CheckSize>::type enc_t;
  typedef typename enc_t::base_t base_t;

  explicit BackslashEscapeView(typename detail::remove_cv<typename base_t::ctor_t>::type &rov) : enc_t(rov) {}
  explicit BackslashEscapeView(const typename base_t::ctor_t &rov) : enc_t(rov) {}
};

} // namespace Ranges

#endif

