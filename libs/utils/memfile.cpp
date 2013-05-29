//---------------------------------------------------------------------------------
/// @file  libs/util/memfile.cpp
/// @brief utility class for holding files in memory
/// @brief with help of heap allocation or mmap(2), with mlock'ing ability
///
//---------------------------------------------------------------------------------

#ifdef HAVE_READAHEAD
#ifndef _GNU_SOURCE
// readahead(2)
#define _GNU_SOURCE
#endif
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <cassert>

#include <vector>
#include <stdexcept>
#include <sstream> // err formating

#include "config.h"
#include "syserror.hpp"
#include "fileutils.hpp"
#include "memfile.hpp"
using namespace std;


namespace gogo {
  
long FileMemHolder::pagesize = sysconf(_SC_PAGESIZE);

//---------------------------------------------------------------------------------
bool
FileMemHolder::open(const char *path, int flags, mode_t mode /* = 0 */)
{
  unload();
  
  m_filename = path;
  
  m_fd = (mode) ? ::open(path, flags, mode) : ::open(path, flags);

  if (m_fd < 0) {
    if (m_exceptions & FileMemHolder::ex_open) {
      throw SysError<FileOpenErrFormat>(path);
    }
    return false;
  }

  struct stat st;
    
  if (fstat(m_fd, &st) < 0) {
    if (m_exceptions & FileMemHolder::ex_open) {
      throw SystemError("fstat failed");
    }
    return false;
  }
  
  m_size = st.st_size;
  return true;
}

bool
FileMemHolder::mmap()
{
  assert(!m_pmem);
  assert(m_fd != -1);
  
  // prevent mmaping of empty file; linux kernel return EINVAL
  if (m_size) 
  {
    void *p = ::mmap(NULL, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0);
    if (p == MAP_FAILED) {
      if (m_exceptions & FileMemHolder::ex_mmap) 
      {
        stringstream ss;
        ss << "mmap of " << m_filename << " failed";
        throw SystemError( ss.str() );
      }
      return false;
    }
  
    m_ismmaped = true;
    m_pmem     = p;
  }
  
  return true;
}

bool
FileMemHolder::mlock()
{
  if (m_fd == -1 || (m_size > 0 && !m_pmem)) {
    throw runtime_error("FileMemHolder: attempt to mlock with no mem");
  }
  
  // prevent mlocking of empty file of empty file like this->mmap() do
  if (m_size) 
  {
    if (::mlock(m_pmem, m_size) < 0) {
      if (m_exceptions & FileMemHolder::ex_mlock) {
        stringstream ss;
        ss << "failed to mlock " << m_size << " bytes of " << m_filename;
        throw SystemError( ss.str() );
      }
        
      return false;
    }
    
    m_islocked = true;
  }

  return true;
}

bool
FileMemHolder::heap()
{
  assert(m_fd != -1);
  
  // load in HEAP
  m_region.resize(m_size + 1); // handle zero-size well
  m_pmem = (void *)&m_region[0];
    
  if (m_size != 0 && !Read(m_fd, m_pmem, m_size)) {
    stringstream ss;
    ss << "failed to read " << m_size << " bytes from " << m_filename;
    throw SystemError( ss.str() );
  }

  m_ismmaped = false;
  return true;
}

//---------------------------------------------------------------------------------
bool
FileMemHolder::load(const char *path, bool dommap, bool domlock)
{
  if (!open(path, O_RDONLY))
    return false;

  if (dommap) {
    if (!mmap())
      return false;
  }
  else {
    if (!heap())
      return false;
  }

  if (domlock && !mlock())
      return false;

  return true;
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
void
FileMemHolder::unload() throw()
{
  if (m_pmem)
  {
    if (m_islocked)
      munlock(m_pmem, m_size);
    
    if (m_ismmaped)
      munmap(m_pmem, m_size);
  }
  
  m_islocked = false;
  m_ismmaped = false;
  m_size     = 0;
  m_pmem     = NULL;
  
  m_region.clear();
  
  if (m_fd != -1)
    close(m_fd);
  
  m_filename.clear();
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
int
FileMemHolder::getPagesCount(off_t offset, size_t length) const
{
  if (!m_pmem || offset >= m_size)
    return -1;
  
  const long page_mask = FileMemHolder::pagesize - 1;
  
  // page boundary or EINVAL
  char *p = (char *)(((long)m_pmem + offset) & ~page_mask);
  length += ((long)m_pmem + offset) & page_mask;
  
  if (p + length > (char *)m_pmem + m_size)
    length = m_size - (p - (char *)m_pmem);
  
  return (length + FileMemHolder::pagesize - 1) / FileMemHolder::pagesize;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void
FileMemHolder::markDataDontNeed()
{
  if (isMmapped() && !isMlocked())
    madvise_a(m_pmem, m_size, MADV_DONTNEED);
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void
FileMemHolder::markDataDontNeed(void *ptr, size_t len)
{
  if (isMmapped() && !isMlocked()) {
    assert( reinterpret_cast<char *>(ptr) >= reinterpret_cast<char *>(m_pmem) && (len / FileMemHolder::pagesize) <= (size_t)(m_size / FileMemHolder::pagesize) );
    madvise_a(reinterpret_cast<char *>(ptr), len, MADV_DONTNEED);
  }
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void
FileMemHolder::incore(off_t offset, size_t length, std::vector<char> &vec)  const
{
#ifdef linux
#define INCORE_ARR_T unsigned char *
#else
#define INCORE_ARR_T char *
#endif
  
  if (!m_pmem)
    return;
  
  const long page_mask = FileMemHolder::pagesize - 1;
  
  // page boundary or EINVAL
  char *p = (char *)(((long)m_pmem + offset) & ~page_mask);
  length += ((long)m_pmem + offset) & page_mask;
  
  if (offset >= m_size) 
  {
    stringstream ss;
    ss << "memfile offset out of bounds: " << offset << " >= " << m_size;
    throw SystemError( ss.str() );
  }
  
  if (p + length > (char *)m_pmem + m_size)
    length = m_size - (p - (char *)m_pmem);
  
  unsigned npages = (length + FileMemHolder::pagesize - 1) / FileMemHolder::pagesize;
  vec.resize(npages);
  fill(vec.begin(), vec.end(), 0);
  
  if (mincore((char *)p, length, (INCORE_ARR_T)&vec[0]) == -1) {
    throw SystemError("mincore");
  }
}
//---------------------------------------------------------------------------------

void
FileMemHolder::preload(off_t offset, off_t size)
{
  if (offset > size)
    return;
  if (offset + size > m_size)
    size = m_size - offset;
  
#if HAVE_READAHEAD
  readahead(m_fd, offset, size);
#else
  // now perform real prereading
  uint64_t sum = 0, *p = (uint64_t *)m_pmem + offset;
  for (size /= sizeof(uint64_t); size; size--)
    sum += *p++; 
  // hack wich guarantee what gcc will not optimize previous
  struct stat st;
  fstat((sum & 0xff) + 1024, &st);
#endif
}

int madvise_a( void *addr, size_t len, int behav )
{
  off_t off  = ( off_t )addr & ~( 4096 - 1 );
  size_t sz  = len + (( off_t )addr - off );

  if ( sz == 0 )
    return -1;

  (void)madvise(( void* )off, sz, behav );
  return 0;
}

} // namespace gogo
