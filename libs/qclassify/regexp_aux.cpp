//------------------------------------------------------------
/// @file   regexp_aux.cpp
/// @brief  regular expression auxillary functions
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   03.06.2009
//------------------------------------------------------------

#include "defs.hpp"
#include "qclassify_impl.hpp"

namespace gogo {
namespace PhraseRegExp
{
  static int fl_mapping[] = {
    'i', PCRE_CASELESS, PHRASE_RE_CASELESS,
    'U', PCRE_UNGREEDY, PHRASE_RE_UNGREEDY,
    'x', PCRE_EXTENDED, PHRASE_RE_EXTENDED
  };
  
  int str2PCRE_flags(const char *s) 
  {
    int res = 0;
    unsigned i;
    
    for(; *s; s++) {
      for (i = 0; i < VSIZE(fl_mapping); i+=3) {
        if ((char)fl_mapping[i] == *s) {
          res |= fl_mapping[i+1];
          break;
        }
      }
      
      if (i >= VSIZE(fl_mapping))
        break; // unknown pattern modifier
    }
    
    return res;
  }
  
  uint8_t compressPCRE_flags(int fl) 
  {
    uint8_t res = 0;
    for (unsigned i = 1; i < VSIZE(fl_mapping); i+=3) {
      if (fl & fl_mapping[i])
        res |= (uint8_t)fl_mapping[i+1];
    }
    return res;
  }
  
  int decompressPCRE_flags(uint8_t cfl) 
  {
    int res = 0;
    for (unsigned i = 2; i < VSIZE(fl_mapping); i+=3) {
      if (cfl & (uint8_t)fl_mapping[i])
        res |= fl_mapping[i-1];
    }
    return res;
  }
  
} // namespace PhraseRegExp
} // namespace gogo
