//-------------------------------------------------------------------
/// @file projects/classify/qcmarker.cpp
/// @brief html marker test frontend
/// @author Kisel Jan
/// @date 28 october 2009
//-------------------------------------------------------------------

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>

#include "config/config.hpp"
#include "utils/syserror.hpp"
#include "lem_interface/lem_interface.hpp"
#include "qclassify/qclassify.hpp"
#include "qclassify/htmlmark.hpp"

using namespace std;
using namespace gogo;

namespace {
  unsigned markupFile(QCHtmlMarker &marker, const char *file);
  void usage();
}

static inline const char *
last_error() { return strerror(errno); }

int
main(int argc, char *argv[])
{
  char *cfg_path = (char *)"config.xml";
  bool  nolemm = false, infinite = false;
  
  
  for(;;)
  {
    static struct option options[] = {
      {"config",   required_argument, 0, 'c'},
      {"infinite", no_argument,       0, 'i'},
      {"nolemm",   no_argument,       0, 'L'},
      {"help",     no_argument,       0, 'h'},
      {0,0,0,0}
    };
    
    int c = getopt_long(argc, argv, "c:Li", options, NULL);
    if (c == -1)
      break;
    
    switch(c) {
      case 'c':
        cfg_path = optarg;
        break;
        
      case 'i':
        infinite = true;
        break;
        
      case 'L':
        nolemm = true;
        break;
        
      case 'h':
      default:
        usage();
    }
  }
  
  argc -= optind;
  argv += optind;
  
  static char *stdin_only[] = { (char *)"-", NULL };
  char **paths = (argc) ? argv : stdin_only;
  
  if (paths == stdin_only && infinite) {
    usage();
  }
  
  try 
  {
    auto_ptr<lemInterface> plem;
    XmlConfig cfg(cfg_path);
    
    if (cfg.GetBool("QueryQualifier", "UseLemmatizer", true))
      plem.reset(new lemInterface);
    
    PhraseCollectionLoader ldr(plem.get());
    if (!ldr.loadByConfig(&cfg)) {
      throw runtime_error("Failed to load phrase index");
    }
    
    QCHtmlMarker marker(ldr.getSearcher());
    marker.loadSettings(&cfg);
    
    do {
      for(int i = 0; paths[i]; i++)
        markupFile(marker, paths[i]);
    } while (infinite);
  }
  catch(exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }
  catch(...) {
    cerr << "Unknown error!" << endl;
    exit(EXIT_FAILURE);
  }
  
  
  return EXIT_SUCCESS;
}

namespace {
  
unsigned
markupFile(QCHtmlMarker &marker, const char *file) 
{
  string content, out;
  ifstream f;
  
  if (strcmp(file, "-")) {
    f.open(file);
    if (!f.is_open()) {
      throw SystemError((std::string)"failed to open file " + file);
    }
  }
  
  istream &is = (f.is_open()) ? f : cin;
  string line;
  
  while (!is.eof()) {
    getline(is, line);
    content += line;
    content += "\n";
  }
  
  unsigned n = marker.markup(content, out);
  
  cout << "==== " << n << " phrases marked" << endl;
  cout << out;
  
  return n;
}

void 
usage()
{
  cerr << "Usage: qcmarker [options] file ...\n"
          "options are:\n"
          "\t-c|--config: use specified config instead of config.xml\n"
          "\t-i|--infinite: mark infinite (DON'T USE WITH STDIN)\n"
          "\t-L|--nolemm: don't use lemmatizer\n"
          "\t-h|--hepl: print this help\n\n";
  
  exit(EX_USAGE);
}

} // anon namespace
