#ifndef GOGO_STRINGUTILS_HPP__
#define GOGO_STRINGUTILS_HPP__

#include <string>
#include <cstring>
#include <sys/types.h>
#include "icuincls.h"

/**
 * trim and return length
 */
static inline int trim_crlf( char *s )
{
  char *p = strchr( s, 0 ) - 1;

  while ( p >= s && ( *p == '\r' || *p == '\n' ) )
    p--;

  *++p = 0;
  return ( p - s );
}

static inline int trim_crlf( std::string &s )
{
  size_t n =  s.find_last_not_of("\r\n");

  if (n != std::string::npos) {
    s.erase(++n);
    return n;
  }

  s.clear();
  return 0;
}

static inline bool utf8_isupper(const char *s) {
    UChar32 c;
    int32_t pos = 0;
    int32_t length = 4; /* no less than */

    U8_NEXT(s, pos, length, c);
    return u_isUUppercase(c);
}


static inline int trim_str( std::string &s )
{
  size_t n =  s.find_last_not_of("\r\n\t ");

  if (n != std::string::npos) {
    s.erase(++n);
    return n;
  }

  s.clear();
  return 0;
}


void strNormalize(std::string &a_str);


size_t base64_encode(const u_char *s, size_t len, u_char *d);
size_t base64_decode(const u_char *s, u_char *d, size_t slen = (size_t)-1);

namespace gogo {
    void str_escape(const char* src, std::string &dest, size_t n = -1U);
    void str_unescape(const char* src, std::string &dest, size_t n = -1U);
}

bool str2bool(const char *s);

#endif // GOGO_STRINGUTILS_HPP__
