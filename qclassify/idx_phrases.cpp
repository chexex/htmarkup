//------------------------------------------------------------
/// @file   idx_phrases.cpp
/// @brief  phrase index utility
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   08.05.2009
//------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <sysexits.h>

#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"

using namespace std;
using namespace gogo;

static char *progname;
static void usage();

int 
main(int argc, char *argv[])
{
    string cfgfile = "config.xml";
    bool bSave = true, bUseLemm  = true;

    {
      extern int optind;
      extern char *optarg;
      
      progname = argv[0];
      int  c;
      while ( (c = getopt(argc, argv, "c:LSv")) != -1) 
          switch(c) {
              case 'S':
                  bSave = false;
                  break;
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
      if (argc)
        usage();
    }
    
    
    try {
      XmlConfig cfg(cfgfile.c_str());
      LemInterface *plem;
      
      if (bUseLemm && cfg.GetBool("QueryQualifier", "UseLemmatizer", true)) {
        plem = new LemInterface(true /* UTF8 */);
      } else {
        plem = NULL;
      }
      
      PhraseCollectionIndexer idx;
      idx.setLemmatizer(plem);
      idx.indexByConfig(&cfg);
      
      if (bSave) {
        idx.save();
      }
    } catch (std::exception &e) {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}

static void usage()
{
    fprintf(stderr, "Usage: %s [-SL] [-c config]\n", progname);
    fprintf(stderr, "\t-c - use specified config file\n");
    fprintf(stderr, "\t-S - don't save index file\n");
    fprintf(stderr, "\t-L - don't use lemmatizer\n\n");
    
    exit(EX_USAGE);
}
