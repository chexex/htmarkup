//------------------------------------------------------------
/// @file   qclassify_impl.hpp
/// @brief  qclassify suite implementation main header (global namespace: gogo)
/// @date   12.05.2009
//------------------------------------------------------------

#ifndef GOGO_QCLASSIFY_IMPL_HPP__
#define GOGO_QCLASSIFY_IMPL_HPP__

#include <stdint.h>
#include <sys/types.h>

#include <cstring> // memset
#include <string>
#include <map>
#include <vector>
#include <stdint.h>
#include <pcre.h>

#include "icuincls.h"
#include "defs.hpp"
#include <Interfaces/cpp/LemInterface.hpp>
#include "utils/memio.hpp"

namespace gogo 
{
  
//
// Query class index writer
//
namespace QueryClassifierHelper {
  static inline std::string QCname2XMLtag(const std::string &name) { 
    return ("QueryClass_" + name); 
  }
  static inline std::string XMLtag2QCname(const std::string &tag) { 
    return tag.substr(sizeof("QueryClass_") - 1);
  }
};

namespace qcls_impl 
{
  typedef uint64_t phrase_hash_t;
  typedef uint32_t word_hash_t;
  
  static const uint16_t QCLASSIFY_INDEX_VERSION = 10;
  
  struct word_entry {
    uint32_t id:22;
    int upcased:1;
    int found:1;
    uint8_t form;
  } __PACKED;
  
  struct phrase_cls_info {
    uint32_t clsid:24;
    uint8_t  phrase_rank;
  } __PACKED;
    
  // runtime structs
  struct phrase_record {
    uint8_t    n:5;
    uint8_t    is_regexp:1;
    uint8_t    __reserved:2;
    word_entry words[0];
  } __PACKED;
  
  struct phrase_classes_list {
    uint16_t n; // number of classes wich match phrase
    phrase_cls_info clse[0];
  } __PACKED;
  
  /// @brief inlined jump from phrase header to it's class list record
  static inline phrase_classes_list* __phrase_header_jump_to_classes(phrase_record *pr) {
    return (phrase_classes_list*)((char *)pr + (sizeof(phrase_record) + sizeof(word_entry) * pr->n));
  }
  
  // phrases file header of size 64 bytes
  struct phrase_file_header {
    uint16_t version;
    char __reserved[62];
    phrase_file_header() : version(QCLASSIFY_INDEX_VERSION) { 
      memset(__reserved, 0, sizeof(__reserved)); 
    }
  } __PACKED;
}


class PhraseSplitterBase 
{
  LemInterface *m_plem;
  protected:
    void addWord(const char *w, int len);
    void addWord(const UnicodeString &us);
    virtual void splitPhrase(const std::string &phrase) = 0;
    
  public:
    struct word_info {
      qcls_impl::word_hash_t hash;
      uint32_t  form;
      bool    upcase;
    };
  
    std::vector<word_info> vWords;
    
  public:
    PhraseSplitterBase(LemInterface *plem = NULL);
    void setLemmatizer(LemInterface *plem);
    unsigned split(const std::string &s);
    virtual ~PhraseSplitterBase() {}
};

//
// Splitting of plain text
//
class PhraseSplitterPlain : public PhraseSplitterBase {
  public:
    virtual void splitPhrase(const std::string &phrase);
    virtual ~PhraseSplitterPlain() {}
};

///////////////////////////////////////////////////////////////////////////////
// REGULAR EXPRESSION RELATED STUFFS
///////////////////////////////////////////////////////////////////////////////

//
// Splitting of text with PCRE (Perl-compatible regular expressions)
//
class PhraseSplitterPCRE : public PhraseSplitterBase {
  std::string m_modString;
  public:
    virtual void splitPhrase(const std::string &phrase);
    const std::string &getModString() const { return m_modString; }
    virtual ~PhraseSplitterPCRE() {}
};

namespace PhraseRegExp {
  enum {
    PHRASE_RE_CASELESS = 0x01,
    PHRASE_RE_UNGREEDY = 0x02,
    PHRASE_RE_EXTENDED = 0x04,
  };
  
  uint8_t compressPCRE_flags(int fl);
  int decompressPCRE_flags(uint8_t cfl);
  int str2PCRE_flags(const char *s);
};

class PhraseRegExpWriter : public QSerializerOut {
  struct phrase_regexp_t {
    uint8_t flags;
    std::string re;
  };
  std::map<unsigned, phrase_regexp_t> m_regs; // phrase ID to regs
  size_t m_reSize;
  
  public:
    PhraseRegExpWriter();
    virtual ~PhraseRegExpWriter() {}
    void add(unsigned phraseID, const std::string &re, uint8_t flags = 0);
    void optimize(const std::vector<unsigned> &vshift);
    
    // export facility
    virtual size_t size() const;
    virtual void   save(MemWriter &mwr);
    unsigned amount() const { return m_regs.size(); };
};

class PhraseRegExReader : public QSerializerIn {
  std::map<unsigned, pcre *> m_regs;
  public:
    // import facility
    virtual void load(MemReader &mrd);
    int match(unsigned phraseID, const std::string &s) const;
    virtual ~PhraseRegExReader();
    unsigned amount() const { return m_regs.size(); };
};

///////////////////////////////////////////////////////////////////////////////

} // namespace gogo

#endif // GOGO_QCLASSIFY_IMPL_HPP__
