#include <strings.h>
#include <stdexcept>
#include <stdint.h>

#include "icuincls.h"

bool str2bool(const char *s) {
  if (!strcasecmp(s, "yes") || !strcasecmp(s, "true"))
    return true;
  if (!strcasecmp(s, "no") || !strcasecmp(s, "false"))
    return false;

  throw std::runtime_error((std::string)"wrong string bool value `" + s + "'");
}


void strNormalize(std::string &a_str) 
{
    const char *s = a_str.c_str();
    int32_t length = a_str.length();

    UnicodeString ustring;
    for ( int32_t i = 0; i < length; )
    {
        UChar32 c;
        U8_NEXT(s, i, length, c);

        // delete combining diacritics
        if (0x0300 <= c && c <= 0x036F)
          continue;
        
        if (c == L'Ё' || c == L'ё')
            c = L'Е';
        else if (u_isULowercase(c))
            c = u_toupper(c);

        ustring += c;
    }

    a_str.clear();
    
    UnicodeString2UTF8(ustring, &a_str);
}
