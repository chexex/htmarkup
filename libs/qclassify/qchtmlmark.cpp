//------------------------------------------------------------
/// @file   qchtmlmark.hpp
/// @brief  qclassify HTML marker implementation
/// @date   05.06.2009
/// @author Kisel Jan, <kisel@corp.mail.ru>
//------------------------------------------------------------

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>

#include <strings.h>
#include <stdio.h>

#include "config/config.hpp"
#include "utils/stringutils.hpp"
#include "qclassify.hpp"
#include "qclassify_impl.hpp"
#include "htmlmark.hpp"

using namespace gogo;
using namespace std;

namespace gogo {
  
typedef enum {TAG_UNKNOWN = -1, TAG_HTML, TAG_BODY, TAG_A} html_tag_t;

/// @name implementation of Query-Classify HTML-marker
class QCHtmlMarkerImpl 
{
  public:
    typedef struct wordentry {
      unsigned offset;
      unsigned tag_dist;
      unsigned len:15;
      unsigned marked:1;
      unsigned grasp_left:4;
      unsigned grasp_right:4;
    } wordentry_t;
    
    typedef struct match_info {
      int first_id;
      int nwords;
      unsigned phrase_id;
      unsigned clsid;
      unsigned freq;
      unsigned rank;
      wordentry_t *first;
      wordentry_t *last;

      unsigned offset;
      unsigned len;
    } match_info_t;
    
  private:
    
    typedef enum {WTYPE_TEXT, WTYPE_TAG} wtype_t;
  
    struct we_draft {
      int      offset;
      int      len;
      unsigned grasp_left:4;
      unsigned grasp_right:4;
      wtype_t  wt;
    };
    
    
    bool m_bDebug;
    const PhraseSearcher *m_psrch;
    
    struct ClsMarkupConfig {
      string marker;
      bool   bUseUdataAsFormat;
      
      ClsMarkupConfig(const XmlConfig &cfg, const std::string &section) {
        const char *ps = cfg.GetStr(section, "Marker");
        if (ps)
            marker = ps;
        bUseUdataAsFormat = cfg.GetBool(section, "UseUdataAsFormat", true);
      }
    };
    
    map<unsigned, ClsMarkupConfig> m_classConfigs;
    
  public:
    QCHtmlMarkerImpl();
    unsigned markup(const string &text, string &os, const QCHtmlMarker::MarkupSettings &st);
    void loadSettings(const XmlConfig *pcfg);
    QCHtmlMarker::MarkupSettings m_cfgSettings;
    
    static inline QCHtmlMarker::sort_order_t parseOrder(const char *order);
    
  private:
    inline html_tag_t html_tag_to_id(const char *tag);
    html_tag_t extract_tag(const char *p, unsigned n, bool &closer);
    void html_getwords(const string &text, const QCHtmlMarker::MarkupSettings &st, vector<wordentry_t> *words);
    
    inline void SpecEncodeString(const char *orig_phrase, std::string &out);
    bool BuildPhraseURL(const struct match_info &pmi, 
                        const char *html, 
                        string &buf);
    
    void select_phrases(const vector<match_info_t> *vmatched, 
                        vector<match_info_t> *vselected, 
                        const QCHtmlMarker::MarkupSettings &st);
    
    void apply_grasps(vector<match_info_t> *vm);
    void matches_reorder_byrank(vector<match_info_t> &vm, bool ascend);
    void matches_reorder_byfreq(vector<match_info_t> &vm, bool ascend);
    
