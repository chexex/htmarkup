//------------------------------------------------------------
/// @file   phrase_searcher.cpp
/// @brief  Qclassify phrase searcher
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   07.05.2009
//------------------------------------------------------------

#include <string>
#include <vector>
#include <iostream>

#include "hashes/hashes.hpp"
#include "utils/hash_array.hpp"
#include "utils/ptr_array.hpp"
#include "qclassify.hpp"
#include "qclassify_impl.hpp"

//#define PHRASE_SEARCHER_DEBUG

#ifdef PHRASE_SEARCHER_DEBUG
#include <stdio.h>
#  define DBG(x) {x;}
#else
#  define DBG(x) {}
#endif

using namespace std;
using namespace gogo::qcls_impl;

namespace gogo
{

//------------------------------------------------------------------
/// @brief phrase searcher implementation
class PhraseSearcherImpl : public QSerializerIn
{
  // following mutables present buffers
  // if you need re-enterant searchPhrase implementaion - put it in local area (slower)
  mutable vector<word_entry> m_match;
  mutable PhraseSplitterPlain m_splitter;
  
  PtrArrayReader<uint32_t, phrase_record> m_phrase_offsets;
  HashArraySearcher<word_hash_t, uint32_t> m_w2id_index;
  HashArraySearcher<uint32_t, uint32_t> m_words2phrases;
  PhraseRegExReader m_regReader;
  QCBasicPhraseReader m_origPhrases;
  QCScatteredStringsReader m_udataReader;
  
  private:
    inline int matchWords(const qcls_impl::word_entry *pwe, unsigned n) const;
    inline void processMatchingWithIDs(const string &s, vector<PhraseSearcher::phrase_matched> &phrases) const;
  
  public:
    virtual ~PhraseSearcherImpl() {}
    virtual void load(MemReader &mwr);
    
    /// @brief search phrase
    /// @arg[in] s - source phrase
    /// @arg[out] phrases - source phrase
    void searchPhrase(const string &s, vector<PhraseSearcher::phrase_matched> &phrases) const;
    
