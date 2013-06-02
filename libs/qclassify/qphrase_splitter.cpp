//------------------------------------------------------------
/// @file   qphrase_splitter.cpp
/// @brief  Qclassify phrase splitter
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   05.05.2009
//------------------------------------------------------------

#include <string>
#include <iostream>

#include "lem_interface/lem_interface.hpp"
#include "hashes/hashes.hpp"
#include "utils/stringutils.hpp"
#include "qclassify_impl.hpp"


#define MAX_WORDS_SPLIT 8
#define MAX_WORD_LENGTH 50

namespace gogo 
{
  
PhraseSplitterBase::PhraseSplitterBase(lemInterface *plem /* = NULL */)
{
  setLemmatizer(plem);
}

unsigned PhraseSplitterBase::split(const std::string &s) 
{
  vWords.clear();
  splitPhrase(s);
  return vWords.size();
}
  
/// @brief set lemmatizer interface pointer
/// @brief without this interface, words will not be transformed to their base forms
void PhraseSplitterBase::setLemmatizer(lemInterface *plem) { m_plem = plem; }

void PhraseSplitterBase::addWord(const UnicodeString &s)
{
    std::string utf8_buf;
    UnicodeString2UTF8(s, &utf8_buf);
    addWord(utf8_buf.c_str(), utf8_buf.length());
}
  
void PhraseSplitterBase::addWord(const char *w, int len)
{
  if (vWords.size() < MAX_WORDS_SPLIT && len <= MAX_WORD_LENGTH) 
  {
    std::string s(w, len), fform;
    word_info wi;
    
    wi.upcase = utf8_isupper(w);
    strNormalize(s);
    MurmurHash(s, &wi.form);
    
    if (m_plem && m_plem->FirstForm(s, &fform))
        MurmurHash(fform, &wi.hash);
    else
        MurmurHash(s, &wi.hash);
     
    vWords.push_back(wi);
  }
}

}
