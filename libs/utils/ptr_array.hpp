//------------------------------------------------------------
/// @file  ptr_array.hpp
/// @brief Pointer array reader and writer operating only with offsets and base
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   07.05.2009
//------------------------------------------------------------

#include <vector>
#include <stdexcept>
#include "memio.hpp"

#ifndef GOGO_PTR_ARRAY_HPP__
#define GOGO_PTR_ARRAY_HPP__

namespace gogo 
{

/// @class PtrArrayWriter
/// @brief offset storage
template <typename Toff>
class PtrArrayWriter : public QSerializerOut 
{
  std::vector<Toff> offsets;
  public:
    void push_back(Toff off) {
      offsets.push_back(off);
    }
    void clear() { offsets.clear(); }
    
    // export format: [N][OFFSETS]
    virtual size_t size() const { return sizeof(Toff) + sizeof(Toff)*offsets.size(); }
    virtual void save(MemWriter &mwr) {
      mwr << (Toff)offsets.size();
      for (unsigned i = 0; i < offsets.size(); i++)
        mwr << offsets[i];
    }
    virtual ~PtrArrayWriter() {};
};

/// @class PtrArrayReader
/// @brief pointer retrieval by offsets and base
template <typename Toff, typename Tobj = char>
class PtrArrayReader : public QSerializerIn
{
  const Toff *m_pOffsets;
  const char *m_pBase;
  Toff m_n;
  
  public:
    PtrArrayReader() : m_pBase(0) {}
    virtual void load(MemReader &mrd) {
      mrd >> m_n;
      m_pOffsets = reinterpret_cast<const Toff *>(mrd.get());
      mrd.advance(sizeof(Toff)*m_n);
    }
    virtual ~PtrArrayReader() {};
    
    size_t size() { return (size_t)m_n; }
    
    /// @brief initialize base to offsets
    void setBase(const char *b) { m_pBase = b; }
    
    /// @brief retrieve pointer w/o any checks
    Tobj *operator[](size_t i) {
      return (Tobj *)(m_pBase + m_pOffsets[i]);
    }
    
    /// @brief retrieve pointer w/o any checks
    const Tobj *operator[](size_t i) const {
      return (const Tobj *)(m_pBase + m_pOffsets[i]);
    }
    
    /// @brief strictly operator[]
    Tobj *at(size_t i) {
      if (i >= m_n || !m_pBase)
        throw std::out_of_range("PtrArrayReader::index too large");
      
      return (Tobj *)(m_pBase + m_pOffsets[i]);
    }
};

}

#endif // GOGO_PTR_ARRAY_HPP__
