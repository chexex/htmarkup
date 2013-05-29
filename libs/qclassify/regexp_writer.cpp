//------------------------------------------------------------
/// @file   regexp_writer.cpp
/// @brief  regular expression collection writer
/// @brief  The only difference between phrase storage is what not each
/// @brief  phrase is RE, so we should use map instead of vector to indexate     
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   02.06.2009
//------------------------------------------------------------

#include <string>
#include <map>
#include <vector>
#include <cassert>
#include "qclassify_impl.hpp"

using namespace std;

namespace gogo 
{
  
PhraseRegExpWriter::PhraseRegExpWriter() : m_reSize(0) {}

void PhraseRegExpWriter::add(unsigned phraseID, const string &re, uint8_t flags /* = 0 */)
{
  phrase_regexp_t re_rec;
  
  re_rec.re = re;
  re_rec.flags = flags;
  m_regs.insert(make_pair<unsigned, phrase_regexp_t>(phraseID, re_rec));
  m_reSize += re.length() + 1;
}

/// @brief reordering phraseID
void PhraseRegExpWriter::optimize(const vector<unsigned> &vshift)
{
  std::map<unsigned, phrase_regexp_t> regsNew;
  map<unsigned, phrase_regexp_t>::const_iterator it;
  
  for (it = m_regs.begin(); it != m_regs.end(); it++) {
    regsNew.insert(make_pair<unsigned, phrase_regexp_t>(vshift.at(it->first), it->second));
  }
  
  m_regs = regsNew;
}

// format:
// [# of RE:4][[ID][FLAG:1,REGULAR EXPRESSIONS(zero-end)]:Nx4]
size_t PhraseRegExpWriter::size() const {
  return sizeof(uint32_t) + (m_regs.size() * (sizeof(uint32_t) + 1)) + m_reSize;
}

void PhraseRegExpWriter::save(MemWriter &mwr)
{
  map<unsigned, phrase_regexp_t>::const_iterator it;
  
  mwr << (uint32_t)m_regs.size();
  for (it = m_regs.begin(); it != m_regs.end(); it++) {
    const phrase_regexp_t &re_rec = it->second;
    mwr << (uint32_t)it->first << re_rec.flags << re_rec.re;
  }
}

} // namespace gogo
