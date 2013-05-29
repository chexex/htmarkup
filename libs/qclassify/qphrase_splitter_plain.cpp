//------------------------------------------------------------
/// @file   qphrase_splitter_plain.cpp
/// @brief  Qclassify phrase splitter for plain-text
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   14.05.2009
//------------------------------------------------------------

#include <string>

#include "utils/stringutils.hpp"
#include "qclassify_impl.hpp"

using namespace std;

namespace gogo 
{

void PhraseSplitterPlain::splitPhrase(const string &phrase)
{
    const char *s = phrase.c_str();
    int32_t length = phrase.length();

    UnicodeString ustring;
    for (int32_t i = 0; i < length;)
    {
        UChar32 c;
        U8_NEXT(s, i, length, c);

        if (u_isalnum(c)) {
            ustring += c;
        }
        else if (!ustring.isEmpty()) {
            addWord(ustring);
            ustring.truncate(0);
        }
    }

    if (!ustring.isEmpty())
        addWord(ustring);
}

} // namespace gogo
