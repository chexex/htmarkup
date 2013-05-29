//---------------------------------------------------------------------
/// @file  str_escape.cpp
/// @brief string escaping and unescaping for HTTP representation
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   17.03.2009
//---------------------------------------------------------------------

#include <string>
#include "bits/escape_tbl.hpp"
#include "stringutils.hpp"

namespace gogo {

//---------------------------------------------------------------------
/// @brief unescape string
/// @param[in]  src  - source string pointer
/// @param[out] dest - destination std::string
/// @param[in]  n    - number of characters to convert from src
//---------------------------------------------------------------------
void str_unescape(const char* src, std::string &dest, size_t n /* = -1U */)
{
  int iI , iJ;
  
  dest.clear();
  if (n != -1U)
    dest.reserve(n);
  
  for(; n > 0 && *src; src++, n--)
  {
    if (*src == '%')
    {
      const char *pC1, *pC2;
      
      pC1 = src + 1;
      iI  = -1;
      if      (*pC1 >= '0' && *pC1 <= '9') { iI = *pC1 - '0'; }
      else if (*pC1 >= 'a' && *pC1 <= 'f') { iI = *pC1 - 'a' + 10; }
      else if (*pC1 >= 'A' && *pC1 <= 'F') { iI = *pC1 - 'A' + 10; }

      if (iI != -1)
      {
        pC2 = src + 2;
        iJ  = -1;
        if      (*pC2 >= '0' && *pC2 <= '9') { iJ = *pC2 - '0'; }
        else if (*pC2 >= 'a' && *pC2 <= 'f') { iJ = *pC2 - 'a' + 10; }
        else if (*pC2 >= 'A' && *pC2 <= 'F') { iJ = *pC2 - 'A' + 10; }

        if (iJ != -1) {
          dest += (char)((iI << 4) + iJ);
          src  += 2;
          n -= 2;
        }
        else {
          dest = *src;
        }
      }
      else
        dest += *src;
    }
    else if (*src == '+') {
      dest += ' ';
    }
    else
      dest += *src;
  }
}

//---------------------------------------------------------------------
/// @brief escape string
/// @param[in]  src  - source string pointer
/// @param[out] dest - destination std::string
/// @param[in]  n    - number of characters to convert from src
//---------------------------------------------------------------------
void str_escape(const char* src, std::string &dest, size_t n /* = -1U */)
{
  dest.clear();
  
  for (; n > 0 && *src; src++, n--) {
    dest += ESCAPE_TABLE[((unsigned)*src) & 0xff];
  }
}

} // namespace gogo
