#ifndef GOGO_MEMFILE_HPP__
#define GOGO_MEMFILE_HPP__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <string>

namespace gogo {

//---------------------------------------------------------------------------------
/// @brief utility class for holding files in memory
/// @brief with help of heap allocation or mmap(2), with mlock'ing ability
///
class FileMemHolder {
  void *m_pmem;
  off_t m_size;
  bool  m_islocked;
  bool  m_ismmaped;
  int   m_exceptions;
  int   m_fd;
  std::string m_filename;

  std::vector<char> m_region;

  public:
    
    enum { ex_open = 0x1, ex_mmap = 0x2, ex_mlock = 0x4 };
    
    FileMemHolder() : m_pmem(NULL), m_size(0), m_islocked(false), m_ismmaped(false), m_fd(-1) {
      m_exceptions = ex_open | ex_mmap | ex_mlock;
    }
    ~FileMemHolder() { unload(); }
    bool load(const char *path, bool dommap, bool domlock);
    bool mmap();
    bool mlock();
    bool heap();
    bool open(const char *path, int flags, mode_t mode = 0);
    void unload() throw();
    
    void setExceptions(int exset) throw() { m_exceptions = exset; }
    
    void *get() { return m_pmem; }
    const void *get() const { return const_cast<const void *>(m_pmem); }
    bool is_loaded() const { return m_pmem != (void *)0; }
    off_t size() const { return m_size; }
    
    void  incore(off_t offset, size_t length, std::vector<char> &vec) const;
    int   getPagesCount(off_t offset, size_t length) const;
    
    void  preload(off_t offset, off_t size);
    void  preload(off_t size) { preload(0, size); }
    void  preload() { preload(m_size); }
    
    void  markDataDontNeed();
    void  markDataDontNeed(void *ptr, size_t len);
    
    bool isMmapped() const { return m_ismmaped; }
    bool isMlocked() const { return m_islocked; }
    
    operator int() { return m_fd; }
    bool operator ==(int fd) { return m_fd == fd; }
    
  private:
    FileMemHolder(const FileMemHolder &) {}
    
  public:
    static long pagesize;
};
//---------------------------------------------------------------------------------

int madvise_a( void *addr, size_t len, int behav );

} // namespace gogo

#endif /* GOGO_MEMFILE_HPP__ */
