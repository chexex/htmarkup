//-----------------------------------------------------------------------------
/// @file     qclassify_test.cpp
/// @brief    testing for phrase classification
/// @author   Kisel Jan <kisel@corp.mail.ru>
//-----------------------------------------------------------------------------

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <memory>

#include "config/config.hpp"
#include "lem_interface/lem_interface.hpp"
#include "defs.hpp"
#include "utils/hash_array.hpp"
#include "utils/ptr_array.hpp"
#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"

using namespace std;
using namespace gogo;

lemInterface lem;

class QClassifyTest : public CppUnit::TestFixture
{
  public:
    
    /// @brief test pointer array which used for phrase offset access
    void PtrArrayTest()
    {
      size_t i;
      unsigned arr[4] = {0, 1, 8, 722};
      PtrArrayWriter<uint32_t> pw;
      
      for (i = 0; i < VSIZE(arr); i++) {
        pw.push_back(i * sizeof(unsigned));
      }
      
      auto_ptr_arr<char> region (new char[pw.size() ]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW(pw.save (mwr));
      
      MemReader mrd (region.get());
      PtrArrayReader<uint32_t, unsigned> pr;
      CPPUNIT_ASSERT_NO_THROW(pr.load(mrd));
      
      pr.setBase( reinterpret_cast<const char *>(&arr[0]) );
      for (i = 0; i < VSIZE(arr); i++) {
        const unsigned *p = pr[i];
        CPPUNIT_ASSERT_MESSAGE("Failed to retrieve with []", &arr[i] == p);
        
        const unsigned *p2 = 0;
        CPPUNIT_ASSERT_NO_THROW(p2 = pr.at(i));
        CPPUNIT_ASSERT_MESSAGE("Pointers with [] and ::at are differ!", p2 == p);
      }
      
      CPPUNIT_ASSERT_THROW(pr.at(VSIZE(arr)), std::out_of_range);
      CPPUNIT_ASSERT_THROW(pr.at(VSIZE(arr) + 1), std::out_of_range);
    }
    
    /// @brief testing hash array wich used for word indexing/searching
    void HashArrayTest() 
    {
      HashArrayIndexer<uint32_t, uint32_t> ha(1024);

      ha.add (3, 3);
      ha.add (3, 9);
      ha.add (3, 27);
      ha.add (7, 2178);
      ha.add (5, 555);
      ha.add (2, 2);
      ha.add (2, 4);
      ha.add (2, 8);
      ha.add (2, 16);

      auto_ptr_arr<char> region (new char[ha.size() ]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW(ha.save (mwr));

      MemReader mrd (region.get());
      HashArraySearcher<uint32_t, uint32_t> srch;
      CPPUNIT_ASSERT_NO_THROW(srch.load (mrd));

      uint32_t val;
      CPPUNIT_ASSERT (srch.search (5, val));
      CPPUNIT_ASSERT_EQUAL (555U, val);
      

      CPPUNIT_ASSERT (srch.search (7, val));
      CPPUNIT_ASSERT_EQUAL (2178U, val);
      
      vector<uint32_t> varr;

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array retrieve failed", 4U, srch.search (2, varr));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 2U, varr[0]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 4U, varr[1]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 8U, varr[2]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 16U, varr[3]);
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array retrieve failed", 3U, srch.search (3, varr));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 3U, varr[0]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 9U, varr[1]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array badly retrieved", 27U, varr[2]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Array retrieve failed (should not been found)", 0U, srch.search (111, varr));

      CPPUNIT_ASSERT(!srch.search (0, val));
      CPPUNIT_ASSERT(!srch.search (321, val));
    }
    
    /// @brief bugfix: hash array seems to be fail unless contain elements
    void EmptyHashArrayBugTest()
    {
      HashArrayIndexer<uint32_t, uint32_t> ha;
      
      auto_ptr_arr<char> region (new char[ha.size()]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW(ha.save (mwr));

      MemReader mrd (region.get());
      HashArraySearcher<uint32_t, uint32_t> srch;
      CPPUNIT_ASSERT_NO_THROW(srch.load (mrd));
    }
    
    /// @brief simpliest test ever
    void QCBasicPhraseStorageTest()
    {
      QCBasicPhraseStorage st;
      vector<string> phrases;
      unsigned i;
      
      phrases.push_back("study is good");
      phrases.push_back("playing pool is bad");
      
      for (i = 0; i < phrases.size(); i++)
        st.addPhrase(phrases[i]);
      
      unsigned _offsets[] = {1,0};
      vector<unsigned> voff(_offsets, _offsets + VSIZE(_offsets));
      st.optimize(voff);
      
      auto_ptr_arr<char> region (new char[st.size() ]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW(st.save (mwr));
      
      MemReader mrd (region.get());
      QCBasicPhraseReader reader;
      CPPUNIT_ASSERT_NO_THROW(reader.load(mrd));
      
      for (i = 0; i < phrases.size(); i++) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("phrases not match?!", (string)reader.getPhrase(i), phrases[ voff[i] ]);
      }
      
      CPPUNIT_ASSERT_THROW_MESSAGE ("out of bounds", reader.getPhrase(phrases.size()), std::out_of_range);
    }
    
    /// @brief test scattered string storage
    /// @brief this is (storable) map in fact, so test it like map
    void QCScatteredStringsTest()
    {
      map<unsigned, string> m;
      
      m.insert(make_pair<unsigned, string>(3, "Can I Get A Witness"));
      m.insert(make_pair<unsigned, string>(5, "Look What You've Done"));
      m.insert(make_pair<unsigned, string>(87, "The Under Assistant West Coast"));
      m.insert(make_pair<unsigned, string>(88, "Gotta Get Away"));
      
      QCScatteredStringsWriter ssw;
      map<unsigned, string>::const_iterator it;
      for (it = m.begin(); it != m.end(); it++)
        ssw.add(it->first, it->second);
      
      auto_ptr_arr<char> region (new char[ssw.size() ]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW(ssw.save (mwr));
      
      MemReader mrd (region.get());
      QCScatteredStringsReader ssr;
      CPPUNIT_ASSERT_NO_THROW(ssr.load(mrd));
      
      for (it = m.begin(); it != m.end(); it++) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("bad string loaded?", it->second, (string)ssr.get(it->first));
      }
      
      CPPUNIT_ASSERT_MESSAGE("Should be zero", !ssr.get(1));
      CPPUNIT_ASSERT_MESSAGE("Should be zero", !ssr.get(10000));
    }
    
    
    /// @brief testing what phrases are split correctly
    void PhraseSplitterPlainTest() {
      PhraseSplitterPlain splitter;
      unsigned n;

      n = splitter.split ("ваня вскрыл сейф");
      CPPUNIT_ASSERT_EQUAL (3U, n);
      CPPUNIT_ASSERT_EQUAL (static_cast<size_t>(3), splitter.vWords.size());

      n = splitter.split ("Первый канал обещал - Задорнов выступит");
      CPPUNIT_ASSERT_EQUAL (5U, n);
      CPPUNIT_ASSERT (splitter.vWords[0].upcase);
      CPPUNIT_ASSERT (!splitter.vWords[1].upcase);
      CPPUNIT_ASSERT (!splitter.vWords[2].upcase);
      CPPUNIT_ASSERT (splitter.vWords[3].upcase);
      CPPUNIT_ASSERT (!splitter.vWords[4].upcase);
    }
    
    /// @brief testing what phrases with regular expressions are split correctly
    void PhraseSplitterPCRETest()
    {
      PhraseSplitterPCRE splitter;
      unsigned n;

      n = splitter.split ("ваня");
      CPPUNIT_ASSERT_EQUAL (1U, n);
      CPPUNIT_ASSERT_EQUAL(splitter.getModString(), (string)"\\b\\w+\\b");

      // phrases without regular expressions should be splitten just as in 
      // previous case
      n = splitter.split ("ваня вскрыл сейф");
      CPPUNIT_ASSERT_EQUAL (3U, n);
      CPPUNIT_ASSERT_EQUAL(splitter.getModString(), (string)"\\b\\w+\\b \\b\\w+\\b \\b\\w+\\b");
      
      CPPUNIT_ASSERT_EQUAL(1U, splitter.split ("Вавилова \\d+"));
      CPPUNIT_ASSERT_EQUAL(splitter.getModString(), (string)"\\b\\w+\\b \\d+");
      
      CPPUNIT_ASSERT_EQUAL(2U, splitter.split ("улица Вавилова \\d+"));
      CPPUNIT_ASSERT_EQUAL(splitter.getModString(), (string)"\\b\\w+\\b \\b\\w+\\b \\d+");
      
      CPPUNIT_ASSERT_EQUAL(1U, splitter.split ("\\d+ Вавилова"));
      CPPUNIT_ASSERT_EQUAL(splitter.getModString(), (string)"\\d+ \\b\\w+\\b");
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("Real one RE", 1U, splitter.split ("^(ул\\.?\\s+|улица\\s+)?Вавилова\\s+\\d+[аб]{0,1}?"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("Real one RE - modify", splitter.getModString(), 
                                    (string)"^(ул\\.?\\s+|улица\\s+)?\\b\\w+\\b\\s+\\d+[аб]{0,1}?");
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("No extacted words", 0U, splitter.split ("\\d+"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("RE should not be modified (1)", splitter.getModString(), (string)"\\d+");
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("No extacted words", 0U, splitter.split ("\\d+\\b\\w+\\b[абв]?"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("RE should not be modified (2)", splitter.getModString(), (string)"\\d+\\b\\w+\\b[абв]?");
      
      // very hard RE
      CPPUNIT_ASSERT_EQUAL_MESSAGE("fuckin hard RE", 1U, splitter.split("^(((г\\.|город)\\s*)?(москва|петербург|санкт-петербург)[,\\.]?\\s*)?Гамсоновский\\s+(пер\\.?|переулок)[,\\.]?(\\s*(д\\.?|дом|[-\\.])?\\s*\\d+[аб]?(\\s*\\/\\s+\\d+)?\\s*[\\.,]?(\\s*(к\\.|корп\\.?|корпус|с\\.|стр\\.|строение|[,-\\.])\\s*\\d+)?)?$"));
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE("fuckin hard RE (mod)", (string)"^(((г\\.|город)\\s*)?(москва|петербург|санкт-петербург)[,\\.]?\\s*)?\\b\\w+\\b\\s+(пер\\.?|переулок)[,\\.]?(\\s*(д\\.?|дом|[-\\.])?\\s*\\d+[аб]?(\\s*\\/\\s+\\d+)?\\s*[\\.,]?(\\s*(к\\.|корп\\.?|корпус|с\\.|стр\\.|строение|[,-\\.])\\s*\\d+)?)?$", splitter.getModString());
      
      //CPPUNIT_ASSERT_THROW_MESSAGE ("malformed RE", splitter.split ("\\&ыол[{{{}x,@!\\+??))))?"), std::runtime_error);
    }
    
    /// @brief write and read PCRE phrases
    void PhrasePCREWriterReaderTest()
    {
      PhraseRegExpWriter rewr;
      
      rewr.add(0, "^(ул\\.\\s+)?\\b\\w+\\b \\d+$");
      rewr.add(1, "^(проспект\\s+)?\\b\\w+\\b \\d+(а|б)?$");
      rewr.add(16, "^\\b\\w+\\b \\d+$");
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("amount of saved", 3U, rewr.amount());
      
      auto_ptr_arr<char> region (new char[rewr.size()]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW (rewr.save (mwr));
      
      MemReader mrd(region.get());
      PhraseRegExReader rerd;
      CPPUNIT_ASSERT_NO_THROW (rerd.load(mrd));
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("amount of loaded", 3U, rerd.amount());
      
      // maybe we test libpcre here, but it's not exhaust thing
      CPPUNIT_ASSERT_EQUAL(-1, rerd.match(2, "not existed RE id"));
      CPPUNIT_ASSERT_EQUAL(0, rerd.match(0, "London"));
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(0, "London 22"));
      
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(1, "проспект Ломоносова 16"));
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(1, "Ломоносова 16а"));
      CPPUNIT_ASSERT_EQUAL(0, rerd.match(1, "Ломоносова 16я"));
      
      CPPUNIT_ASSERT_EQUAL(0, rerd.match(16, "Ломоносова 16а"));
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(16, "Ломоносова 16"));
    }
    
    /// @brief write and read PCRE phrases
    void PhrasePCREWriterReaderFlagsTest()
    {
      PhraseRegExpWriter rewr;
      
      rewr.add(0, "^(ул\\.\\s+)?\\b\\w+\\b \\d{2}$", PhraseRegExp::PHRASE_RE_CASELESS);
      rewr.add(1, "^(проспект\\s+)?\\b\\w+\\b \\d+[аб]?$");
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("amount of saved", 2U, rewr.amount());
      
      auto_ptr_arr<char> region (new char[rewr.size()]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW (rewr.save (mwr));
      
      MemReader mrd(region.get());
      PhraseRegExReader rerd;
      CPPUNIT_ASSERT_NO_THROW (rerd.load(mrd));
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("amount of loaded", 2U, rerd.amount());
      
      // maybe we test libpcre here, but it's not exhaust thing
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(0, "ул. Кабаева 14"));
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(0, "Ул. Кабаева 14"));
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(0, "УЛ. Кабаева 14"));
      
      CPPUNIT_ASSERT_EQUAL(1, rerd.match(1, "проспект Жукова 12а"));
      CPPUNIT_ASSERT_EQUAL(0, rerd.match(1, "Проспект Жукова 12а"));
    }

    /// @brief write and read query class list
    void QClassIndexerTest() {
      QCIndexWriter qcw;
      XmlConfig cfg ("cfg/config_2qc.xml");

      QCPenalties pens;
      pens.partial_penalty = 0.5;
      qcw.addQClass ("test", pens);
      qcw.addConfig (&cfg); // there should be 2 query classes
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("class(es) skipped (write)?", (size_t) 3, qcw.amount());

      auto_ptr_arr<char> region (new char[qcw.size() ]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW (qcw.save (mwr));

      QCIndexReader qcr;
      MemReader mrd (region.get());
      CPPUNIT_ASSERT_NO_THROW (qcr.load (mrd));
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("class(es) skipped (read)?", (size_t) 3, qcr.amount());

      bool found = false;
      for (unsigned i = 0; i < qcr.amount(); i++) {
        if (qcr.getName (i) == "test") {
          const QCPenalties &pens = qcr.getPenalties (i);
          CPPUNIT_ASSERT_EQUAL_MESSAGE ("bad penalty loaded", 0.5, pens.partial_penalty);
          CPPUNIT_ASSERT_EQUAL_MESSAGE ("bad penalty default", 1.0, pens.reorder_penalty);
          CPPUNIT_ASSERT_EQUAL_MESSAGE ("bad penalty default", 1.0, pens.diff_form_penalty);
          CPPUNIT_ASSERT_EQUAL_MESSAGE ("bad penalty default", 1.0, pens.diff_caps_penalty);

          found = true;
          break;
        }
      }
      CPPUNIT_ASSERT_EQUAL_MESSAGE ("\"test\" class not found", true, found);
    }
    
    /// @brief test what phrase indexing stat not lies
    void QPhraseIndexerStatTest() {
      PhraseIndexer idx;
      PhraseIndexer::stat st;
      
      idx.addPhrase(0, "Владимир Путин", 100);
      idx.addPhrase(0, "Женевские конвенции", 50);
      idx.addPhrase(0, "Женевские отели", 50);
      idx.addPhrase(1, "Женевские отели", 75);
      
      idx.getStat(&st);
      CPPUNIT_ASSERT_EQUAL(4U, st.nphrases);
      CPPUNIT_ASSERT_EQUAL(3U, st.nphrases_uniq);
      CPPUNIT_ASSERT_EQUAL(8U, st.nwords);
      CPPUNIT_ASSERT_EQUAL(5U, st.nwords_uniq);
    }
    
    /// @brief test what phrase indexer working fine (simple index/search)
    void QPhraseIndexerTest() {
      PhraseIndexer idx(&lem);
      // or use setLemmatizer
      
      idx.addPhrase(22, "Владимир Путин", 100);
      idx.addPhrase(22, "Женевские конвенции", 100);
      idx.addPhrase(22, "Женевские отели", 100);
      idx.addPhrase(22, "автобусная остановка", 100);
      
      
      auto_ptr_arr<char> region (new char[idx.size() ]);
      MemWriter mwr (region.get());
      CPPUNIT_ASSERT_NO_THROW (idx.save (mwr));
      
      MemReader mrd (region.get());
      PhraseSearcher srch;
      srch.setLemmatizer(&lem); // or use in contructor
      CPPUNIT_ASSERT_NO_THROW(srch.load (mrd));
      
      std::vector<PhraseSearcher::phrase_matched> vres;
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found", 1U == srch.searchPhrase("Женевские отели стоимость", vres));
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == PhraseSearcher::MATCH_FL_PARTIAL);
      
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found (lem)", 1U == srch.searchPhrase("автобусные остановки", vres));
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == PhraseSearcher::MATCH_FL_DIFF_FORM);
            
      const int flags_full = PhraseSearcher::MATCH_FL_DIFF_FORM | 
                       PhraseSearcher::MATCH_FL_REORDERED | 
                       PhraseSearcher::MATCH_FL_DIFF_CAPS |
                       PhraseSearcher::MATCH_FL_PARTIAL;
      
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found (lem)", 1U == srch.searchPhrase("на остановки Автобусные проход", vres));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Flags (full) of matching are wrong", flags_full, vres[0].match_flags);
      
      CPPUNIT_ASSERT_MESSAGE("This phrase should not been found", 0U == srch.searchPhrase("частотный анализатор", vres));
      CPPUNIT_ASSERT_MESSAGE("Partial", 0U == srch.searchPhrase("Женевские", vres));
    }
    
    /// @brief index external data and check
    void PhraseCollectionIndexerTest()
    {
      PhraseCollectionIndexer idx(&lem);
      XmlConfig cfg("cfg/config_2qc.xml");
      
      CPPUNIT_ASSERT_NO_THROW(idx.indexByConfig(&cfg));
      CPPUNIT_ASSERT_NO_THROW(idx.save());
      
      bool bLoaded;
      PhraseCollectionLoader ldr(&lem);
      CPPUNIT_ASSERT_NO_THROW(bLoaded = ldr.loadByConfig(&cfg));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, bLoaded);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("PhraseCollectionLoader::loaded lies", true, ldr.is_loaded());
      
      
      std::vector<PhraseSearcher::phrase_matched> vres;
      
      // test exactly matching
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found", 1U == ldr->searchPhrase("портфель", vres));
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == 0);
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found", 1U == ldr->searchPhrase("первое сентября", vres));
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == 0);
      
