//------------------------------------------------------------
/// @file   phrase_indexer.cpp
/// @brief  Qclassify phrase indexer
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   05.05.2009
//------------------------------------------------------------

#include <string>
#include <vector>

//#define PHRASE_INDEXER_DEBUG

#ifdef PHRASE_INDEXER_DEBUG
#include <stdio.h>
#  define DBG(x) {x;}
#else
#  define DBG(x) {}
#endif

#include "utils/hash_array.hpp"
#include "utils/ptr_array.hpp"
#include "qclassify.hpp"
#include "qclassify_impl.hpp"
#include "hashes/hashes.hpp"

using namespace std;
using namespace gogo::qcls_impl;

using gogo::PhraseRegExp::compressPCRE_flags;
using gogo::PhraseRegExp::str2PCRE_flags;

namespace gogo 
{
  
//------------------------------------------------------------------
/// @brief Phrase indexer implementation
class PhraseIndexerImpl : public QSerializerOut 
{
  /// @brief phrase information class:
  /// @brief store separate words
  class Phrase {
    vector<word_entry> m_words;
    vector<phrase_cls_info> m_classes;
    bool m_isRegexp;
    
    public:
      Phrase() : m_isRegexp(false) {}
      void setRegexp(bool isRegexp) { m_isRegexp = isRegexp; }
      void addWord(unsigned id, unsigned form_hash, bool upcased) {
        qcls_impl::word_entry we;
        we.id = id;
        we.form = form_hash;
        we.upcased = upcased ? 1 : 0;
        we.found = 0; // shut up valgrind!
        m_words.push_back(we);
      }
      
      void addClass(unsigned clsid, unsigned phrase_rank) {
        phrase_cls_info ci;
        ci.clsid = clsid;
        ci.phrase_rank = phrase_rank;
        m_classes.push_back(ci);
      }
      
      unsigned nwords() const { return m_words.size(); }
      
      
      // format of export:
      // [number of words(NW)][word_entry x NW][number of cls(NC)][phrase_cls_info x NC]
      size_t size() const {
        return sizeof(uint8_t)  + m_words.size() * sizeof(word_entry) + 
               sizeof(uint16_t) + m_classes.size() * sizeof(phrase_cls_info);
      }
      
      /// @brief save phrase
      void save(MemWriter &mwr) 
      {
        unsigned i;
        
        struct qcls_impl::phrase_record phrase_hdr;
        phrase_hdr.n = m_words.size();
        phrase_hdr.is_regexp = (int)m_isRegexp;
        phrase_hdr.__reserved = 0; // shut up valgrind!
        
        mwr << phrase_hdr;
        for (i = 0; i < m_words.size(); i++)
          mwr << m_words[i];
        
        mwr << (uint16_t)m_classes.size();
        for (i = 0; i < m_classes.size(); i++)
          mwr << m_classes[i];
      }
  };
  
  // exporting things
  mutable PtrArrayWriter<uint32_t> m_phrase_offsets;
  mutable size_t m_phrases_size;
  mutable HashArrayIndexer<word_hash_t, uint32_t> m_w2id_index;
  mutable HashArrayIndexer<uint32_t, uint32_t>    m_words2phrases;
  mutable bool m_bDirty;
  
  bool m_bSaveOrigPhrases;
  QCBasicPhraseStorage m_origPhrases;
  PhraseRegExpWriter m_regWriter;
  QCScatteredStringsWriter m_udataWriter;
  
  PhraseSplitterPlain m_splitterPlain;
  PhraseSplitterPCRE  m_splitterRE;
  
  map<word_hash_t, unsigned> m_w2id;
  vector< vector<unsigned> > m_wId2phrasesId;
  map<phrase_hash_t, unsigned> m_phrase2id;
  vector<Phrase> m_phrases;
  
  private:
    Phrase *insertPhraseWords(const std::string &phrase, unsigned phraseId);
    PhraseIndexer::stat m_stat;
    
  public:
    PhraseIndexerImpl() : m_bDirty(true), m_bSaveOrigPhrases(false) {};
    virtual ~PhraseIndexerImpl() {};
    void addPhrase(unsigned clsid, const std::string &phrase, 
                   unsigned rank, const char *udata);
    
    // export facilities
    void prepareExport() const;
    void optimize();
    virtual size_t size() const;
    virtual void   save(MemWriter &mwr);
    
