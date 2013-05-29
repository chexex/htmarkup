//------------------------------------------------------------
/// @file   searcher_aux.cpp
/// @brief  Qclassify phrase searcher auxilary functions
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   09.06.2009
//------------------------------------------------------------

#include <string>
#include <algorithm>
#include "qclassify/qclassify.hpp"

using namespace gogo;
using namespace std;

namespace gogo 
{
  
static bool ResClsNumComparator(const pair<unsigned, PhraseSearcher::phrase_info> &a, 
                                const pair<unsigned, PhraseSearcher::phrase_info> &b) 
{
  return a.second.rank < b.second.rank;
}

static bool ResNumComparator(const pair<unsigned, unsigned> &a, 
                             const pair<unsigned, unsigned> &b) 
{
  return a.second < b.second;
}

static bool ResComparator(const pair<string, unsigned> &a, 
                          const pair<string, unsigned> &b) 
{
  return a.second < b.second;
}

// ----------------------------------------------------------------------------
  
PhraseSearcher::res_cls_num_t::iterator 
PhraseSearcher::selectBest(PhraseSearcher::res_cls_num_t &r) {
  return max_element(r.begin(), r.end(), ResClsNumComparator);
}

PhraseSearcher::res_num_t::iterator
selectBest(PhraseSearcher::res_num_t &r) {
  return max_element(r.begin(), r.end(), ResNumComparator);
}

PhraseSearcher::res_t::iterator
PhraseSearcher::selectBest(PhraseSearcher::res_t &r) {
  return max_element(r.begin(), r.end(), ResComparator);
}
    
} // namespace gogo
