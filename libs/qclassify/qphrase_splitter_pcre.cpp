//------------------------------------------------------------
/// @file   qphrase_splitter_pcre.cpp
/// @brief  Qclassify phrase splitter for PCRE (in fact, just plain-only token extractor)
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   14.05.2009
//------------------------------------------------------------

#include <sstream>
#include <assert.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <pcre.h>

#include "defs.hpp"
#include "utils/stringutils.hpp"
#include "qclassify_impl.hpp"
using namespace std;

#define CHECK_PCRE_COMPILE

namespace gogo
{

void PhraseSplitterPCRE::splitPhrase(const string &s)
{
#ifdef CHECK_PCRE_COMPILE
  {
    const char *pcre_err_buf;
    int erroffset;
    
    pcre *re = pcre_compile(s.c_str(), PCRE_UTF8, &pcre_err_buf, &erroffset, NULL);
    if (!re) {
      stringstream ss;
      ss << "Error while compiling RE \"" << s << "\" (pos: " << erroffset << "): " << pcre_err_buf;
      throw std::runtime_error(ss.str());
    }
    
    pcre_free(re);
  }
#endif
  
    int nGroups   = 0,  // depth in '['
        nBrackets = 0,  // '('
        nBraces   = 0;  // '{'      
    
    int  wstart = -1, wlen = 0;
    bool escUsed = false;
    m_modString.clear();
  
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString word;
    UnicodeString uexpr = UTF8toUnicodeString(s);
    std::vector< std::pair<int, int> > wbounds; // word's {offset, len}
    const int32_t length = uexpr.countChar32();

#define OUTSIDE_ANY_BRACKETS (nGroups == 0 && nBrackets == 0 && nBraces == 0)
#define STOP_WORD if (OUTSIDE_ANY_BRACKETS && wlen && !escUsed) { \
  assert(wstart != -1 && !word.isEmpty()); \
  addWord(word);     \
  word.truncate(0);  \
  wbounds.push_back(std::make_pair(wstart, wlen)); \
  wlen = 0;          \
  wstart = -1;       \
}

    for (int32_t i = 0; i < length; i++)
    {
        UChar32 c = uexpr.char32At(i);

        switch(c) {
            case L'[':
                STOP_WORD;
                nGroups++;
                break;
            case L']':
                nGroups--;
                break;
              
            case L'(':
                STOP_WORD;
                nBrackets++;
                break;
            case L')':
                nBrackets--;
                break;
                
            case L'{':
                STOP_WORD;
                nBraces++;
                break; 
            case L'}':
                nBraces--;
                break; 
              
            case L'\\': // ESCAPE - skip
                escUsed = (uexpr.char32At(++i) != L's');
                break;
              
            case L' ':
            case L'\t':
                escUsed = false; // reset escape flag
              
            default:
                if (OUTSIDE_ANY_BRACKETS && !escUsed && u_isalnum(c)) {
                    if (!wlen)
                        wstart = i;

                    word += c;
                    wlen++;
                }
                else {
                    STOP_WORD;
                }
        }
    } 
  
    STOP_WORD; // save the last one


    // create modified expression:
    // simply replace found tokens to "\w+" regular expression
    UnicodeString mod_str;
    std::vector< std::pair<int, int> >::const_iterator it = wbounds.begin();

    for (int32_t i = 0; i < length; i++)
    {
        if (it != wbounds.end() && i > (it->first + it->second)) {
            it++;
        }

        if (it != wbounds.end() && i >= it->first && i < (it->first + it->second)) {
            if (i == it->first)
                mod_str += "\\b\\w+\\b";
        }
        else {
            mod_str += uexpr.char32At(i);
        }
    }

    UnicodeString2UTF8(mod_str, &m_modString);
}

} // namespace gogo
