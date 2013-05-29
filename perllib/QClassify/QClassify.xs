#ifdef __cplusplus
extern "C" {
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef __cplusplus
}
#endif

#include "ppport.h"

// undef conflicting perl macroses
#ifdef Copy
#  undef Copy
#endif

#ifdef do_close
#   undef do_close
#endif

#ifdef do_open
#   undef do_open
#endif

#include <stdio.h>
#include <string>
#include "config/config.hpp"
#include "lem_interface/lem_interface.hpp"
#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"
#include "qclassify/htmlmark.hpp"
using namespace gogo;


//
// Class wrapper around PhraseCollectionLoader
//
class QClassifyAgent 
{
  const PhraseSearcher *m_psrch;
  PhraseCollectionLoader m_ldr;
  static lemInterface *m_pLem;
  static int lem_nrefs;
  XmlConfig m_cfg;
  std::string m_req;
  QCHtmlMarker m_marker;

  int  m_error;

public:
  enum { ESTATUS_OK = 0, ESTATUS_CONFERROR, ESTATUS_INITERROR, ESTATUS_INDEXERROR, ESTATUS_LOADERROR };

  PhraseSearcher::res_t m_clsRes;
  int init() 
  {
    m_psrch = 0;
    try {
        if (!m_pLem) {
            assert(!lem_nrefs);
            m_pLem = new lemInterface();
        }
        lem_nrefs++;
    } catch(...) {
        return (m_error = ESTATUS_INITERROR);
    }

    return (m_error = ESTATUS_OK);
  }
  
  int loadConfig(const char *path) 
  {
    m_psrch = NULL;
    try {
        if (!m_cfg.Load(path)) 
            return (m_error = ESTATUS_CONFERROR);
    } catch(...) {
        return (m_error = ESTATUS_CONFERROR);
    }
    return ESTATUS_OK;
  }
  
  QClassifyAgent() { init(); }

  QClassifyAgent(const char *s) {
    init();
    m_error = loadConfig(s);
  }


  ~QClassifyAgent() {
    if (--lem_nrefs == 0) {
        delete m_pLem;
        m_pLem = NULL;
    }
  }

  int index2file() {
   try {
       PhraseCollectionIndexer idx(m_pLem); 
       idx.indexByConfig(&m_cfg);
       idx.save();
   } catch(...) {
        return (m_error = ESTATUS_INDEXERROR);
   }
   return ESTATUS_OK;
  }

  PhraseSearcher::res_t& searchPhrase(const char *s) {
    if (!m_psrch) 
      prepareSearch();

    m_req.assign(s);
    m_psrch->searchPhrase(m_req, m_clsRes);
    return m_clsRes;
  }

  int initMarkup() {
    int ret = prepareSearch();
    if (ret != ESTATUS_OK)
        return ret;

    m_marker.setPhraseSearcher(m_psrch);
    m_marker.loadSettings(&m_cfg);
    return ESTATUS_OK;
  }
  
  unsigned callMarkup(const std::string &s, std::string &out, 
                      const QCHtmlMarker::MarkupSettings &st) 
  {
    return m_marker.markup(s, out, st);
  }

  
  void getMarkupSettings(QCHtmlMarker::MarkupSettings *pst) {
    *pst = m_marker.getConfigSettings();
  }

  void getIndexFileName(std::string &s) {
    m_cfg.GetStr("QueryQualifier", "IndexFile", s, "phrases.idx");
  }

  int error() const { return m_error; }
  void clearerr() { m_error = ESTATUS_OK; }

private:

  int prepareSearch() {
    m_ldr.setLemmatizer(m_pLem);
    if (!m_ldr.loadByConfig(&m_cfg)) {
        return (m_error = ESTATUS_LOADERROR);
    }
        
    m_psrch = m_ldr.getSearcher();
    return ESTATUS_OK;
  }
};

lemInterface *QClassifyAgent::m_pLem = NULL;
int QClassifyAgent::lem_nrefs = 0;


MODULE = QClassify		PACKAGE = QClassify	

QClassifyAgent *
QClassifyAgent::new(const char *s)


void
QClassifyAgent::DESTROY()

void
QClassifyAgent::initMarkup()
CODE:
    if (!THIS->initMarkup()) {
        XSRETURN_YES;
    }
    XSRETURN_NO;

void
QClassifyAgent::reinitMarkup()
CODE:
    if (!THIS->initMarkup()) {
        XSRETURN_YES;
    }
    XSRETURN_NO;

unsigned
QClassifyAgent::version()
CODE:
    RETVAL = qcls_impl::QCLASSIFY_INDEX_VERSION;
OUTPUT:
RETVAL


void
QClassifyAgent::clearerr()
CODE:
    THIS->clearerr();

int
QClassifyAgent::error()
CODE:
    RETVAL = THIS->error();
OUTPUT:
RETVAL

void
QClassifyAgent::loadConfig(const char *path)
CODE:
    if (!THIS->loadConfig(path)) {
        XSRETURN_YES;
    }
    XSRETURN_NO;

void
QClassifyAgent::index2file()
CODE:
    if (!THIS->index2file()) {
        XSRETURN_YES;
    }
    XSRETURN_NO;

void
QClassifyAgent::getIndexFileName()
PPCODE:
    std::string s;
    THIS->getIndexFileName(s);
    XPUSHs( sv_2mortal(newSVpv(s.c_str(), 0)) );

void
QClassifyAgent::classifyPhrase(const char *phrase)
PPCODE:
  PhraseSearcher::res_t &r = THIS->searchPhrase(phrase);
  if (r.size() != 0) 
  {
    EXTEND(SP, r.size() * 2);
    for(PhraseSearcher::res_t::const_iterator it = r.begin(); it != r.end(); it++) {
      XPUSHs( sv_2mortal(newSVpv(it->first.data(), it->first.length())) ); // class
      mXPUSHu(it->second); // rating (unsigned)
    }
  }

void
QClassifyAgent::markup(const char *s)
INIT:
  //HV *settings; /* settings hash */
  //SV **pval;
PPCODE:
  QCHtmlMarker::MarkupSettings st;
  std::string out;
  unsigned n;

  THIS->getMarkupSettings(&st);

  // initialize settings from passed hash
  /*settings = (HV *)SvRV(hashref);
  pval     = hv_fetch(settings,"uniq",4,0);
  if (pval)
    st.bUniq = (bool)SvIV(*pval);*/

  n = THIS->callMarkup((std::string)s, out, st);

  // now push returning data to stack
  XPUSHs(sv_2mortal(newSViv(n)));
  XPUSHs(sv_2mortal( newSVpv(out.data(), out.length() )));

