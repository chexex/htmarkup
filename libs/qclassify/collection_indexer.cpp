//------------------------------------------------------------
/// @file   collection_indexer.cpp
/// @brief  phrase files indexer
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   08.05.2009
//------------------------------------------------------------

#include <cstdlib>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <errno.h>

#include "defs.hpp"
#include "utils/memio.hpp"
#include "utils/stringutils.hpp"
#include "qclassify.hpp"
#include "qclassify_impl.hpp"


namespace gogo
{

PhraseCollectionIndexer::PhraseCollectionIndexer(LemInterface *plem /* = NULL */) : m_optimizeIndex(true), quiet_(false)
{
    setLemmatizer(plem);
}

void PhraseCollectionIndexer::setLemmatizer(LemInterface *plem) {
    m_phraseIndexer.setLemmatizer(plem);
}

void PhraseCollectionIndexer::addPhrase(unsigned cls, const std::string &phrase, 
                                        unsigned rank, const char *udata)
{
    m_phraseIndexer.addPhrase(cls, phrase, rank, udata);
}

/// @brief add phrases from input stream
void PhraseCollectionIndexer::addFile(unsigned cls, std::istream &is)
{
  std::string s;
  
  while(!is.eof()) 
  {
    std::getline(is, s);
    if (s.length() != 0) 
    {
      unsigned rank = 100;
      std::string udata;
      
      size_t pos = s.find("//");
      if (pos != std::string::npos)
      {
        char *ech = NULL;
        const char *rank_pos = s.c_str() + pos + 2;
        while (*rank_pos == ' ' || *rank_pos == '\t')
            rank_pos++; // skip spaces after "//"

        if (*rank_pos != '\0') {
            long int i = strtol(rank_pos, &ech, 10);
            if (*ech == '\0' || *ech == ';' || *ech == '%') {
              // it seems to be phrase rank declaration
              rank = (unsigned)i;
            }
        }

        if (ech) {
            // look for user-data
            while (*ech && *ech != ';')
                ech++;

            if (*ech == ';')
              udata.assign(ech + 1);
        }

        s.erase(pos);
      }

      if (trim_str(s))
        addPhrase(cls, s, rank, udata.empty() ? NULL : udata.c_str());
    }
  }
}

/// @brief save phrase collection; first to  memory area, then to files
/// @arg[in] path - path of file to save
void PhraseCollectionIndexer::save(const char *path /* = NULL */)
{
  size_t sz;

  std::stringstream ss_null;
  std::ostream &logstream = (quiet_) ? ss_null : std::cerr;
  
  if (!path)
    path = m_idxpath.c_str();
  
  PhraseIndexer::stat st;
  m_phraseIndexer.getStat(&st);

  
  logstream << "===============================================\n";
  logstream << "Phrase statistics:\n";
  logstream << "  - " << m_qcIndexer.amount() << " classes (files)\n";
  logstream << "  - " << st.nwords << " words (" << st.nwords_uniq << " uniq)\n";
  logstream << "  - " << st.nphrases << " phrases (" << st.nphrases_uniq << " uniq)\n";
  logstream << "  - " << st.nregexp << " regular expressions\n";
  logstream << "===============================================\n\n";
  
  if (m_optimizeIndex) {
    logstream << "Optimizing phrase index...\n";
    m_phraseIndexer.optimize();
  }

  sz = m_qcIndexer.size() + m_phraseIndexer.size();
  logstream << "Preparing phrase index to export...\n";
  logstream << "Saving(" << (unsigned)(sz >> 10) << "Kb)\n";
  
  auto_ptr_arr<char> region(new char[sz]);
  MemWriter mwr(region.get());
  
  m_qcIndexer.save(mwr);
  m_phraseIndexer.save(mwr);
  
  std::ofstream of(path, std::ios::out | std::ios::trunc);
  if (!of.is_open()) {
    std::stringstream ss;
    ss << "Failed to open file \"" << path << "\": " << strerror(errno);
    throw std::runtime_error(ss.str());
  }
  
  qcls_impl::phrase_file_header hdr;
  of.write((const char *)&hdr, sizeof(hdr));
  of.write(region.get(), sz);
  of.close();
}

void PhraseCollectionIndexer::saveOrigPhrases(bool bSave)
{
  m_phraseIndexer.saveOrigPhrases(bSave);
}

/// @brief add classes and phrase files referenced by config
void PhraseCollectionIndexer::indexByConfig(const XmlConfig *pcfg)
{
  quiet_ = pcfg->GetBool( "QueryQualifier", "Quiet", false );
  m_qcIndexer.addConfig(pcfg);
  unsigned n = m_qcIndexer.amount(), i;

  std::stringstream ss_null;
  std::ostream &logstream = (quiet_) ? ss_null : std::cerr;
  
  pcfg->GetStr("QueryQualifier", "IndexFile", m_idxpath, "phrases.idx");
  m_optimizeIndex = pcfg->GetBool("QueryQualifier", "OptimizeIndex", m_optimizeIndex);
  
  bool bSave = pcfg->GetBool("QueryQualifier", "SaveOrigins", false);
  saveOrigPhrases(bSave);
  
  logstream << "\nindexing by config file\n";
  for (i = 0; i < n; i++) {
    std::string qcname = m_qcIndexer.getName(i);
    const char *path = pcfg->GetStr(gogo::QueryClassifierHelper::QCname2XMLtag(qcname).c_str(), "PhrasesFile");
    if (!path) {
      std::stringstream ss;
      ss << "PhraseCollectionIndexer: no `PhrasesFile' param for \"" << qcname << "\"";
      throw std::runtime_error(ss.str());
    }
    
    std::ifstream is(path, std::ios::in);
    if (!is.is_open()) {
      std::stringstream ss;
      ss << "PhraseCollectionIndexer: failed to open file \"" << path << "\"";
      throw std::runtime_error(ss.str());
    }

    logstream << "\rIndexing file " << (i+1) << "/" << n << "\r";
    
    addFile(i, is);
    is.close();
  }
  
  logstream << std::endl;
}

} // namespace gogo
