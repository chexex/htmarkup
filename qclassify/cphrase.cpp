//------------------------------------------------------------
/// @file   cphrase.cpp
/// @brief  phrase search utility
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   08.05.2009
//------------------------------------------------------------

#include <vector>
#include <map>
#include <stdexcept>

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>

#include <iostream>
#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"

using namespace std;
using namespace gogo;

static char *progname;
static void usage();

int main(int argc, char *argv[])
{
  string cfgfile = "config.xml";
  bool bUseLemm  = true;
  
  {
    extern int optind;
    extern char *optarg;
      
    progname = argv[0];
    int  c;
    while ( (c = getopt(argc, argv, "c:Lv")) != -1) 
      switch(c) {
        case 'c':
          cfgfile = optarg;
          break;
        case 'L':
          bUseLemm = false;
          break;
          
        case 'v':
          printf("Format version: %d\n", qcls_impl::QCLASSIFY_INDEX_VERSION);
          exit(EXIT_SUCCESS);
  
        default:
          usage();
      }
          
      argc -= optind;
      argv += optind;
      if (!argc)
        usage();
  }
  
  LemInterface *plem = NULL;
  
  try {
    XmlConfig cfg(cfgfile.c_str());
    
    if (bUseLemm && cfg.GetBool("QueryQualifier", "UseLemmatizer", true))
      plem = new LemInterface(true /* UTF8 */);
      
    PhraseCollectionLoader ldr(plem);
    ldr.loadByConfig(&cfg);
    if (!ldr.is_loaded()) {
      throw std::runtime_error("Phrase collection not loaded");
    }
    
    for (; argc > 0; argc--, argv++) 
    {
      string phrase = *argv;
      PhraseSearcher::res_cls_num_t res;
      PhraseSearcher::res_cls_num_t::iterator it;
      
      ldr->searchPhrase(phrase, res);
      cout << "\"" << phrase << "\": " << res.size() << " results\n";
      for (it = res.begin(); it != res.end(); it++) {
        const char *origPhrase = ldr->getOriginPhrase(it->second.phrase_id);
        if (!origPhrase) origPhrase = "-";
        
        cout << "\tC: " << ldr->getClassName(it->first) << " (#" << it->first << "); P: \"" << 
            origPhrase << "\" (#" << it->second.phrase_id << "); R: " << 
            it->second.rank << endl;
      }
      if (res.size())
        cout << endl;
    }
  } 
  catch (std::exception &e) {
    delete plem;
    fprintf(stderr, "%s\n", e.what());
    return 1;
  }
  
  delete plem;
  return 0;
}


static void usage()
{
  fprintf(stderr, "Usage: %s [-L] [-c config] phrase ...\n", progname);
  fprintf(stderr, "\t-c - use specified config file\n");
  fprintf(stderr, "\t-L - don't use lemmatizer\n\n");
  exit (EX_USAGE);
}