  friend class PhraseIndexer;
};

//---------------------------------------------------------------------------------
/// @brief add phrase to index
/// @param cls phrase class
/// @param phrase phrase
/// @param rank phrase rank (0..255]
/// @param udata optional user string (used by clshtml)
void PhraseIndexer::addPhrase(unsigned clsid, const string &phrase, 
                              unsigned rank, const char *udata /* = NULL */)
{
  m_pimpl->addPhrase(clsid, phrase, rank, udata);
}

PhraseIndexer::PhraseIndexer(LemInterface *plem /* = NULL */) { 
  m_pimpl = new PhraseIndexerImpl; 
  setLemmatizer(plem);
}

PhraseIndexer::~PhraseIndexer() { delete m_pimpl; }

void PhraseIndexer::getStat(stat *st) const { *st = m_pimpl->m_stat; }
void PhraseIndexer::optimize() { m_pimpl->optimize(); }
size_t PhraseIndexer::size() const { return m_pimpl->size(); }
void PhraseIndexer::save(MemWriter &mwr) { m_pimpl->save(mwr); }
void PhraseIndexer::setLemmatizer(LemInterface *plem) { 
  m_pimpl->m_splitterPlain.setLemmatizer(plem); 
  m_pimpl->m_splitterRE.setLemmatizer(plem); 
}
void PhraseIndexer::saveOrigPhrases(bool bSave) { 
  m_pimpl->m_bSaveOrigPhrases = bSave; 
}
 

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Phrase indexer implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

void PhraseIndexerImpl::addPhrase(unsigned clsid, const string &phrase, 
                                  unsigned rank, const char *udata)
{
  map<phrase_hash_t, unsigned>::const_iterator it;
  phrase_hash_t h;
  Phrase *pphrase;
  
  MurmurHash(phrase, &h);
  
  // chech whatever such phrase already stored
  it = m_phrase2id.find(h);
  if (it == m_phrase2id.end()) {
    unsigned phraseId = m_stat.nphrases_uniq;
    pphrase = insertPhraseWords(phrase, phraseId);
    if (!pphrase)
      return;
    
    m_phrase2id.insert(pair<phrase_hash_t, unsigned>(h, phraseId));
    if (udata != NULL)
      m_udataWriter.add(phraseId, (string)udata);
    
    m_stat.nphrases_uniq++;
  }
  else {
    pphrase = &m_phrases[it->second];
    m_stat.nwords += pphrase->nwords();
  }
  
  m_stat.nphrases++;
  pphrase->addClass(clsid, rank);
  m_bDirty = true;
}

/// @brief split and insert phrase to prepareExport
/// @arg[in] phrase - source phrase
/// @return pointer to inserted phrase or NULL in case of empty phrase
PhraseIndexerImpl::Phrase *PhraseIndexerImpl::insertPhraseWords(const std::string &phrase, unsigned phraseId)
{
  string phrase_mod;
  unsigned i, nwords;
  vector<unsigned> wids;
  
  // select appropriate token splitter depending on phrase looks like RE (/.../) or not
  uint8_t  regexp_flags = 0;
  bool isRegexp;
  PhraseSplitterBase *pSplitter;
  {
    size_t pos;
    if (phrase[0] == '/' && (pos = phrase.rfind('/')) != 0) {
      phrase_mod = phrase.substr(1, pos - 1); // extracted RE
      pSplitter = &m_splitterRE;
      isRegexp  = true;
      
      regexp_flags = compressPCRE_flags(str2PCRE_flags(phrase.c_str() + pos + 1));
    } else {
      phrase_mod = phrase;
      pSplitter = &m_splitterPlain;
      isRegexp  = false;
    }
  }
  
  nwords = pSplitter->split(phrase_mod);
  if (!nwords)
    return NULL;
  
  m_stat.nwords += nwords;
  
  // map word-hash to word-id
  wids.reserve(nwords);
  for (i = 0; i < nwords; i++) {
    word_hash_t h = pSplitter->vWords[i].hash;
    map<word_hash_t, unsigned>::iterator it;
    
    it = m_w2id.find(h);
    if (it != m_w2id.end()) 
      wids[i] = it->second;
    else {
      // insert new word
      wids[i] = m_stat.nwords_uniq;
      m_w2id.insert(pair<word_hash_t, unsigned>(h, m_stat.nwords_uniq++));
      DBG( printf("ADD_WORD: 0x%08X:%d\n", pSplitter->vWords[i].hash, wids[i]) );
    } 
  }
  
  // balance word mapping:
  // phrase will be searched by one word
  // so select the less frequent one.
  m_wId2phrasesId.resize(m_stat.nwords_uniq);
  unsigned imin = 0, vmin = m_wId2phrasesId[ wids[0] ].size();
  
  for (i = 1; i < nwords; i++) {
    unsigned n = m_wId2phrasesId[ wids[i] ].size();
    if (n < vmin) {
      vmin = n;
      imin = i;
    }
  }
  
  unsigned keywordId = wids[imin];
  
  // add phrase id to "keyword" list
  m_wId2phrasesId[keywordId].push_back(phraseId);
  
  // store words (info) of phrase
  m_phrases.resize(m_phrases.size() + 1);
  Phrase &ph = m_phrases.back();
  ph.setRegexp(isRegexp);
  for (i = 0; i < nwords; i++) {
    ph.addWord(wids[i], pSplitter->vWords[i].form, pSplitter->vWords[i].upcase);
  }
  
  if (isRegexp) {
    // save regular expression (modified phrase)
    m_regWriter.add(phraseId, dynamic_cast<PhraseSplitterPCRE *>(pSplitter)->getModString(), regexp_flags);
    m_stat.nregexp++;
  }
  
  if (m_bSaveOrigPhrases)
    m_origPhrases.addPhrase(phrase_mod);
  
  return &ph;
}

/// @brief optimize phrase index for quicker retrieval
// As you can see, phrases are address by words and only.
// So, comparing with phrases containing that word (what's how phrase searcher actually works)
// will caught a great data cache miss since phrase records are scattered in address space.
// This optimization will reorder phrase records directly by word reference, apparently reducing
// CPU-L2d pitfalls while traversing throught
/// @warning optimization is completely optional (as like as transparent for phrase searcher)
void PhraseIndexerImpl::optimize()
{
  vector<unsigned> vShiftTbl;
  unsigned nPhrases = m_phrases.size();
  
  vShiftTbl.resize(nPhrases);
  
  {
    // fill shift table and patch m_wId2phrasesId mapping
    unsigned cnt = 0;
    for (vector< vector<unsigned> >::iterator w2p_it = m_wId2phrasesId.begin(); 
         w2p_it != m_wId2phrasesId.end(); 
         w2p_it++) 
     {
      vector<unsigned> &v = *w2p_it;
      for (unsigned i = 0; i < v.size(); i++) {
        vShiftTbl[ v[i] ] = cnt;
        v[i] = cnt++;
      }
    }
  }
  
  // patch m_phrase2id table (for allowing futural additions)
  { 
    map<phrase_hash_t, unsigned>::iterator it;
    for (it = m_phrase2id.begin(); it != m_phrase2id.end(); it++) {
      it->second = vShiftTbl[ it->second ];
    }
  }
  
  // reorder elements in m_phrases: it would be profligacy enought...
  {
    vector<Phrase> vPhrases;
    
    vPhrases.resize(nPhrases);
    for (unsigned i = 0; i < vShiftTbl.size(); i++) {
      vPhrases[ vShiftTbl[i] ] = m_phrases[i];
    }
    m_phrases.assign(vPhrases.begin(), vPhrases.end());
  }
  
  m_regWriter.optimize(vShiftTbl);
  m_origPhrases.optimize(vShiftTbl);
  m_udataWriter.optimize(vShiftTbl);
}

/// @brief build wordHash -> {phrasesId} array
void PhraseIndexerImpl::prepareExport() const
{
  if (!m_bDirty)
    return;
  
  assert(m_w2id.size() == m_stat.nwords_uniq);
  assert(m_wId2phrasesId.size() == m_stat.nwords_uniq);
  
  map<word_hash_t, unsigned>::const_iterator wh_it;
  
  // word hash to ID mapping
  m_w2id_index.clear();
  m_w2id_index.init(m_stat.nwords_uniq);
  for (wh_it = m_w2id.begin(); wh_it != m_w2id.end(); wh_it++) {
    m_w2id_index.add(wh_it->first, wh_it->second);
  }
  
  // word ID to phrases ID mapping (multy)
  m_words2phrases.clear();
  m_words2phrases.init(m_stat.nwords_uniq);
  
  for (wh_it = m_w2id.begin(); wh_it != m_w2id.end(); wh_it++) {
    unsigned word_id = wh_it->second;
    
    const vector<unsigned> &phrases_id = m_wId2phrasesId[word_id];
    for (unsigned i = 0; i < phrases_id.size(); i++) {
      m_words2phrases.add(word_id, phrases_id[i]);
    }
  }
  
  // build phrase offsets
  m_phrase_offsets.clear();
  m_phrases_size = 0;
  vector<Phrase>::const_iterator it;
  for (it = m_phrases.begin(); it != m_phrases.end(); it++) {
    m_phrase_offsets.push_back(m_phrases_size);
    m_phrases_size += it->size();
  }
  
  m_bDirty = false;
}

/// @brief compute space enought for export buffer
size_t PhraseIndexerImpl::size() const 
{
  prepareExport();
  return m_w2id_index.size() + m_words2phrases.size() +
      m_phrase_offsets.size() + sizeof(uint32_t) + m_phrases_size + 
      m_regWriter.size() + m_origPhrases.size() + m_udataWriter.size();
}

/// @brief export phrase storage
// export format: [WORD-HASH_TO_PHRASEID][PHRASES_OFFSETS][PHRASES]
void PhraseIndexerImpl::save(MemWriter &mwr) 
{
  prepareExport();
  
  m_w2id_index.save(mwr);
  m_words2phrases.save(mwr);
  m_phrase_offsets.save(mwr);
  
  mwr << (uint32_t)m_phrases_size;
  for (vector<Phrase>::iterator it = m_phrases.begin(); it != m_phrases.end(); it++)
    it->save(mwr);
  
  m_regWriter.save(mwr);
  m_origPhrases.save(mwr);
  m_udataWriter.save(mwr);
}

} // namespace gogo
