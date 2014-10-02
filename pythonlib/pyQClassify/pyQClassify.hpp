#include <stdio.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <Interfaces/cpp/LemInterface.hpp>
#include "config/config.hpp"
#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"
#include "qclassify/htmlmark.hpp"

class QClassifyError: public std::exception {
public:
    explicit QClassifyError(const std::string & inMessage);
    virtual ~QClassifyError() throw() {}
    virtual const char* __repr__() const throw();
    std::string message;    
};  


class QClassifyAgent 
{
    const gogo::PhraseSearcher *m_psrch;
    gogo::PhraseCollectionLoader m_ldr;
    static LemInterface *m_pLem;
    static int lem_nrefs;
    XmlConfig m_cfg;
    std::string m_req;
    gogo::QCHtmlMarker m_marker;
    gogo::PhraseSearcher::res_t m_clsRes;

    void prepareSearch();
    gogo::PhraseSearcher::res_t& searchPhrase(const char *s) ;
    void initMarkup(); 
    std::string getIndexFileName();
    

public:
    static const int QCLASSIFY_INDEX_VERSION = gogo::qcls_impl::QCLASSIFY_INDEX_VERSION;
   
    QClassifyAgent(const char *s);
    ~QClassifyAgent();

    void index2file() ;
    std::string markup(std::string s);
};
