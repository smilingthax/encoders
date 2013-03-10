#include "utfbase.h"

// TODO? plain c?

namespace UTFEncoding {

namespace detail {

static inline bool isSurrogate(unsigned int c) // {{{
{
  return (c>=0xd800)&&(c<=0xdfff);
}
// }}}

static inline bool isLeadSurrogate(unsigned int c) // {{{
{
  return (c>=0xd800)&&(c<=0xdbff);
}
// }}}

static inline bool isTrailSurrogate(unsigned int c) // {{{
{
  return (c>=0xdc00)&&(c<=0xdfff);
}
// }}}

static inline bool isInRange(unsigned int c) // {{{
{
  return (c<=0x10ffff);
}
// }}}

static inline signed char declenUTF8(unsigned char ch) // {{{ or -1
{
  if (ch<=0x7f) {
    return 1;
  }
  if ((ch&0xe0)==0xc0) {
    return 2;
  }
  if ((ch&0xf0)==0xe0) {
    return 3;
  }
  if ((ch&0xf8)==0xf0) {
    return 4;
  }
// TODO? 5 and 6 encoding
  return -1;
}
// }}}

static inline signed char enclenUTF8(unsigned int ch) // {{{ or -1
{
  if (ch<=0x7f) {
    return 1;
  }
  if (ch<=0x7ff) {
    return 2;
  }
  if (ch<=0xffff) {
    return 3;
  }
  if (!isInRange(ch)) {
    return -1;
  }
  if (ch<=0x1fffff) {
    return 4;
  }
// TODO? 5 and 6 encoding
  return -1;
}
// }}}

static inline CharInfo getFirstUTF8(unsigned char ch) // {{{
{
  CharInfo ret;
  ret.len=declenUTF8(ch);
  switch (ret.len) {
  case 1:
    ret.ch=ch;
    break;
  case 2:
    ret.ch=ch&0x1f;
    break;
  case 3:
    ret.ch=ch&0x0f;
    break;
  case 4:
    ret.ch=ch&0x07;
    break;
// TODO? 5 and 6 encoding
  default:
    ret.len=-1;
  case -1:
    break;
  }
  return ret;
}
// }}}

static inline bool getMoreUTF8(CharInfo &ret,const char *str) // {{{
{
  for (signed char iA=ret.len-1;iA>0;iA--) {
    const unsigned char ch=*str++;
    if ((ch&0xc0)!=0x80) {
      return false;
    }
    ret.ch=(ret.ch<<6) | (ch&0x3f);
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

static inline CharInfo rawgetUTF8(const char *str,size_t maxlen) // {{{
{
  if (maxlen<1) {
    CharInfo ret;
    ret.len=-1;
    return ret;
  }
  CharInfo ret=getFirstUTF8(*str++);
  if (ret.len>0) {
    if (maxlen<(size_t)ret.len) {
      ret.len=-1; // ret.len=~ret.len;
    } else if (!getMoreUTF8(ret,str)) { // overlong, not in range, ...
      ret.len=-1;
    }
  }
  return ret;
}
// }}}

static inline bool rawputUTF8(char *dst,unsigned int ch,signed char len) // {{{
{
  switch (len) {
  case 1:
    *dst=ch;
    break;
  case 2:
    *dst++=0xc0 | (ch>>6);
    *dst=0x80 | (ch&0x3f);
    break;
  case 3:
    *dst++=0xe0 | (ch>>12);
    *dst++=0x80 | ((ch>>6)&0x3f);
    *dst=0x80 | (ch&0x3f);
    break;
  case 4:
    *dst++=0xf0 | (ch>>18);
    *dst++=0x80 | ((ch>>12)&0x3f);
    *dst++=0x80 | ((ch>>6)&0x3f);
    *dst=0x80 | (ch&0x3f);
    break;
// TODO? 5 and 6 encoding
  default:
    return false;
  }
  return true;
}
// }}}

static inline bool getFirstUTF16(CharInfo &ret,unsigned short ch,size_t len) // {{{ more?
{
  ret.ch=ch;
  if (isSurrogate(ch)) {
    if (!isLeadSurrogate(ch)) {
      ret.len=-1;
      return false;
    }
    ret.len=2*len;
  } else {
    ret.len=len;
  }
  return true;
}
// }}}

static inline void getSecondUTF16(CharInfo &ret,unsigned short ch) // {{{
{
  if (!isTrailSurrogate(ch)) {
    ret.len=-1;
  } else {
    ret.ch=((ret.ch&0x3ff)<<10) | (ch&0x3ff);
    ret.ch+=0x10000;
    // always 'in range'
  }
}
// }}}

static inline unsigned short leadSurrogate(unsigned int ch) // {{{
{
  ch-=0x10000;
  return 0xd800 | (ch>>10);
}
// }}}

static inline unsigned short trailSurrogate(unsigned int ch) // {{{
{
  return 0xdc00 | (ch&0x03ff);
}
// }}}

} // namespace detail

} // namespace UTFEncoding

