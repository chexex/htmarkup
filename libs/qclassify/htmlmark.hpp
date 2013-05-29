#ifndef GOGO_HTML_MARK_HPP__
#define GOGO_HTML_MARK_HPP__

#include <string>

namespace gogo {
 
  
class QCHtmlMarkerImpl;

class QCHtmlMarker 
{
  public:
    typedef enum {
      MARKUP_ORDER_NATIVE,
      MARKUP_ORDER_RANK_ASC,
      MARKUP_ORDER_RANK_DESC,
      MARKUP_ORDER_FREQ_ASC,
      MARKUP_ORDER_FREQ_DESC
    } sort_order_t;
    
    struct MarkupSettings {
      sort_order_t order;
      unsigned  range; // search phrase length of [1 .. range]
      unsigned  gap;   // preserve such count of words between selection
      unsigned  nmax;  // limit of phrases to mark
      
      bool bUniq;          // mark only uniq phrases (phrase id)
      bool bSkipFirstWord; // skip first word in sentence
      
      bool bUseUdataAsFormat; // use userdata as format or %U marker
  
      MarkupSettings() : order(MARKUP_ORDER_NATIVE), range(5), gap(0), nmax(0), 
                     bUniq(false), bSkipFirstWord(false), bUseUdataAsFormat(true) {}
    };
    
  private:
    QCHtmlMarkerImpl *m_pimpl;
  public:
    QCHtmlMarker(const PhraseSearcher *psrch = NULL);
    ~QCHtmlMarker();
    
    //---------------------------------------------------------------------------------
    /// @brief set phrase searcher
    void setPhraseSearcher(const PhraseSearcher *psrch);
    
    //---------------------------------------------------------------------------------
    /// @brief markup text
    /// @param text - input text
    /// @param os   - marked up output stream
    /// @param st   - settings
    /// @return amount of marked blocks
    unsigned markup(const std::string &text, std::string &os, const MarkupSettings &st);
    
    //---------------------------------------------------------------------------------
    /// @brief markup text with default settings
    unsigned markup(const std::string &text, std::string &os);
    
    //---------------------------------------------------------------------------------
    /// @brief use it for debugging
    void setDebug(bool bEnabled);
    
    //---------------------------------------------------------------------------------
    /// @brief load settings from config file
    /// @param pcfg config pointer
    void loadSettings(const XmlConfig *pcfg);
    
    //---------------------------------------------------------------------------------
    /// @brief get default settings parsed from config
    /// @return settings struct
    MarkupSettings getConfigSettings() const;
};

} // namespace gogo

#endif // GOGO_HTML_MARK_HPP__
