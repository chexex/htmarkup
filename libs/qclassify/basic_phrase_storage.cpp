//------------------------------------------------------------
/// @file   basic_phrase_storage.cpp
/// @brief  Qclassify string (phrase) primitive storage
/// @brief  It provides linear storage of original phrases which may be retrieved next
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   13.05.2009
//------------------------------------------------------------

#include <string>
#include <vector>

#include "utils/ptr_array.hpp"
#include "qclassify.hpp"

using namespace std;

namespace gogo
{

QCBasicPhraseStorage::QCBasicPhraseStorage() : m_acclen(0), m_bDirty(true) {}

void QCBasicPhraseStorage::addPhrase(const string &s)
{
  // there should be no overhead since std::string use COW technique
  string s_buf = s; 
  
  preparePhrase(s_buf);
  m_phrases.push_back(s_buf);
  m_bDirty = true;
}

void QCBasicPhraseStorage::prepareExport() const
{
  if (m_bDirty) {
    m_offsets.clear();
    m_acclen = 0;
    for (vector<string>::const_iterator it = m_phrases.begin();
        it != m_phrases.end();
        it++)
    {
      m_offsets.push_back(m_acclen);
      m_acclen += it->length() + 1;
    }
    m_bDirty = false;
  }
}

size_t QCBasicPhraseStorage::size() const {
  prepareExport();
  return m_offsets.size() + sizeof(uint32_t) + m_acclen;
}

/// @brief reordering phraseID
void QCBasicPhraseStorage::optimize(const std::vector<unsigned> &vshift)
{
  if (m_phrases.size() > 0)
  {
    // it would be profligacy enought...
    vector<string> phrasesNew;
    
    phrasesNew.resize(m_phrases.size());
    for (unsigned i = 0; i < vshift.size(); i++) {
      phrasesNew[ vshift[i] ] = m_phrases[i];
    }
    m_phrases.assign(phrasesNew.begin(), phrasesNew.end());
    m_bDirty = true;
  }
}

// export format: [OFFSETS][SIZE][PHRASES]
void QCBasicPhraseStorage::save(MemWriter &mwr) 
{
  prepareExport();
  m_offsets.save(mwr);
  
  mwr << (uint32_t)m_acclen;
  for (vector<string>::iterator it = m_phrases.begin(); it != m_phrases.end(); it++)
    mwr << *it;
}

/////////////////////////////////////////////////////////////////////////
// QCBasicPhraseReader implementation
/////////////////////////////////////////////////////////////////////////

void QCBasicPhraseReader::load(MemReader &mrd)
{
  uint32_t sz;
  
  m_offsets.load(mrd);
  mrd >> sz;
  m_offsets.setBase(mrd.get());
  mrd.advance((size_t)sz);
}

const char *QCBasicPhraseReader::getPhrase(unsigned i) {
  return m_offsets.at(i);
}

}