    friend class PhraseSearcher;
};
  
PhraseSearcher::PhraseSearcher(LemInterface *plem /* = NULL */) {
  m_pQCIndex = NULL;
  m_pimpl = new PhraseSearcherImpl;
  setLemmatizer(plem);
}

void PhraseSearcher::setLemmatizer(LemInterface *plem) {
  m_pimpl->m_splitter.setLemmatizer(plem);
}

PhraseSearcher::~PhraseSearcher() { delete m_pimpl; }
void PhraseSearcher::load(MemReader &mrd) {  m_pimpl->load(mrd); }

unsigned PhraseSearcher::searchPhrase(const string &s, vector<phrase_matched> &phrases) const
{
  m_pimpl->searchPhrase(s, phrases);
  return phrases.size();
}

void PhraseSearcher::setQCIndex(QCIndexReader *pQCIndex) { 
  m_pQCIndex = pQCIndex;
}

const char *PhraseSearcher::getOriginPhrase(unsigned phraseid) const
{
  const char *res;
  try {
    res = m_pimpl->m_origPhrases.getPhrase(phraseid);
  } catch(std::out_of_range) {
    res = NULL;
  }
  
  return res;
}

const char *PhraseSearcher::getUserData(unsigned phraseid) const {
  return m_pimpl->m_udataReader.get(phraseid);
}

const std::string &PhraseSearcher::getClassName(unsigned clsid) const {
  if (!m_pQCIndex)
    throw std::runtime_error("PhraseSearcher: query class index uninitialized");
  
  return m_pQCIndex->getName(clsid);
}

inline unsigned PhraseSearcher::applyPenalties(unsigned clsid, unsigned base, int flags) const
{
  double rank = (double)base;
  
  assert(rank != -1);
  
  const QCPenalties &pens = m_pQCIndex->getPenalties(clsid);
  rank *= pens.baseRank;
  if (flags & MATCH_FL_REORDERED)
    rank *= pens.reorder_penalty;
  if (flags & MATCH_FL_DIFF_FORM)
    rank *= pens.diff_form_penalty;
  if (flags & MATCH_FL_PARTIAL)
    rank *= pens.partial_penalty;
  if (flags & MATCH_FL_DIFF_CAPS)
    rank *= pens.diff_caps_penalty;
  
  return (unsigned)rank;
}

/// @brief search for phrase and return map of class_id to {phrase_id,rank}
/// @arg[in] s - phrase to match
/// @arg[out] res - class_id -> rank map
unsigned PhraseSearcher::searchPhrase(const std::string &s, res_cls_num_t &res) const
{
  res.clear();
  if (!m_pQCIndex) { // need for penalties accounting
    return 0;
  }
  
  vector<phrase_matched> phrasesIds;
  m_pimpl->searchPhrase(s, phrasesIds);
  unsigned nres = phrasesIds.size();
  if (!nres)
    return 0;
  
  unsigned i, j, clsid;
  res_cls_num_t::iterator rit;
  
  phrase_info ph_info;
  
  phrase_record *phrec;
  phrase_classes_list *pclassList;
  
  for (i = 0; i < nres; i++) 
  {
    ph_info.phrase_id = phrasesIds[i].phrase_id;
    phrec = m_pimpl->m_phrase_offsets[ ph_info.phrase_id ];
    pclassList = __phrase_header_jump_to_classes(phrec);
    
    for (j = 0; j < pclassList->n; j++) {
      clsid = pclassList->clse[j].clsid;
      ph_info.rank  = applyPenalties(clsid, pclassList->clse[j].phrase_rank, phrasesIds[i].match_flags);
      
      if (ph_info.rank) {
        rit = res.find(clsid);
        if (rit == res.end())
          res.insert(pair<unsigned, phrase_info>(clsid, ph_info));
        else if (rit->second.rank < ph_info.rank) {
          rit->second = ph_info;
        }
      }
    }
  }
  
  return res.size();
}

/// @brief distribute matched phrase by classIDs
/// @param phrases vector with matched phrases [in]
/// @param phraseByCls classID to phrase_matched multimap [out]
void PhraseSearcher::getClasses(const vector<phrase_matched> &phrases, 
                                std::multimap<unsigned, phrasecls_matched> &phraseByCls) const
{
  phraseByCls.clear();
  
  phrasecls_matched mi;
  phrase_record *phrec;
  phrase_classes_list *pclassList;
  
  for (vector<phrase_matched>::const_iterator it = phrases.begin();
       it != phrases.end();
       it++) 
  {
    phrec = m_pimpl->m_phrase_offsets[ it->phrase_id ];
    pclassList = __phrase_header_jump_to_classes(phrec);
    
    mi.phrase_id = it->phrase_id;
    mi.match_flags = it->match_flags;
    
    for (unsigned i = 0; i < pclassList->n; i++) {
      mi.baserank = pclassList->clse[i].phrase_rank;
      phraseByCls.insert(pair<unsigned, phrasecls_matched>(pclassList->clse[i].clsid, mi));
    }
  }
}

/// @brief distribute matched phrase by names
/// @param phrases vector with matched phrases [in]
/// @param phraseByCls classID to phrase_matched multimap [out]
// this impementatin doesn't use getClasses (by ids) since huge multimap overhead
void PhraseSearcher::getClasses(const vector<phrase_matched> &phrases, 
                                std::multimap<string, phrasecls_matched> &phraseByCls) const
{
  phraseByCls.clear();
  
  phrasecls_matched mi;
  phrase_record *phrec;
  phrase_classes_list *pclassList;
  
  for (vector<phrase_matched>::const_iterator it = phrases.begin();
       it != phrases.end();
       it++) 
  {
    phrec = m_pimpl->m_phrase_offsets[ it->phrase_id ];
    pclassList = __phrase_header_jump_to_classes(phrec);
    
    mi.phrase_id = it->phrase_id;
    mi.match_flags = it->match_flags;
    
    for (unsigned i = 0; i < pclassList->n; i++) {
      mi.baserank = pclassList->clse[i].phrase_rank;
      phraseByCls.insert(pair<string, phrasecls_matched>(m_pQCIndex->getName(pclassList->clse[i].clsid), mi));
    }
  }
}

/// @brief search for phrase and return map of class_id to rank
/// @arg[in] s - phrase to match
/// @arg[out] res - class_id -> rank map
unsigned PhraseSearcher::searchPhrase(const std::string &s, res_num_t &res) const
{
  res.clear();
  if (searchPhrase(s, m_bufresult)) {
    for (res_cls_num_t::iterator it = m_bufresult.begin(); it != m_bufresult.end(); it++) {
      res.insert(pair<unsigned, unsigned>(it->first, it->second.rank));
    }
    return res.size();
  }
  return 0;
}

/// @brief search for phrase and return map of class name to phrase rank
/// @arg[in] s - phrase to match
/// @arg[out] res - class_name -> rank map
unsigned PhraseSearcher::searchPhrase(const std::string &s, res_t &res) const
{
  res.clear();
  if (searchPhrase(s, m_bufresult)) {
    for (res_cls_num_t::iterator it = m_bufresult.begin(); it != m_bufresult.end(); it++) {
      res.insert(pair<string, unsigned>(m_pQCIndex->getName(it->first), it->second.rank));
    }
    return res.size();
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Phrase searcher implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief read saved phrase index
// you can see format in phrase_indexer.cpp
void PhraseSearcherImpl::load(MemReader &mrd) 
{
  m_w2id_index.load(mrd);
  m_words2phrases.load(mrd);
  m_phrase_offsets.load(mrd);
  
  // remember phrase region base and skip it
  uint32_t phrase_region_sz;
  mrd >> phrase_region_sz;
  m_phrase_offsets.setBase(mrd.get());
  mrd.advance(phrase_region_sz);
  m_regReader.load(mrd);
  m_origPhrases.load(mrd);
  m_udataReader.load(mrd);
}


/// @brief compare words with original phrase
/// @arg[in] pwe - word_entry array pointer
/// @arg[in] n - number of words
/// @return matching (penalty) flags or (-1) if not matched
inline int PhraseSearcherImpl::matchWords(const qcls_impl::word_entry *pwe, unsigned n) const
{
  if (m_match.size() < n) {
    // no matching is possible, at least one word from indexed phrase absent in query
    return (-1);
  }
  
  int match_mask = (n == m_match.size()) ? 0 : PhraseSearcher::MATCH_FL_PARTIAL;  
  unsigned i, j, prev_pos = (1 << 31);
  
  /* Prevent situation when word used twice
   * For example:
   *  template: go far go
   *  query: go far
   *
   * query should not satisfy template in this case
   */
  unsigned wmask_used = 0x0;

  for (i = 0; i < n; i++) 
  {
    // Use simple(brute force) search, since number of words is quite small
    for (j = 0; j < m_match.size(); j++) 
    {
      if (m_match[j].found && pwe[i].id == m_match[j].id && !(wmask_used & (1 << j)))
      {
        DBG( printf("COMPARE: [wid=%d]; upcase = %d; upcase_orig = %d\n", 
             pwe[i].id, m_match[j].upcased?1:0, pwe[i].upcased?1:0));
        
        if (!(prev_pos & (1 << 31)) && prev_pos+1 != j) 
          match_mask |= PhraseSearcher::MATCH_FL_REORDERED;
        if (m_match[j].form != pwe[i].form) 
          match_mask |= PhraseSearcher::MATCH_FL_DIFF_FORM;
        if (m_match[j].upcased != pwe[i].upcased)
          match_mask |= PhraseSearcher::MATCH_FL_DIFF_CAPS;

        wmask_used |= (1 << j);
        prev_pos = j;
        goto word_found;
      }
    }

    return (-1); // query doesn't contain this word
    word_found:
        ;
  }

  return match_mask;
}

/// @brief process with phrase matching:
/// @arg[out] phrases - phraseID:flags pair
inline void PhraseSearcherImpl::processMatchingWithIDs(const string &s, vector<PhraseSearcher::phrase_matched> &phrases) const
{
  vector<uint32_t> vPhraseIds;
  unsigned i, j, n;
  PhraseSearcher::phrase_matched match_res;
  
  DBG( printf("+processMatchingWithIDs: %s\n", s.c_str()));
  
  for(i = 0; i < m_match.size(); i++) 
  {
    qcls_impl::word_entry &w = m_match[i];
    if (!w.found)
      continue;
    
    // match with every phrase containing this word
    n = m_words2phrases.search(w.id, vPhraseIds);
    DBG( printf("+m_words2phrases.search(%u)=%u\n", w.id, n));
    for (j = 0; j < n; j++) 
    {
      match_res.phrase_id   = vPhraseIds[j];
      const phrase_record *phrec  = m_phrase_offsets[match_res.phrase_id];
      match_res.match_flags = matchWords(phrec->words, phrec->n);
      
      DBG( printf("+match with phrase: %d; flags=%02X\n", match_res.phrase_id, match_res.match_flags));
      // ckeck regular expression matching if phrase is RE
      if (match_res.match_flags != -1 && (!phrec->is_regexp || 
          m_regReader.match(match_res.phrase_id, s) > 0)) 
      {
        phrases.push_back(match_res);
      }
    }
  }
}

void PhraseSearcherImpl::searchPhrase(const string &s, vector<PhraseSearcher::phrase_matched> &phrases) const
{ 
  phrases.clear();
  unsigned nwords = m_splitter.split(s);
  DBG( printf("+NWORDS: %d\n", nwords));
  if (!nwords)
    return;
  
  m_match.resize(nwords);
  for(unsigned i = 0; i < nwords; i++) {
    uint32_t id;
    PhraseSplitterPlain::word_info &wi = m_splitter.vWords[i];
    word_entry &ma = m_match[i];
    
    if (m_w2id_index.search(wi.hash, id)) {
      ma.id = id;
      ma.found = 1;
      ma.upcased = wi.upcase & 0x1;
      ma.form = wi.form;
    }
    else {
      ma.found = 0;
    }
    
    DBG( printf("+WORD: [%d]; id=%d; upcased=%d\n", ma.found ? 1 : 0, (int)ma.id, ma.upcased ? 1 : 0) );
  }
  
  processMatchingWithIDs(s, phrases);
}

} // namespace gogo
