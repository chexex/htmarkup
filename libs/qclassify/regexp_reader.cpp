//------------------------------------------------------------
/// @file   regexp_reader.cpp
/// @brief  regular expression collection reader and matcher
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   02.06.2009
//------------------------------------------------------------
#include <string>
#include <iostream>
#include <map>

#include "defs.hpp"
#include "qclassify_impl.hpp"
#include <pcre.h>

namespace gogo 
{

PhraseRegExReader::~PhraseRegExReader()
{
  std::map<unsigned, pcre *>::iterator it;
  
  for (it = m_regs.begin(); it != m_regs.end(); it++) {
    pcre_free(it->second);
  }
}
  
void PhraseRegExReader::load(MemReader &mrd)
{
  uint32_t n;
  const char *pcre_err;
  int   erroffset;
  
  mrd >> n;
  while(n--) {
    uint8_t  flags;
    uint32_t id;
    std::string   re;
    
    mrd >> id >> flags >> re;
    pcre *reg = pcre_compile(re.c_str(), PCRE_UTF8 | PhraseRegExp::decompressPCRE_flags(flags), 
                             &pcre_err, &erroffset, NULL);
    if (!reg) {
      std::cerr << "Failed to compile saved RE: " << pcre_err << std::endl;
    }
    else {
      m_regs.insert(std::make_pair<unsigned, pcre *>(id, reg));
    }
  }
}

/// @brief match string (s) against compiled RE of phrase (phraseID)
/// @return -1 if phraseID regexp not exist, 0 - not matched; 1 - OK.
int PhraseRegExReader::match(unsigned phraseID, const std::string &s) const
{
  std::map<unsigned, pcre *>::const_iterator it;
  
  it = m_regs.find(phraseID);
  if (it != m_regs.end()) {
    return (pcre_exec (it->second, NULL, (char *) s.c_str(), s.length(), 0, 0, NULL, 0) == -1) ? 0 : 1;
  }
  return (-1);
}

} // namespace gogo
