//------------------------------------------------------------
/// @file   collection_loader.cpp
/// @brief  phrase collection loader
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   08.05.2009
//------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

#include <iostream>
#include <sstream>

#include <string>

#include "utils/memfile.hpp"
#include "utils/memio.hpp"
#include "qclassify.hpp"
#include "qclassify_impl.hpp"

namespace gogo
{
  
PhraseCollectionLoader::PhraseCollectionLoader(lemInterface *plem /* = NULL */) : 
  m_plem(plem), quiet_(false)
{
  
}


void PhraseCollectionLoader::setLemmatizer(lemInterface *plem) {
  m_plem = plem;
  if (m_psearcher.get()) 
    m_psearcher->setLemmatizer(m_plem);      
}

const QCIndexReader &PhraseCollectionLoader::getQCIndex() { 
  return *(m_qcreader.get());
}

/// @brief load phrase index file
/// @arg[in] path - file path
// We should not throw any exceptions here
bool PhraseCollectionLoader::loadFile(const char *path, 
                                      bool bmmap /* = false */, bool bmlock /* = false */)
{
  std::stringstream ss_null;
  std::ostream &logstream = (quiet_) ? ss_null : std::cerr;

  logstream << "PhraseCollectionLoader: loading \"" << path << "\"\n";
  
  m_idxfile.setExceptions(0);
  if (!m_idxfile.load(path, bmmap, bmlock)) {
    std::cerr << "PhraseCollectionLoader: failed to load " << path << std::endl;
    return false;
  }
  
  if ((size_t)m_idxfile.size() < sizeof(qcls_impl::phrase_file_header)) {
    std::cerr << "Index file size (" << m_idxfile.size() 
        << ") smaller than header (" << sizeof(qcls_impl::phrase_file_header) << ")\n";
    return false;
  }
  
  qcls_impl::phrase_file_header *hdr = reinterpret_cast<qcls_impl::phrase_file_header *>(m_idxfile.get());
  if (hdr->version != qcls_impl::QCLASSIFY_INDEX_VERSION) {
    std::cerr << "PhraseCollectionLoader: mismatched versions; self= " << 
        qcls_impl::QCLASSIFY_INDEX_VERSION <<  "; index file=" << hdr->version << std::endl;
    return false;
  }
  
  if ((size_t)m_idxfile.size() == sizeof(qcls_impl::phrase_file_header)) {
    std::cerr << "epmty-data phrases file (" << path << ")\n";
    return true;
  }

  
  try {
    void *pdata = reinterpret_cast<void *>(((char *)hdr + sizeof(qcls_impl::phrase_file_header)));
    MemReader mrd(pdata);
    
    m_qcreader.reset(new QCIndexReader);
    m_qcreader->load(mrd);
    
    m_psearcher.reset(new PhraseSearcher(m_plem));
    m_psearcher->load(mrd);
    m_psearcher->setQCIndex(m_qcreader.get());
  }
  catch (std::exception &e) {
    std::cerr << "PhraseCollectionLoader: exception while loading: " << e.what() << std::endl;
    return false;
  }
  
  logstream << "PhraseCollectionLoader: \"" << path << "\" successfully loaded (" <<
      (m_idxfile.size() >> 10) << "K)\n"  << std::endl;
  
  return true;
}

bool PhraseCollectionLoader::loadByConfig(const XmlConfig *pcfg)
{
  quiet_ = pcfg->GetBool( "QueryQualifier", "Quiet", false );
  
  std::string idxpath;
  bool bmmap  = pcfg->GetBool("QueryQualifier", "MMaped", false),
       bmlock = pcfg->GetBool("QueryQualifier", "MLocked", false);
  
  pcfg->GetStr("QueryQualifier", "IndexFile", idxpath, "phrases.idx");
  return loadFile(idxpath.c_str(), bmmap, bmlock);
}

} // namespace gogo
