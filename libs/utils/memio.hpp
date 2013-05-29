//------------------------------------------------------------
/// @file  memio.hpp
/// @brief Memory writer and reader with simply overloaded IO ops.
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   05.05.2009
//------------------------------------------------------------

#ifndef GOGO_MEMIO_HPP__
#define GOGO_MEMIO_HPP__

#include <string>
#include <cstring>
#include <fstream>

namespace gogo {

class MemWriter
{
  char *m_buf_orig;
  char *m_buf;
  std::ofstream *m_pof;
  size_t m_pos;

  public:
    MemWriter (void *buf) : m_pof(NULL) { m_buf = (char *) buf; m_buf_orig = m_buf; m_pos = 0;}
    explicit MemWriter (std::ofstream *pof) : m_pof(pof) { m_buf = NULL; m_pos = m_pof->tellp(); }

    MemWriter &operator<< (const std::string &s) {
      write(s.c_str(), s.length() + 1);
      return *this;
    }
    template<typename T>MemWriter &operator<< (const T &x) { return write(&x, sizeof(x)); }
    
    template<typename T>MemWriter &write(const T *data, size_t sz) {
      if (m_buf) {
        memcpy(m_buf, (const void *)data, sz);
        m_buf += sz;
      } else {
        m_pof->write(reinterpret_cast<const char *>(data), sz);
        m_pos += sz;
      }

      return *this;
    }
    
    size_t pos() const { return (m_buf) ? (m_buf - m_buf_orig) : m_pos; }
    size_t advance (size_t n) {
      if (m_buf) {
        m_buf += n;
        return pos();
      }
      else
        m_pof->seekp(m_pos += n);
    }
    
    char *get() { return m_buf; }
};


class MemReader
{
  const char *m_buf_orig;
  const char *m_buf;

  public:
    MemReader (const void *buf) { m_buf = (const char *) buf; m_buf_orig = m_buf; }
    MemReader &operator>> (std::string &s) {
      s.assign(m_buf);
      advance(s.length() + 1);
      return *this;
    }
    template<typename T>MemReader &operator>> (T &x) {
      x = * (T *) m_buf;
      m_buf += sizeof (T);
      return *this;
    }
    
    size_t pos() const { return m_buf - m_buf_orig; }
    size_t advance (size_t n) { m_buf += n; return pos(); }
    const char *get() { return m_buf; }
};

/// TODO: overload MemWriter/MemReader <</>> ops for following class

/// @brief interface for output-serialization
class QSerializerOut 
{
  public:
    virtual size_t size() const = 0;
    virtual void save(MemWriter &) = 0;
    
    virtual ~QSerializerOut() {}
};

/// @brief interface for input-serialization
class QSerializerIn 
{
  public:
    virtual void load(MemReader &) = 0;
    virtual ~QSerializerIn() {}
};

} // namespace gogo

#endif // GOGO_MEMIO_HPP__
