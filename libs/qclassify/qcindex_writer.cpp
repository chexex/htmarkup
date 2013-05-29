//------------------------------------------------------------
/// @file   qcindex_writer.cpp
/// @brief  Query class index writer
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   05.05.2009
//------------------------------------------------------------

#include <string>
#include <vector>
#include "qclassify.hpp"
#include "qclassify_impl.hpp"

using namespace std;
using namespace gogo::QueryClassifierHelper;

namespace gogo
{

QCIndexWriter::QCIndexWriter() {};

/// @brief add query classes from config file
void QCIndexWriter::addConfig(const XmlConfig *pcfg)
{
  vector<string> qclist;
  string name;
  QCPenalties pens;
  
  pcfg->GetSections(&qclist, "QueryClass_");
  
  for (vector<string>::const_iterator it = qclist.begin(); 
       it != qclist.end(); 
       it++) 
  {
    const char *n = it->c_str();
    
    // default - no penalties
    pens.reorder_penalty   = QCPenalties::penaltiToMultiplier( pcfg->GetDouble(n, "ReorderingPenalty", 0.0) );
    pens.partial_penalty   = QCPenalties::penaltiToMultiplier( pcfg->GetDouble(n, "PartialPenalty",    0.0) );
    pens.diff_form_penalty = QCPenalties::penaltiToMultiplier( pcfg->GetDouble(n, "DiffFormPenalty",   0.0) );
    pens.diff_caps_penalty = QCPenalties::penaltiToMultiplier( pcfg->GetDouble(n, "DiffCapsPenalty",   0.0) );
    pens.baseRank          = pcfg->GetDouble(n, "BaseRank", 1.0);
    
    addQClass(XMLtag2QCname(*it), pens);
  }
}

/// @brief add query class with penalties
void QCIndexWriter::addQClass(const std::string &name, const QCPenalties &pens)
{
  QCClass qc;
  
  qc.name = name;
  qc.pens = pens;
  
  m_classes.push_back(qc);
}

// format of QCINDEX serialization:
// [ N ][ {PENALTIES,CLSNAME(ending with \0)}xN ]

/// @brief count size what need for serializing
size_t QCIndexWriter::size() const
{
  size_t len = sizeof(uint32_t) + m_classes.size()*sizeof(QCPenalties);
  for (std::vector<QCClass>::const_iterator it = m_classes.begin(); it != m_classes.end(); it++) {
    len += it->name.length() + 1;
  }
  
  return len;
}

void QCIndexWriter::save(MemWriter &mwr)
{
  std::vector<QCClass>::const_iterator it;
  mwr << (uint32_t)m_classes.size();
  
  for (it = m_classes.begin(); it != m_classes.end(); it++)
    mwr << it->pens << it->name;
}

}
