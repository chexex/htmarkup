#include "pyQClassify.hpp"

using namespace gogo;

LemInterface *QClassifyAgent::m_pLem = NULL;
int QClassifyAgent::lem_nrefs = 0;

QClassifyError::QClassifyError(const std::string & inMessage):
    std::exception(),
    message(inMessage) 
{
}

const char* QClassifyError::__repr__() const throw(){
    return message.c_str();
}

void QClassifyAgent::prepareSearch() {
    this->m_ldr.setLemmatizer(this->m_pLem);
    if (!this->m_ldr.loadByConfig(&this->m_cfg)) {
        throw QClassifyError("Unable to load PhraseCollectionLoader config");
    }
        
    this->m_psrch = this->m_ldr.getSearcher();
}

PhraseSearcher::res_t& QClassifyAgent::searchPhrase(const char *s) {
    if (!this->m_psrch) 
      this->prepareSearch();

    this->m_req.assign(s);
    this->m_psrch->searchPhrase(this->m_req, this->m_clsRes);
    return this->m_clsRes;
}

void QClassifyAgent::initMarkup() {
    this->prepareSearch();
    this->m_marker.setPhraseSearcher(this->m_psrch);
    this->m_marker.loadSettings(&this->m_cfg);

}

std::string QClassifyAgent::getIndexFileName() {
    std::string filename;
    this->m_cfg.GetStr("QueryQualifier", "IndexFile", filename, "phrases.idx");
    return filename;
}

QClassifyAgent::QClassifyAgent(const char *path) {    
    struct stat buffer; 
    this->m_psrch = 0;
    try {
        if (!(this->m_pLem)){
            assert(!(this->lem_nrefs));
            this->m_pLem = new LemInterface(true /* UTF8 */);
        }
        this->lem_nrefs++;
    } 
    catch(...){
        throw QClassifyError("liblemmatizer can not be initialized");
    }

    // load config
    this->m_psrch = NULL;

    if  (stat (path, &buffer) != 0){
        throw QClassifyError("Config file does not exists");
    }
    
    try {
        if (!this->m_cfg.Load(path))             
            throw QClassifyError("Unable to load config file");
    } 
    catch(...) {
        throw QClassifyError("Unknown error while loading config file");
    }
    this->initMarkup();
}

QClassifyAgent::~QClassifyAgent() {
    if (--(this->lem_nrefs) == 0) {
        delete this->m_pLem;
        this->m_pLem = NULL;
    }
}

void QClassifyAgent::index2file() {
    try {
       gogo::PhraseCollectionIndexer idx(this->m_pLem); 
       idx.indexByConfig(&(this->m_cfg));
       idx.save();
    } 
    catch(...) {
        throw QClassifyError("Indexing error");
    }
    this->initMarkup();    
}

std::string QClassifyAgent::markup(std::string s) {
    struct stat buffer;    
    QCHtmlMarker::MarkupSettings settings;
    std::string out;
    std::string index_filename = getIndexFileName();

    if  (stat (index_filename.c_str(), &buffer) != 0){
        throw QClassifyError("Index file does not exist");        
    }
    settings = this->m_marker.getConfigSettings();
    this->m_marker.markup(s, out, settings);
    return out;
}
