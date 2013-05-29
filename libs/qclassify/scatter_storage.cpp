//------------------------------------------------------------
/// @file   scatter_storage.cpp
/// @brief  scattered strings reader/writer
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   10.06.2009
//------------------------------------------------------------

#include <map>
#include "utils/hash_array.hpp"
#include "qclassify.hpp"

namespace gogo
{
  
QCScatteredStringsWriter::QCScatteredStringsWriter() : m_acclen(0), m_bDirty(true) {}
  
void QCScatteredStringsWriter::add(unsigned id, const std::string &s)
{
  m_stings.insert(std::make_pair<uint32_t, std::string>(id, s));
  m_bDirty = true;
}

void QCScatteredStringsWriter::optimize(const std::vector<unsigned> &vshift)
{
  std::map<uint32_t, std::string> mapNew;
  std::map<uint32_t, std::string>::const_iterator it;

  for (it = m_stings.begin(); it != m_stings.end(); it++) {
    mapNew.insert(std::make_pair<int32_t, std::string>(vshift.at(it->first), it->second));
  }

  m_stings = mapNew;
  m_bDirty = true;
}

/// @brief prepare export creating offset hash_array
void QCScatteredStringsWriter::prepareExport() const
{
  if (m_bDirty) {
    m_id2offset.init(m_stings.size());
    std::map<uint32_t, std::string>::const_iterator it;

    m_acclen = 0;
    for (it = m_stings.begin(); it != m_stings.end(); it++) {
      m_id2offset.add(it->first, m_acclen);
      m_acclen += it->second.length() + 1;
    }
    
    m_bDirty = false;
  }
}

// format: [OFFSET_MAP][STR_SIZE:4][STRINGS:m_acclen]
size_t QCScatteredStringsWriter::size() const {
  prepareExport();
  return m_id2offset.size() + sizeof(uint32_t) + m_acclen;
}

void QCScatteredStringsWriter::save(MemWriter &mwr) 
{
  prepareExport();
  m_id2offset.save(mwr);
  mwr << (uint32_t)m_acclen;
  
  std::map<uint32_t, std::string>::const_iterator it;
  for (it = m_stings.begin(); it != m_stings.end(); it++)
    mwr << it->second;
}

//////////////////////////////////////////////////////////////
// QCScatteredStringsReader implementation
//////////////////////////////////////////////////////////////

void QCScatteredStringsReader::load(MemReader &mrd) 
{
  uint32_t sz;
  
  m_id2offset.load(mrd);
  mrd >> sz;
  m_pBase = mrd.get();
  mrd.advance(sz);
}

const char *QCScatteredStringsReader::get(unsigned id) const throw()
{
  uint32_t offset;
  if (!m_id2offset.search(id, offset))
    return NULL;
  
  return m_pBase + offset;
}

} // namespace gogo
