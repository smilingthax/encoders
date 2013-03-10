#ifndef _UTFBASE_H
#define _UTFBASE_H

namespace UTFEncoding {

struct CharInfo {
  unsigned int ch : 24;     // the codepoint
  signed char len; // or -1 on error // TODO? other idea: 'parse ch', but set 0x80 / negate ... [substituting with 0xfffd can be done by caller]
  // ~0 = unknown error ... ~1 'would have been len 1'
};

} // namespace UTFEncoding

#endif