      // test different flags
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found", 1U == ldr->searchPhrase("учебники", vres));
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == PhraseSearcher::MATCH_FL_DIFF_FORM);
      
      CPPUNIT_ASSERT_MESSAGE("Simple phrase not found", 1U == ldr->searchPhrase("учебники по физике", vres));
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", 
          vres[0].match_flags == (PhraseSearcher::MATCH_FL_DIFF_FORM | PhraseSearcher::MATCH_FL_PARTIAL));
    }
    
    /// @brief index external data (with regular expressions) and check
    void PhraseCollectionIndexerWithRETest()
    {
      PhraseCollectionIndexer idx(&lem);
      XmlConfig cfg("cfg/config_street.xml");
      
      CPPUNIT_ASSERT_NO_THROW(idx.indexByConfig(&cfg));
      CPPUNIT_ASSERT_NO_THROW(idx.save());
      
      PhraseCollectionLoader ldr(&lem);
      CPPUNIT_ASSERT_NO_THROW(ldr.loadByConfig(&cfg));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("PhraseCollectionLoader::is_loaded", true, ldr.is_loaded());
      
      std::vector<PhraseSearcher::phrase_matched> vres;
      
      // test exactly matching
      CPPUNIT_ASSERT_MESSAGE("Should be found", 1U == ldr->searchPhrase("улица Ломоносова 10", vres));      
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == PhraseSearcher::MATCH_FL_PARTIAL);
      
      CPPUNIT_ASSERT_MESSAGE("Should be found", 1U == ldr->searchPhrase("Красного Октября 14 ", vres));      
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == PhraseSearcher::MATCH_FL_PARTIAL);
      
      CPPUNIT_ASSERT_MESSAGE("Should be found", 1U == ldr->searchPhrase("ул. Ломоносова 10", vres));      
      CPPUNIT_ASSERT_MESSAGE("Flags of matching are wrong", vres[0].match_flags == PhraseSearcher::MATCH_FL_PARTIAL);
      
      CPPUNIT_ASSERT_MESSAGE("Should not be found", 0U == ldr->searchPhrase("улица Менделеева 10", vres));
      
      CPPUNIT_ASSERT_MESSAGE("Partial (by RE)", 0U == ldr->searchPhrase("ул. Ломоносова", vres));
      CPPUNIT_ASSERT_MESSAGE("Should not be found (by RE)", 0U == ldr->searchPhrase("улица Ломоносова перекрыта", vres));      
      
      // test different forms order
      const int fl_pd = PhraseSearcher::MATCH_FL_PARTIAL | PhraseSearcher::MATCH_FL_DIFF_FORM,
                fl_pr = PhraseSearcher::MATCH_FL_PARTIAL | PhraseSearcher::MATCH_FL_REORDERED,
                fl_pc = PhraseSearcher::MATCH_FL_PARTIAL | PhraseSearcher::MATCH_FL_DIFF_CAPS,
                fl_all = fl_pd | fl_pr | fl_pc;
                
      
      CPPUNIT_ASSERT_MESSAGE("diff form", 1U == ldr->searchPhrase("ул. Ломоносову 10", vres));
      CPPUNIT_ASSERT_MESSAGE("diff form flag", vres[0].match_flags == fl_pd);
      
      CPPUNIT_ASSERT_MESSAGE("diff caps", 1U == ldr->searchPhrase("ул. ломоносова 10", vres));
      CPPUNIT_ASSERT_MESSAGE("diff caps flag", vres[0].match_flags == fl_pc);
      
      CPPUNIT_ASSERT_MESSAGE("reorder", 1U == ldr->searchPhrase("ул. Октября Красного 14", vres));
      CPPUNIT_ASSERT_MESSAGE("diff form flag", vres[0].match_flags == fl_pr);
      
      CPPUNIT_ASSERT_MESSAGE("reorder+form", 1U == ldr->searchPhrase("ул. Октябрю Красного 14", vres));
      CPPUNIT_ASSERT_MESSAGE("diff form+reorder flag", vres[0].match_flags == (fl_pr | fl_pd));
      
      CPPUNIT_ASSERT_MESSAGE("all penalties in 1", 1U == ldr->searchPhrase("ул. Октябрю красного 14", vres));
      CPPUNIT_ASSERT_MESSAGE("all flags", vres[0].match_flags == fl_all);
      
      CPPUNIT_ASSERT_MESSAGE("Should not be found (RE)", 0U == ldr->searchPhrase("красного №33 октября", vres));
      
      // testing on plain phrase
      CPPUNIT_ASSERT_MESSAGE("Should be found", 1U == ldr->searchPhrase("проспект Коломенский", vres));
      CPPUNIT_ASSERT_MESSAGE("Should be found (not RE)", 1U == ldr->searchPhrase("сегодня проспект 33 Коломенский перкрыт", vres));
      
      // test flags
      CPPUNIT_ASSERT_MESSAGE("diff form (and re-flag: i)", 1U == ldr->searchPhrase("Ул. Ломоносова 10", vres));
      CPPUNIT_ASSERT_MESSAGE("no flags", vres[0].match_flags == PhraseSearcher::MATCH_FL_PARTIAL);
      CPPUNIT_ASSERT_MESSAGE("Should not be found (!re-flag: i)", 0U == ldr->searchPhrase("Улица Красного Октября", vres));
    }
    
    void QPhraseIndexerRankTest()
    {
      PhraseCollectionIndexer idx(&lem);
      XmlConfig cfg("cfg/config_2qc.xml");
      
      CPPUNIT_ASSERT_NO_THROW(idx.indexByConfig(&cfg));
      CPPUNIT_ASSERT_NO_THROW(idx.save());
      
      bool bLoaded;
      PhraseCollectionLoader ldr(&lem);
      CPPUNIT_ASSERT_NO_THROW(bLoaded = ldr.loadByConfig(&cfg));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, bLoaded);
      
      
      PhraseSearcher::res_num_t res;
      unsigned n;
      
      n = ldr->searchPhrase("портфель", res);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("phrase is only one", 1U, n);
      
      n = ldr->searchPhrase("учебник", res);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("phrase present in both classes", 2U, n);
      
      
      PhraseSearcher::res_t r;
      n = ldr->searchPhrase("портфель", r);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("phrase is only one", 1U, n);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("different size of ret and map", n, (unsigned)r.size());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bad class matched?", 100U, r["school"]);
      
      n = ldr->searchPhrase("учебник", r);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("phrase present in both classes", 2U, n);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("different size of ret and map", n, (unsigned)r.size());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bad class matched?", 75U, r["school"]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bad class matched?", 10U, r["hitech"]);
      
      n = ldr->searchPhrase("учебники", r);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("phrase present in both classesб but penalty at for second", 1U, n);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bad class matched?", 75U, r["school"]);
      
      n = ldr->searchPhrase("школьный портфель", r);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Partial phrase should not match", 0U, n);
    }
    
    void QPhraseGetClassesTest() {
      PhraseCollectionIndexer idx(&lem);
      XmlConfig cfg("cfg/config_2qc.xml");
      
      CPPUNIT_ASSERT_NO_THROW(idx.indexByConfig(&cfg));
      CPPUNIT_ASSERT_NO_THROW(idx.save());
      
      PhraseCollectionLoader ldr(&lem);
      CPPUNIT_ASSERT_NO_THROW(true == ldr.loadByConfig(&cfg));
      
      vector<PhraseSearcher::phrase_matched> vmatch;
      
      // Really best notebooks, excuse for advertisment :-)
      unsigned n = ldr->searchPhrase((string)"ноутбук lenovo", vmatch);
      CPPUNIT_ASSERT_EQUAL(3U, n); // "ноутбук", "ноутбуки", "ноутбук lenovo"
      
      std::multimap<string, PhraseSearcher::phrasecls_matched> phraseByCls;
      ldr->getClasses(vmatch, phraseByCls);
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bad ::getClasses distribution", 1U, (unsigned)phraseByCls.count("school"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("bad ::getClasses distribution", 2U, (unsigned)phraseByCls.count("hitech"));
      
      // goddamn!
      pair<multimap<string, PhraseSearcher::phrasecls_matched>::iterator, multimap<string, PhraseSearcher::phrasecls_matched>::iterator> itp;
      multimap<string, PhraseSearcher::phrasecls_matched>::iterator it;
      
      itp = phraseByCls.equal_range("school");
      it = itp.first;
      CPPUNIT_ASSERT_MESSAGE("Match flags", PhraseSearcher::MATCH_FL_PARTIAL == (*it).second.match_flags);
      
      // one phrase should be matched w/o penalties (100%), another - with difform and partial (90%)
      PhraseSearcher::phrasecls_matched *p1 = 0, *p2 = 0;
      
      itp = phraseByCls.equal_range("hitech");
      it  = itp.first;
      
      if (!strcmp("ноутбуки", ldr->getOriginPhrase((*it).second.phrase_id))) 
        p1 = &(*it).second;
      else
        p2 = &(*it).second;
      
      it++;
      if (!strcmp("ноутбук lenovo", ldr->getOriginPhrase((*it).second.phrase_id))) 
        p2 = &(*it).second;
      else
        p1 = &(*it).second;
      
      CPPUNIT_ASSERT_MESSAGE("phrases distributed badly ", p1 && p2);
      
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Match flags (1)", PhraseSearcher::MATCH_FL_PARTIAL | PhraseSearcher::MATCH_FL_DIFF_FORM, p1->match_flags);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Match flags (2)", 0, p2->match_flags);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("baserank (1)", 90U, p1->baserank);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("baserank (2)", 100U, p2->baserank); // surely, this is lenovo! :-)*/
    }
    

    CPPUNIT_TEST_SUITE (QClassifyTest);
      CPPUNIT_TEST (PtrArrayTest);
      CPPUNIT_TEST (HashArrayTest);
      CPPUNIT_TEST (EmptyHashArrayBugTest);
      CPPUNIT_TEST (QCBasicPhraseStorageTest);
      CPPUNIT_TEST (QCScatteredStringsTest);
      CPPUNIT_TEST (PhraseSplitterPlainTest);
      //CPPUNIT_TEST (PhraseSplitterPCRETest);
      //CPPUNIT_TEST (PhrasePCREWriterReaderTest);
      //CPPUNIT_TEST (PhrasePCREWriterReaderFlagsTest);
      CPPUNIT_TEST (QClassIndexerTest);
      CPPUNIT_TEST (QPhraseIndexerStatTest);
      CPPUNIT_TEST (QPhraseIndexerTest);
      CPPUNIT_TEST (PhraseCollectionIndexerTest);
      //CPPUNIT_TEST (PhraseCollectionIndexerWithRETest);
      CPPUNIT_TEST (QPhraseIndexerRankTest);
      CPPUNIT_TEST (QPhraseGetClassesTest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION (QClassifyTest);
