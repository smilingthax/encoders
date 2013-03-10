//#include "rangeview.h"
//#include "swappingview.h"
#include "rtypeview.h"
#include <stdio.h>
#include "rutfview.h"

namespace Ranges {

#if 0
template <>
struct range_done<const unsigned char *> {
  static bool done(const unsigned char *str) {
    return strlen(str);
  }
};
#endif

#if 0
template <typename T>
struct range_done<T *> {
  static bool done(T *str) {
    return true; // no content.
  }
};
#endif

} // namespace Ranges

int main()
{

#if 0
  const char *str="asdf";
//  Ranges::RangeView<const char *> v(str);
//  Ranges::SwappingView<Ranges::RangeView<const char *> > v(str);
  Ranges::SwappingView<const char *> v(str);

  unsigned char c0,c1;
  if (v.get(c0,c1)) {
    printf("%c %c\n",c0,c1);
  } else {
    puts("read failed");
  }
#endif

#if 0
  const char *str="\xd2\x34\xd6\x78";
//  Ranges::RangeView<const unsigned char *> v((const unsigned char*&)str); // BAD cast
//  Ranges::RangeView<const char *> v(str);
  Ranges::TypeView<const char *>::LEs16 v(str);

  unsigned int c0;
  if (v.get(c0)) {
    printf("%x\n",c0);
  } else {
    puts("read failed");
  }
#endif

#if 0
  char str2[]="    ";
  char *tmp=str2;

//  Ranges::TypeView<char [5]>::LEu16 w(str2); // size 5  // [] would be unbounded
//  Ranges::TypeView<char *>::LEu16 w(str2); // size by 0-terminated
//  Ranges::TypeView<char *>::BEu16 w(tmp);
//  Ranges::TypeView<const std::pair<char *,char *> >::LEu16 w(std::make_pair(str2,str2+sizeof(str2)));
  Ranges::TypeView<std::pair<char *,char *> >::LEu16 w(std::make_pair(str2,str2+sizeof(str2)));
  unsigned int c1=0x12345678;
  if (w.put(c1)) {
    printf("%02x %02x %02x %02x\n",str2[0],str2[1],str2[2],str2[3]);
  } else {
    puts("write failed");
  }
#endif

#if 1
  char str3[10];

//  Ranges::UTFView<Ranges::UTF8,Ranges::RangeView<char [10]> > x(str3);
  Ranges::UTFView<Ranges::UTF8,char [10]> x(str3);
//  Ranges::UTFView<Ranges::UTF8,Ranges::SwappingView<char [10]> > x(str3);
//  Ranges::UTFView<Ranges::UTF8,Ranges::SwappingView<char *> > x(str3);
//  Ranges::UTFView<Ranges::UTF8,Ranges::SwappingView<unsigned char *> > x((unsigned char *)str3);
  unsigned int c2=0x13456;
  if (x.put(c2)) {
    printf("%02x %02x %02x %02x\n",(unsigned char)str3[0],(unsigned char)str3[1],(unsigned char)str3[2],(unsigned char)str3[3]);
  } else {
    puts("write failed");
  }
#endif

  return 0;
}
