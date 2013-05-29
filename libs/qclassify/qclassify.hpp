//------------------------------------------------------------
/// @file   qclassify.hpp
/// @brief  qclassify suite main header (global namespace: gogo)
/// @date   05.05.2009
//------------------------------------------------------------

#ifndef GOGO_QCLASSIFY_HPP__
#define GOGO_QCLASSIFY_HPP__

#include <sys/types.h>
#include <stdint.h>

#include <string>
#include <memory>
#include <vector>

#include "defs.hpp"
#include "lem_interface/lem_interface.hpp"

#include "config/config.hpp"
#include "utils/ptr_array.hpp"
#include "utils/memfile.hpp"
#include "utils/memio.hpp"
#include "utils/hash_array.hpp"
#include "utils/fileutils.hpp"


namespace gogo {
  

class QCBasicPhraseStorage : public QSerializerOut 
{
  mutable size_t m_acclen;
  std::vector<std::string> m_phrases;
  mutable PtrArrayWriter<uint32_t> m_offsets;
  mutable bool m_bDirty;
  
  public:
    QCBasicPhraseStorage();
    virtual ~QCBasicPhraseStorage() {};
    
    void prepareExport() const;
    
    void optimize(const std::vector<unsigned> &vshift);
    void addPhrase(const std::string &s);
    // you can redefine this for some unusual things with string
    virtual void preparePhrase(__UNUSED(std::string &s)) {};
    // export facility
    virtual size_t size() const;
    virtual void   save(MemWriter &mwr);
};

class QCBasicPhraseReader : public QSerializerIn
{
  PtrArrayReader<uint32_t> m_offsets;
  const char* m_phrases;
  public:
    virtual ~QCBasicPhraseReader() {};
    const char *getPhrase(unsigned i);
    // import facility
    virtual void load(MemReader &mrd);
};



class QCScatteredStringsWriter : public QSerializerOut
{
  std::map<unsigned, std::string> m_stings;
  mutable size_t m_acclen;
  mutable bool m_bDirty;
  mutable HashArrayIndexer<uint32_t, uint32_t> m_id2offset;
  
  public:
    QCScatteredStringsWriter();
    virtual ~QCScatteredStringsWriter() {};
    void add(unsigned id, const std::string &s);
    void optimize(const std::vector<unsigned> &vshift);
    void prepareExport() const;
    
    // export facility
    virtual size_t size() const;
    virtual void   save(MemWriter &mwr);
};

class QCScatteredStringsReader : public QSerializerIn
{
  const char *m_pBase;
  HashArraySearcher<uint32_t, uint32_t> m_id2offset;
  
  public:
    virtual ~QCScatteredStringsReader() {};
    /// @brief  retrieve source string
    /// @param id saved string identifyer
    /// @return string pointer or NULL unless found
    const char *get(unsigned id) const throw();
    // import facility
    virtual void load(MemReader &mrd);
};



  
struct QCPenalties {
  double reorder_penalty;
  double partial_penalty;
  double diff_form_penalty;
  double diff_caps_penalty;
  double baseRank;
  
  static double penaltiToMultiplier(double x) { return 1.0 - x; }
  QCPenalties() : reorder_penalty(1.0), partial_penalty(1.0), 
                  diff_form_penalty(1.0), diff_caps_penalty(1.0), baseRank(1.0) {}
} __PACKED;

//
// Query class index writer
//
class QCIndexWriter : public QSerializerOut 
{
  struct QCClass {
    std::string name;
    QCPenalties pens;
  };
  
  std::vector<QCClass> m_classes;
  
  public:
    QCIndexWriter();
    virtual ~QCIndexWriter() {};
    
    void addConfig(const XmlConfig *pcfg);
    void addQClass(const std::string &name, const QCPenalties &pens);
    size_t amount() const { return m_classes.size(); }
    const std::string& getName(unsigned id) const {
      return m_classes[id].name;
    }
    
    // export facilities
    virtual size_t size() const;
    virtual void   save(MemWriter &mwr);
};

//
// Query class index reader
//
class QCIndexReader : public QSerializerIn
{
  struct QCClass {
    std::string name;
    QCPenalties pens;
  }; 
  std::vector<QCClass> m_classes;
  
  public:
    virtual ~QCIndexReader() {};
    
    void mergeConfig(const XmlConfig *pcfg);
    size_t amount() const { return m_classes.size(); }
    const QCPenalties& getPenalties(unsigned id) const {
      return m_classes[id].pens;
    }
    const std::string& getName(unsigned id) const {
      return m_classes[id].name;
    }
    
    // import facilities
    virtual void load(MemReader &mrd);
};

class PhraseIndexerImpl;

//
// Phrase index writer
//
class PhraseIndexer : public QSerializerOut
{
  PhraseIndexerImpl *m_pimpl;
  
  public:
    struct stat {
      unsigned nwords;
      unsigned nphrases;
      unsigned nwords_uniq;
      unsigned nphrases_uniq;
      unsigned nregexp;
      
      stat() : nwords(0), nphrases(0), nwords_uniq(0), nphrases_uniq(0), nregexp(0) {}
    };
  
  public:
    PhraseIndexer(lemInterface *plem = NULL);
    virtual ~PhraseIndexer();
    void setLemmatizer(lemInterface *plem);
    
    //---------------------------------------------------------------------------------
    /// @brief save original phrases
    /// @param bSave trigger
    void saveOrigPhrases(bool bSave);
    
    //---------------------------------------------------------------------------------
    /// @brief add phrase to index
    /// @param cls phrase class
    /// @param phrase phrase
    /// @param rank phrase rank (0..255]
    /// @param udata optional user string (used by clshtml)
    void addPhrase(unsigned cls, const std::string &phrase, 
                   unsigned rank, const char *udata = NULL);
    