    friend class QCHtmlMarker;
};

QCHtmlMarkerImpl::QCHtmlMarkerImpl() : m_bDebug(false) {}

inline QCHtmlMarker::sort_order_t QCHtmlMarkerImpl::parseOrder(const char *order)
{
  switch (*order) {
    case 'r': return QCHtmlMarker::MARKUP_ORDER_RANK_ASC;
    case 'R': return QCHtmlMarker::MARKUP_ORDER_RANK_DESC;
    case 'f': return QCHtmlMarker::MARKUP_ORDER_FREQ_ASC;
    case 'F': return QCHtmlMarker::MARKUP_ORDER_FREQ_DESC;
  }
  
  throw std::runtime_error("Unknown sort order value");
}

//---------------------------------------------------------------------------------
/// @brief load settings from config file
/// @param pcfg config pointer
void QCHtmlMarkerImpl::loadSettings(const XmlConfig *pcfg) 
{
  const QCIndexReader &qci = m_psrch->getQCIndex();
  unsigned n = qci.amount();
  
  for (unsigned i = 0; i < n; i++) {
    string tag = QueryClassifierHelper::QCname2XMLtag(qci.getName(i));
    m_classConfigs.insert(make_pair<unsigned, ClsMarkupConfig>(i, ClsMarkupConfig(*pcfg, tag)));
  }
  
  QCHtmlMarker::MarkupSettings def;
  static const char *sec = (const char *)"HtmlMarker";
  
  m_cfgSettings.range = (unsigned)pcfg->GetInt(sec, "MaxPhraseSize", def.range);
  m_cfgSettings.gap   = (unsigned)pcfg->GetInt(sec, "Gap", def.gap);
  m_cfgSettings.nmax  = (unsigned)pcfg->GetInt(sec, "Limit", def.nmax);
  m_cfgSettings.bSkipFirstWord = pcfg->GetBool(sec, "SkipFirstWord", def.bSkipFirstWord);
  m_cfgSettings.bUniq          = pcfg->GetBool(sec, "Uniq", def.bUniq);
  m_cfgSettings.bUseUdataAsFormat = pcfg->GetBool(sec, "UseUdataAsFormat", def.bUseUdataAsFormat);
  
  const char *order = pcfg->GetStr(sec, "SortOrder");
  m_cfgSettings.order = (order) ? parseOrder(order): def.order;
}


/// @brief aux comparators
typedef QCHtmlMarkerImpl::match_info_t mi_t;

static bool compar_matches_offset(mi_t inf1, mi_t inf2)    { return (inf1.offset < inf2.offset); }
static bool compar_matches_rank_asc(mi_t inf1, mi_t inf2)  { return (inf1.rank < inf2.rank); }
static bool compar_matches_rank_desc(mi_t inf1, mi_t inf2) { return (inf1.rank > inf2.rank); }
static bool compar_matches_freq_asc(mi_t inf1, mi_t inf2)  { return (inf1.freq < inf2.freq); }
static bool compar_matches_freq_desc(mi_t inf1, mi_t inf2) { return (inf1.freq > inf2.freq); }


//-----------------------------------------------------------------------------------
/// @brief Main function - markup text
/// @return amount of marked blocks
//-----------------------------------------------------------------------------------
unsigned QCHtmlMarkerImpl::markup(const string &text, string &os, const QCHtmlMarker::MarkupSettings &st)
{
  if (!m_psrch) {
    os = text;
    return 0;
  }
    
  PhraseSearcher::res_cls_num_t cres;
  vector<wordentry_t>  words;
  vector<match_info_t> vmatched, vselected;
  
  const char *html = const_cast<const char *>(text.data()); 
  size_t size = text.size();  

  html_getwords(text, st, &words);

  // lookup matched phrases
  int i, n = words.size(), range, maxi;
  match_info_t mi;
  wordentry_t *curw, *endw, *pw;

  for (range = st.range; range > 0; range--)
    for (i=0, maxi = n - range; i <= maxi; i++) 
    {
      curw = &words[i];
  
      if ((int)curw->tag_dist < range) 
        continue;
  
      endw = curw + range - 1;
      // construct string from words
      /// TODO: replace by ...
      string s;
      for (pw = curw; pw <= endw; pw++) {
        s.append(html + pw->offset, pw->len);
        if (pw != endw)
          s += " ";
      }
      
      unsigned n = m_psrch->searchPhrase(s, cres);
      if (m_bDebug)
        printf("=== CLS: \"%s\": %u\n", s.c_str(), n);
      
      if (n)  {
        // remember best of matched
        PhraseSearcher::res_cls_num_t::iterator cit = PhraseSearcher::selectBest(cres);
        mi.rank = cit->second.rank;
        mi.phrase_id = cit->second.phrase_id;
        mi.clsid     = cit->first;
        
        mi.offset   = curw->offset;
        mi.len      = endw->offset + endw->len - curw->offset;
        mi.first_id = i;
        mi.nwords = range;
        mi.first  = curw;
        mi.last   = endw;
  
        vmatched.push_back(mi);
      }
    }
   
    // Preorder matched phrases if need
    // (default is no ordering - i.e. bigger phrases from left to right)
    switch(st.order) {
      case QCHtmlMarker::MARKUP_ORDER_NATIVE:
        break;
        
      case QCHtmlMarker::MARKUP_ORDER_RANK_ASC:
      case QCHtmlMarker::MARKUP_ORDER_RANK_DESC:
        matches_reorder_byrank(vmatched, st.order == QCHtmlMarker::MARKUP_ORDER_RANK_ASC);
        break;
      
      case QCHtmlMarker::MARKUP_ORDER_FREQ_ASC:
      case QCHtmlMarker::MARKUP_ORDER_FREQ_DESC:
        matches_reorder_byfreq(vmatched, st.order == QCHtmlMarker::MARKUP_ORDER_FREQ_ASC);
        break;  
    }
    
    // now we have ALL phrases in vmatched. Theese phrases are self-ordered
    // by phrase lenght from beggining of text to it's end. So, now we should
    // select only non-overlapped phrases with some restrictions from <args>.
    select_phrases(&vmatched, &vselected, st);
    apply_grasps(&vselected);
    
    sort(vselected.begin(), vselected.end(), compar_matches_offset);
    n = vselected.size();
    assert(st.nmax == 0 || (unsigned)n <= st.nmax);

    unsigned nMarked = 0;
    // markup
    {
      unsigned bc = 0;
      vector<match_info_t>::const_iterator it;
      string url;

      // add matched phrases with formatted markup
      for (it = vselected.begin(); it != vselected.end(); it++) {
        os.append(html + bc, it->offset - bc);

        if (BuildPhraseURL(*it, html, url)) {
          os.append(url);
          nMarked++;
        } else {
          // put text unchanged
          os.append(html + it->offset, it->len);
        }
            
        bc = it->offset + it->len;
      }

      // add data after last match (tail)
      os.append(html + bc, size - bc);
    }

    return nMarked;
}

struct StrCasecomp {
  bool operator() (const char *s1, const char *s2) const { return (strcasecmp(s1, s2) < 0); }
};

//-----------------------------------------------------------------------------------
/// @brief Apply left and right grasp if phrase fully matched.
/// @brief It's usually used in quotation marks, for example:
/// @brief   "&laquo;<MARK>Mister Putin</MARK>&raquo;" (wrong)
/// @brief   "<MARK>&laquo;Mister Putin&raquo;</MARK>" (right)
//-----------------------------------------------------------------------------------
void QCHtmlMarkerImpl::apply_grasps(vector<match_info_t> *vm)
{
  vector<match_info_t>::iterator it;
  int grasp_left, grasp_right;
    
  for (it = vm->begin(); it != vm->end(); it++) {
    grasp_left  = (*it).first->grasp_left;
    grasp_right = (*it).last->grasp_right;

    if (grasp_left != 0 && grasp_right != 0) {
      /* apply grasps, i.e. expand matched boundaries */
      (*it).offset -= grasp_left;
      (*it).len    += grasp_left + grasp_right;
    }
  }
}

//-----------------------------------------------------------------------------------
/// @brief select limited amount of matched phrases from all
/// @param vmatched  - all matched phrases
/// @param vselected - [out] selected phrases
//-----------------------------------------------------------------------------------
void QCHtmlMarkerImpl::select_phrases(const vector<match_info_t> *vmatched, 
                   vector<match_info_t> *vselected, 
                   const QCHtmlMarker::MarkupSettings &st)
{   
  vector<match_info_t>::const_iterator it;
  set<int> begs, ends;
  int l, r, first_id, last_id;
  set<int>::const_iterator its;
  wordentry_t *wef, *wel;
  bool already_marked;
  set<unsigned> phrases_tomark;

  for (it = vmatched->begin(); 
       it != vmatched->end() && (!st.nmax || vselected->size() < st.nmax); 
       it++) 
  {
    if (st.bUniq && phrases_tomark.count(it->phrase_id) != 0)
      continue;

    first_id = (*it).first_id;
    last_id = first_id + (*it).nwords - 1;
    wel = (*it).last;
       
    // verify that phrases doesn't overlapped
    already_marked = false;
    for (wef = (*it).first; wef <= wel; wef++) {
      if (wef->marked) {
        already_marked = true;
        break;
      }
    }
    if (already_marked)
      continue;

    // now look for nearest phrase end (l) and nearest
    // phrase beginning (r); this only make sense for non-zero gap.
    l = r = -1;
    if (st.gap) {
      its = ends.lower_bound(first_id);
      if (its == ends.end()) {
        // no elements >= first_id, so get biggest value
        if (!ends.empty())
          l = (*ends.rbegin()); 
      } else {
        for(;;) {
          if ((*its) <= first_id) {
            l = (*its);
            break;
          }
          if (its == ends.begin())
            break;
          its--;
        }
      }

      its = begs.upper_bound(last_id);
      if (its != begs.end()) 
        r = *its;
    }

    if ((l == -1 || first_id - l > (int)st.gap) && 
         (r == -1 || r - last_id  > (int)st.gap))
    {
      // phrase satisfy conditions, so insert.
      for (wef = (*it).first; wef <= wel; wef++) 
        wef->marked = 1;
            
      if (st.gap) {
        ends.insert(last_id);
        begs.insert(first_id);
      }

      if (st.bUniq)
        phrases_tomark.insert(it->phrase_id);
            
      vselected->push_back(*it);
    }
  }
}

//-----------------------------------------------------------------------------------
/// @brief reorder matches by frequency
/// @param vm - [in] match vector
/// @param ascend - [in] order by ascendancy
//-----------------------------------------------------------------------------------
void QCHtmlMarkerImpl::matches_reorder_byfreq(vector<match_info_t> &vm, bool ascend)
{
  map<int, int> phrases_freq; /* phrase ID => ntimes */
  map<int, int>::iterator mit;
  vector<match_info_t>::iterator it;
    
  for (it = vm.begin(); it != vm.end(); it++) {
    mit = phrases_freq.find((*it).phrase_id);
    if (mit == phrases_freq.end())
      phrases_freq.insert( pair<int, int>((*it).phrase_id, 1) );
    else
      mit->second++;
  }
    
  for (it = vm.begin(); it != vm.end(); it++) {
    mit = phrases_freq.find((*it).phrase_id);
    assert(mit != phrases_freq.end());
    (*it).freq = mit->second;
  }

  stable_sort(vm.begin(), vm.end(), ascend ? compar_matches_freq_asc : compar_matches_freq_desc);
}

//-----------------------------------------------------------------------------------
/// @brief reorder matches by rank
/// @param vm - [in] match vector
/// @param ascend - [in] order by ascendancy
//-----------------------------------------------------------------------------------
void QCHtmlMarkerImpl::matches_reorder_byrank(vector<match_info_t> &vm, bool ascend)
{
  stable_sort(vm.begin(), vm.end(), ascend ? compar_matches_rank_asc : compar_matches_rank_desc);
}

//-----------------------------------------------------------------------------------
/// @brief special encoding of string ( base64(s) ) where `s' in UTF-8
//-----------------------------------------------------------------------------------
inline void QCHtmlMarkerImpl::SpecEncodeString(const char *orig_phrase, std::string &out)
{
  size_t ilen = strlen(orig_phrase);
  std::vector<char> b64_buf(ilen * 2);

  size_t n = base64_encode((u_char *)orig_phrase, ilen, (u_char *)&b64_buf[0]);
  gogo::str_escape(&b64_buf[0], out, n);
}

//-----------------------------------------------------------------------------------
/// @brief build URL base on fomrmat provided in config file
/// @param pmi - [in] matching info
/// @param buf - [out] resulting buffer
//-----------------------------------------------------------------------------------
bool QCHtmlMarkerImpl::BuildPhraseURL(const struct match_info &pmi, 
                    const char *html, 
                    string &buf) 
{
  buf.clear();
  
  static const char *markup_fmt_default = (const char *)"<a class=\"gomail_search\" target=\"_blank\" href=\"http://go.mail.ru/search?q=%O\">%P</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\" width=\"13\" height=\"13\" alt=\"\" />";
  
  const char *udata = m_psrch->getUserData(pmi.phrase_id);
  const char *orig_phrase = m_psrch->getOriginPhrase(pmi.phrase_id);

  map<unsigned, ClsMarkupConfig>::const_iterator it = m_classConfigs.find(pmi.clsid);
  const ClsMarkupConfig *pccfg = (it == m_classConfigs.end()) ? NULL : &it->second;
  bool useUdataAsFormat = !pccfg || pccfg->bUseUdataAsFormat;
  const char *markup_fmt;
  
  if (useUdataAsFormat && udata) 
    markup_fmt = udata;
  else {
    markup_fmt = (pccfg && !pccfg->marker.empty()) ? pccfg->marker.c_str() : markup_fmt_default;
  }
  
  for (const char *pfmt = markup_fmt; *pfmt != 0; pfmt++) 
  {
    if (*pfmt == '%') 
    {
      char esc_symbol = *(pfmt + 1);
      switch(esc_symbol)
      {
        case 'O':
          if (!orig_phrase) return false;
          buf += orig_phrase;
          break;
          
        case 'S':
        {
          if (!orig_phrase) return false;
          
          std::string escaped_orig;
          gogo::str_escape( orig_phrase, escaped_orig );
          buf.append( escaped_orig );
          break;
        }
          
        case 'U':
          if (!udata || markup_fmt == udata /* prevent recursion */) return false;
          buf += udata;
          break;
          
        case 'M': {
          if (!orig_phrase) return false;
          
          string e;
          SpecEncodeString(orig_phrase, e);
          buf += e;
          }
          break;
          
        case 'P':
          buf.append(html + pmi.offset, pmi.len);
          break;
          
        case 'Q':
        {
          std::string escaped_match;
          
          gogo::str_escape( html + pmi.offset, escaped_match, pmi.len );
          buf.append( escaped_match );
          break;
        }
        
        default:
          buf += '%';
          buf += esc_symbol;
      }
      
      pfmt++;
      continue;
    }
    
    buf += *pfmt;
  }
  
  return true;
}

//-----------------------------------------------------------------------------------
/// @brief recognize HTML tag (convert to quite limited set of tags)
//-----------------------------------------------------------------------------------
inline html_tag_t QCHtmlMarkerImpl::html_tag_to_id(const char *tag)
{
  static map< const char *, html_tag_t, StrCasecomp > html_tags;
  map< const char *, html_tag_t >::const_iterator it;

  if (!html_tags.size()) {
    struct tag_match {
      char *tag;
      html_tag_t id;
    } matches[] = { 
      {(char*)"html", TAG_HTML}, 
      {(char*)"body", TAG_BODY}, 
      {(char*)"a",    TAG_A   }, 
    };

    for (unsigned i = 0; i < sizeof(matches) / sizeof(matches[0]); i++)
    {
      html_tags.insert( pair<const char *, html_tag_t>(matches[i].tag, (matches[i].id)) );
    }
  }

  it = html_tags.find(tag);
  return (it == html_tags.end()) ? TAG_UNKNOWN : it->second;
}

//-----------------------------------------------------------------------------------
/// @brief Get tag identifier based on string p length n;
/// @param p - tag sting
/// @param n - string length
/// @param closer - [out] closer tag or not
/// @return HTML tag identifier
//-----------------------------------------------------------------------------------
html_tag_t QCHtmlMarkerImpl::extract_tag(const char *p, unsigned n, bool &closer)
{
  static char *tag_symbols = (char *)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char tag[8];
    
  if (*p != '<')
    return TAG_UNKNOWN;
  p++; n--;

  if (*p == '/') {
    closer = true;
    p++; n--;
  } else
    closer = false;

    if (n < 1)
      return TAG_UNKNOWN;

    if (n >= sizeof(tag))
      n = sizeof(tag) - 1;

    strncpy(tag, p, n); tag[n] = '\0';
    tag[ strspn(tag, tag_symbols) ] = '\0';
    
    return html_tag_to_id(tag);
}


//-----------------------------------------------------------------------------------
/// @brief Extract words from HTML document
/// @param text  - [in] source HTML text
/// @param st    - [in] markup settings
/// @param words - [out] words selected vector
//-----------------------------------------------------------------------------------
void QCHtmlMarkerImpl::html_getwords(const string &text, const QCHtmlMarker::MarkupSettings &st, vector<wordentry_t> *words)
{
  typedef enum {STATE_INTEXT, STATE_INTAG} STATES;
  const char *html = const_cast<const char *>(text.data()); 
  size_t size = text.size();

  STATES state = STATE_INTEXT;
  const char *p, *lastopen = NULL;
  int    wlen = 0, step = 1;
  vector<struct we_draft> weds;
  bool   within_a = false; /* we should ignore content within <a> tag */
  bool   within_body = false; /* collect words only within body */
  bool   within_html = false;
  bool   skip_word = st.bSkipFirstWord; // skip first word at document
  bool   mem_text, spacer, splitter;
  static char *wsplit_symbols = (char *)",.;:!?()\"[]/\\{}"; // +«»
  size_t offset;
  int    quote_open_beg = -1, quote_close_end = -1;

#define ESC_TYPE_SPACER      's'
#define ESC_TYPE_SPLITTER    'S'
#define ESC_TYPE_QUOTE_OPEN  'q'
#define ESC_TYPE_QUOTE_CLOSE 'Q'

#define ESC_SPACER(s)      {(char *)s, sizeof(s)-1, ESC_TYPE_SPACER}
#define ESC_SPLITTER(s)    {(char *)s, sizeof(s)-1, ESC_TYPE_SPLITTER}
#define ESC_QUOTE_OPEN(s)  {(char *)s, sizeof(s)-1, ESC_TYPE_QUOTE_OPEN}
#define ESC_QUOTE_CLOSE(s) {(char *)s, sizeof(s)-1, ESC_TYPE_QUOTE_CLOSE}

  struct escape {
    char *esc;
    int  len;
    char type;
  } escapes[] = {
    ESC_SPLITTER("&lt;"),    ESC_SPLITTER("&gt;"),
    ESC_QUOTE_OPEN("&laquo;"),  ESC_QUOTE_CLOSE("&raquo;"),
    ESC_SPLITTER("&mdash;"), ESC_SPLITTER("&quot;"),
    ESC_SPACER("&nbsp;"),    ESC_SPACER("&amp;"), ESC_SPACER("&#039;"),
    {NULL, 0, 0}
  };

#define UPDATE_MEM_TEXT do { \
  mem_text = !within_a && (!within_html || within_body); \
} while(0)

#define SAVE_WORD if (mem_text && wlen > 0 && !(wlen == 1 && *(p-1) == '-')) { \
  struct we_draft wed;             \
  \
  wed.offset = offset - wlen;      \
  wed.len = wlen;                  \
  wed.wt = WTYPE_TEXT;             \
  \
  if (quote_open_beg != -1) {      \
  wed.grasp_left = wed.offset - quote_open_beg;      \
  quote_open_beg = -1;                               \
} else                           \
  wed.grasp_left = 0;                                \
  \
  if (quote_close_end != -1) {     \
  wed.grasp_right  = quote_close_end - offset;       \
  quote_close_end = -1;                              \
} else                           \
  wed.grasp_right = 0;                               \
  \
  if (skip_word) {                   \
    skip_word = false;               \
  } else {                           \
    weds.push_back(wed);             \
  }                                  \
}

  UPDATE_MEM_TEXT;

  for (offset=0, p = html; offset < size; p+=step, offset+=step)
  {
    char c = *p;
    step = 1;
            
    if (c == '<')
      lastopen = p;

    switch(state) {
      case STATE_INTEXT:
        splitter = (strchr(wsplit_symbols, c) != NULL || c == '\0');
        spacer   = (c == ' ' || c == '\t' || c == '\r' || c == '\n'); // TODO: utf8_isspace
                       
        if (c == '&') {
          /* check for special HTML sequences (escapes) */
          int rest = size - offset;
          struct escape *pe;

          for (pe = &escapes[0]; pe->esc; pe++) 
            if (rest >= pe->len && !strncmp(p, pe->esc, pe->len)) {
            switch(pe->type) {
              case ESC_TYPE_SPACER:
                spacer = true;
                break;
              case ESC_TYPE_SPLITTER:
                splitter = true;
                skip_word = st.bSkipFirstWord;
                break;
              case ESC_TYPE_QUOTE_OPEN:
                splitter = true;
                quote_open_beg = (int)offset;
                skip_word = st.bSkipFirstWord;
                break;
              case ESC_TYPE_QUOTE_CLOSE:
                splitter = true;
                quote_close_end = (int)offset + pe->len;
                skip_word = st.bSkipFirstWord;
                break;
            }
            step = pe->len;
            break;
            }
        }
        if (c == '<' || spacer || splitter) {
          SAVE_WORD;
                    
          if (c == '<' || splitter)
          {
            if (weds.size() && weds.back().wt != WTYPE_TAG) {
              // we only interest of fact of tag, not their number
              struct we_draft wed = {offset, 0, 0, 0, WTYPE_TAG};
              weds.push_back(wed);
            }
                        
            if (c == '<')
              state =  STATE_INTAG;
          }

          wlen = 0;
                    
          // check for end of sentence - mark
          if (c == '.' || c == '?' || c == '!' || c == ';') 
            skip_word = st.bSkipFirstWord;
        }
        else 
          wlen++;
        break;

      case STATE_INTAG:
        if (c == '>') {
          bool closer;
          html_tag_t tag;
                    
          tag = extract_tag(lastopen, p - lastopen, closer);
          switch(tag) {
            case TAG_HTML:
              within_html = !closer;
              UPDATE_MEM_TEXT;
              break;
            case TAG_BODY:
              within_body = !closer;
              UPDATE_MEM_TEXT;
              break;
            case TAG_A:
              within_a = !closer;
              UPDATE_MEM_TEXT;
            default:
              ;
          }
                    
          state = STATE_INTEXT;
          skip_word = st.bSkipFirstWord;
          quote_open_beg = quote_close_end = -1;
        }
    }
  } /* for p ... */

  // save last word
  if (!skip_word) 
    SAVE_WORD;

  vector<struct we_draft>::const_iterator it;

  if (m_bDebug) {
    printf("\nPARSED TEXT ---------------------------\n");
    for (it = weds.begin(); it != weds.end(); it++) {
      if (it->wt == WTYPE_TAG)
        printf("<TAG>\n");
      else {
        printf("WORD: \"%.*s\"\t(offset=%d)\n",
               it->len, html + it->offset, it->offset);
      }
    }
  }

  // now fill tag_dist with distance to nearest tag or end of document
  // looking from end
  deque<wordentry_t> wq;

  unsigned dist = 0;
  for (int i = (int)weds.size() - 1; i >= 0; i--) 
  {
    const struct we_draft &wer = weds[i];
    if (wer.wt == WTYPE_TAG)
      dist = 0;
    else {
      wordentry_t we;

      ++dist;

      we.offset = wer.offset;
      we.tag_dist = dist;
      we.len = wer.len;
      we.grasp_left  = wer.grasp_left;
      we.grasp_right = wer.grasp_right;
      we.marked = 0;
      wq.push_front(we);
    }
  }
   
  // now write result in direct sequence
  deque<wordentry_t>::const_iterator dqi;
  for (dqi = wq.begin(); dqi != wq.end(); dqi++) {
    words->push_back(*dqi);
  }
}



