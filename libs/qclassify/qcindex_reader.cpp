//------------------------------------------------------------
/// @file   qcindex_reader.cpp
/// @brief  Query class index reader
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   05.05.2009
//------------------------------------------------------------

#include <string>
#include <vector>
#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"

using namespace std;
using namespace gogo::QueryClassifierHelper;

namespace gogo
{

void QCIndexReader::load(MemReader &mrd)
{
  uint32_t n;
  
  mrd >> n;
  m_classes.reserve(n);
  while (n--) {
    QCClass qc;
    
    mrd >> qc.pens >> qc.name;
    m_classes.push_back(qc);
  }
}

/// @brief override loaded penalties and baseRank from config
void QCIndexReader::mergeConfig(const XmlConfig *pcfg)
{
  
}

}