    void getStat(stat *st) const;
    void optimize();
    
    // export facilities
    virtual size_t size() const;
    virtual void   save(MemWriter &mwr);
};


class PhraseSearcherImpl;

//
// Phrase searcher using saved index
//
class PhraseSearcher : public QSerializerIn
{
  public:
    enum {
      MATCH_FL_REORDERED = 0x01,
      MATCH_FL_DIFF_FORM = 0x02,
      MATCH_FL_PARTIAL   = 0x04,
      MATCH_FL_DIFF_CAPS = 0x08
    };
    
    void setQCIndex(QCIndexReader *pQCIndex);
    
    typedef std::map<std::string, unsigned> res_t;  // phrase class name to rank
    typedef std::map<unsigned, unsigned> res_num_t; // phrase class ID to rank
    
    struct phrase_matched {
      unsigned phrase_id;
      int match_flags;
    };
    
    struct phrasecls_matched {
      unsigned phrase_id;
      int match_flags;
      unsigned baserank;
    };
    
    struct phrase_info {
      unsigned phrase_id;
      unsigned rank;
    };
    
    typedef std::map<unsigned, phrase_info> res_cls_num_t; // phrase class ID to phrase_info
    
     
    PhraseSearcher(lemInterface *plem = NULL);
    void setLemmatizer(lemInterface *plem);
    
    /// @brief distribute matched phrase by class IDs
    /// @param phrases vector with matched phrases [in]
    /// @param phraseByCls classID to phrase_matched multimap [out]
    void getClasses(const std::vector<phrase_matched> &phrases, 
                    std::multimap<unsigned, phrasecls_matched> &phraseByCls) const;
    
    /// @brief distribute matched phrase by class names
    /// @param phrases vector with matched phrases [in]
    /// @param phraseByCls classID to phrase_matched multimap [out]
    void getClasses(const std::vector<phrase_matched> &phrases, 
                    std::multimap<std::string, phrasecls_matched> &phraseByCls) const;
    
    unsigned searchPhrase(const std::string &s, std::vector<phrase_matched> &phrases) const;
    unsigned searchPhrase(const std::string &s, res_cls_num_t &res) const;
    unsigned searchPhrase(const std::string &s, res_num_t &res) const;
    unsigned searchPhrase(const std::string &s, res_t &res) const;
    
    static res_cls_num_t::iterator selectBest(PhraseSearcher::res_cls_num_t &r);
    static res_num_t::iterator selectBest(PhraseSearcher::res_num_t &r);
    static res_t::iterator selectBest(PhraseSearcher::res_t &r);
    
    // import facilities
    virtual void load(MemReader &mrd);
    virtual ~PhraseSearcher();
    
    // compatibility functions
    int classify_phrase(const std::string &s, res_t &res, __UNUSED(bool __unused = false)) const { 
      return (int)searchPhrase(s, res);
    }
    
  // origins retrieval
  // handle with care since we give pointers without copying
  public:
    const char *getOriginPhrase(unsigned phraseid) const;
    const char *getUserData(unsigned phraseid) const;
    const std::string &getClassName(unsigned clsid) const;
    const QCIndexReader &getQCIndex() const { return *m_pQCIndex; }
    
  private:
    mutable res_cls_num_t m_bufresult;
    PhraseSearcherImpl *m_pimpl;
    QCIndexReader *m_pQCIndex;
  
    inline unsigned applyPenalties(unsigned clsid, unsigned base, int flags) const;
};


//
// Phrase files index writer
//
class PhraseCollectionIndexer
{
  PhraseIndexer m_phraseIndexer;
  QCIndexWriter m_qcIndexer;
  std::string   m_idxpath;
  bool m_optimizeIndex;
  
  FileMemHolder m_idxfile;
  bool quiet_;
  
  public:
    PhraseCollectionIndexer(lemInterface *plem = NULL);
    void setLemmatizer(lemInterface *plem);
    
    void indexByConfig(const XmlConfig *pcfg);
    void addFile(unsigned cls, std::istream &is);
    void addPhrase(unsigned cls, const std::string &phrase, 
                   unsigned rank, const char *udata);
    void saveOrigPhrases(bool bSave);
    
    void save(const char *path = NULL);
};


//
// Phrase files index loader (several modes: heap/mmap(file), ...)
//
class PhraseCollectionLoader
{
  std::auto_ptr<PhraseSearcher> m_psearcher;
  std::auto_ptr<QCIndexReader> m_qcreader;
  FileMemHolder m_idxfile;
  
  lemInterface   *m_plem;
  PhraseSearcher m_emptySearcher;
  bool quiet_;
  
  public:
    PhraseCollectionLoader(lemInterface *plem = NULL);
    void setLemmatizer(lemInterface *plem);
    
    bool loadFile(const char *path, bool bmmap = false, bool bmlock = false);
    bool loadByConfig(const XmlConfig *pcfg);
    
    bool is_loaded() const { return m_psearcher.get() != NULL; }
    bool reload() { return true; } /// @todo: write me
    
    // you can use phrase searcher directly or throught `->' of this class
    const PhraseSearcher *getSearcher() const { return (m_psearcher.get()) ? m_psearcher.get() : (&m_emptySearcher); }
    const PhraseSearcher *operator->() const { return getSearcher(); }
    
    const QCIndexReader &getQCIndex();
};

} // namespace gogo

/// @deprecated phrase_classifier alias
typedef gogo::PhraseSearcher phrase_classifier;

#endif // GOGO_QCLASSIFY_HPP__