//////////////////////////////////////////////////////////////////////
// QCHtmlMarker - interface class
//////////////////////////////////////////////////////////////////////

QCHtmlMarker::QCHtmlMarker(const PhraseSearcher *psrch /* = NULL */) {
  m_pimpl = new QCHtmlMarkerImpl();
  setPhraseSearcher(psrch);
}

QCHtmlMarker::~QCHtmlMarker() { delete m_pimpl; }

void QCHtmlMarker::setPhraseSearcher(const PhraseSearcher *psrch) { m_pimpl->m_psrch = psrch; }

//---------------------------------------------------------------------------------
/// @brief markup text
/// @param text - input text
/// @param os   - marked up output stream
/// @param st   - settings
/// @return amount of marked blocks
//---------------------------------------------------------------------------------
unsigned QCHtmlMarker::markup(const string &text, string &os, const MarkupSettings &st)
{
  os.clear();
  if (text.empty())    
    return 0;
  
  return m_pimpl->markup(text, os, st);
}

//---------------------------------------------------------------------------------
/// @brief markup text with default settings
/// @param text - input text
/// @param os   - marked up output stream
/// @return amount of marked blocks
//---------------------------------------------------------------------------------
unsigned QCHtmlMarker::markup(const string &text, string &os)
{
  MarkupSettings def = getConfigSettings();
  return markup(text, os, def);
}

//---------------------------------------------------------------------------------
/// @brief use it for debugging
//---------------------------------------------------------------------------------
void QCHtmlMarker::setDebug(bool bEnabled) { m_pimpl->m_bDebug = bEnabled; }

//---------------------------------------------------------------------------------
/// @brief load settings from config file
/// @param pcfg config pointer
void QCHtmlMarker::loadSettings(const XmlConfig *pcfg) { 
  m_pimpl->loadSettings(pcfg); 
}

//---------------------------------------------------------------------------------
/// @brief get default settings parsed from config
/// @return settings struct
QCHtmlMarker::MarkupSettings QCHtmlMarker::getConfigSettings() const { 
  return m_pimpl->m_cfgSettings; 
}

} // namespace gogo
