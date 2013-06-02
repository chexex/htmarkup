//-----------------------------------------------------------------------------
/// @file     qclassify_test.cpp
/// @brief    testing for qclassify based HTML-markup
/// @author   Kisel Jan <kisel@corp.mail.ru>
//-----------------------------------------------------------------------------

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>
#include <sstream>
#include <string>

#include "config/config.hpp"
#include "lem_interface/lem_interface.hpp"
#include "qclassify/qclassify.hpp"
#include "qclassify/htmlmark.hpp"

using namespace std;
using namespace gogo;

extern lemInterface lem;

static const char *CONFIG_PATH_SIMPLE="cfg/config_simple.xml";
static const char *CONFIG_PATH_MARKERCFG="cfg/config_markercfg.xml";

class QCMarkupHtmlTest : public CppUnit::TestFixture
{
  private:
    string EncodeAddress(string text, string enc) 
    {
      stringstream ss;
    
      ss << "<a class=\"gomail_search\" name=\"cln5173\" target=\"_blank\" href=\"http://map.mail.ru/?fullscreen=true&q=" << enc << "\">" << text << "</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\"  width=\"13\" height=\"13\" alt=\"\" />";
    
      return ss.str();
    }
    
    void PrepareIndex() 
    {
      const char *cfgs[] = {CONFIG_PATH_SIMPLE, CONFIG_PATH_MARKERCFG};
    
      for (unsigned i = 0; i < VSIZE(cfgs); i++) {
        PhraseCollectionIndexer idx(&lem);
        XmlConfig cfg(cfgs[i]);
  
        CPPUNIT_ASSERT_NO_THROW(idx.indexByConfig(&cfg));
        CPPUNIT_ASSERT_NO_THROW(idx.save());
      }
    }
    
  public:
  
  void LoadingTest() 
  {
    XmlConfig cfg(CONFIG_PATH_SIMPLE);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
  }
  
  void SimpleMarkupTest() 
  {
    XmlConfig cfg(CONFIG_PATH_SIMPLE);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    
    string s1 = "Ужасные ураганы в подмосковье.";
    
    string os;
    CPPUNIT_ASSERT_EQUAL(1U, mrk.markup(s1, os));
    CPPUNIT_ASSERT_EQUAL(2U, mrk.markup(s1 + s1, os)); // should be marked twice
  }
  
  void MarkupFlagsTest() 
  {
    XmlConfig cfg(CONFIG_PATH_SIMPLE);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    
    string s1 = "Ужасные ураганы в подмосковье. Еще раз - ураган в подмосковье";
    string s2 = "Ужасные ураганы в подмосковье совпали с выходом Fedora 11 Leonidas";
    string s3 = "Ужасные ураганы в подмосковье совпали с выходом Терминатора";
    string s4 = "Терминатора играл Арнольд Шварцнеггер";
    string s5 = "Терминатор и Fedora 11 Leonidas. Ураган в подмосковье был ужасен.";
    string s6 = "Новинки: Терминатор Fedora 11 Leonidas, ураган в подмосковье";
    
    string os;
    
    {
      QCHtmlMarker::MarkupSettings st;
      CPPUNIT_ASSERT_EQUAL(2U, mrk.markup(s1, os));
      st.bUniq = true;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: UNIQ", 1U, mrk.markup(s1, os, st));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: UNIQ (absent)", 2U, mrk.markup(s2, os, st));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: UNIQ (absent)", 2U, mrk.markup(s3, os, st));
    }
    
    {
      QCHtmlMarker::MarkupSettings st;
      st.nmax = 2;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: nmax(1)", 2U, mrk.markup(s2, os, st));
      st.nmax = 1;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: nmax(2)", 1U, mrk.markup(s2, os, st));
      st.nmax = 0;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: nmax(3, all should be marked)", 2U, mrk.markup(s1, os, st));
    }
    
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: gap(0-gap)", 3U, mrk.markup(s6, os));
      
      QCHtmlMarker::MarkupSettings st;
      CPPUNIT_ASSERT_EQUAL(2U, mrk.markup(s2, os));
      st.gap = 3;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: gap(<)", 2U, mrk.markup(s2, os, st));
      st.gap = 4;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: gap(==)", 1U, mrk.markup(s2, os, st));
      st.gap = 6;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: gap(>)", 1U, mrk.markup(s2, os, st));
    }
    
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: skipfirst(0)", 1U, mrk.markup(s4, os));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: skipfirst(00)", 3U, mrk.markup(s5, os));
      
      QCHtmlMarker::MarkupSettings st;
      st.bSkipFirstWord = true;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: skipfirst(1)", 0U, mrk.markup(s4, os, st));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("flags: skipfirst(2)", 1U, mrk.markup(s5, os, st));
    }
  }
  
  void MarkerTest() 
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    
    {
      string os;
      mrk.loadSettings(&cfg);
      mrk.markup("наши терминаторы наступают!", os);
      CPPUNIT_ASSERT_EQUAL((string)"наши <a href=\"http://www.wikipedia.org/Терминатор\">терминаторы</a> наступают!", os);
    }
    
    // skip first word, remember
    {
      string os;
      mrk.loadSettings(&cfg);
      mrk.markup("терминаторы наступают!", os);
      CPPUNIT_ASSERT_EQUAL((string)"терминаторы наступают!", os);
    }
    
    // here we expect %U replacement too
    {
      string os;
      mrk.loadSettings(&cfg);
      mrk.markup("дистрибутив Ubuntu а также Fedora Core", os);
      CPPUNIT_ASSERT_EQUAL((string)"дистрибутив <a href=\"http://www.gogo.ru/search?q=ubuntu\">Ubuntu</a> а также <a href=\"http://www.fedoraproject.org\">Fedora Core</a>", os);
    }
  }
  
  void LoadConfigSettingsTest()
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    QCHtmlMarker::MarkupSettings def;
    def = mrk.getConfigSettings();
    
    CPPUNIT_ASSERT_EQUAL(4U, def.range);
    CPPUNIT_ASSERT_EQUAL(0U, def.gap);
    CPPUNIT_ASSERT_EQUAL(3U, def.nmax);
    CPPUNIT_ASSERT_EQUAL(true, def.bSkipFirstWord);
    CPPUNIT_ASSERT_EQUAL(true, def.bUniq);
    CPPUNIT_ASSERT_EQUAL(QCHtmlMarker::MARKUP_ORDER_RANK_DESC, def.order);
  }
  
  void MarkerNothingToMarkTest()
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
  
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    string s = "<p>&laquo;Ленинград&raquo; так и не был&nbsp;взят</p>";
    
    string os;
    CPPUNIT_ASSERT_EQUAL(0U, mrk.markup(s, os));
    CPPUNIT_ASSERT_EQUAL(s, os);
  }
  
  void MarkerEncodeTest()
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
  
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    
    QCHtmlMarker::MarkupSettings st = mrk.getConfigSettings();
    st.bSkipFirstWord = false;
    
    string os;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Казань' (n)", 1U, mrk.markup("Казань", os, st));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Казань'", EncodeAddress("Казань", "0JrQsNC30LDQvdGM"), os);
    
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Ростов-на-Дону' (n)", 1U, mrk.markup("Ростов-на-Дону", os, st));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Ростов-на-Дону'", EncodeAddress("Ростов-на-Дону", "0KDQvtGB0YLQvtCyLdC90LAt0JTQvtC90YM%3D"), os);
  }
  
  void MarkerSkipEscapesProperlyTest()
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    string os;
    
    string input = "<p></p><p>МОСКВА, 14 июн&nbsp;&mdash; РИА Новости. &laquo;Москва&raquo; ушла на&nbsp;перерыв в&nbsp;чемпионате России по&nbsp;футболу в&nbsp;ранге лидера, выиграв в&nbsp;матче 12-го тура со&nbsp;счетом 2:0 в&nbsp;столичном дерби у&nbsp;ЦСКА.</p><p>Результаты остальных матчей, сыгранных в&nbsp;российской премьер-лиге в&nbsp;выходные:</p><p>&laquo;Рубин&raquo; (Казань)&nbsp;&mdash; &laquo;Ростов&raquo; (Ростов-на-Дону)&nbsp;&mdash; 0:2,</p><p>&laquo;Спартак&raquo; (Москва)&nbsp;&mdash; &laquo;Химки&raquo; (Московская область)&nbsp;&mdash; 1:0,</p><p>&laquo;Динамо&raquo; (Москва)&nbsp;&mdash; &laquo;Кубань&raquo; (Краснодар)&nbsp;&mdash; 1:1,</p><p>&laquo;Терек&raquo; (Грозный)&nbsp;&mdash; &laquo;Крылья Советов&raquo; (Самара)&nbsp;&mdash; 3:2,</p><p>&laquo;Томь&raquo; (Томск)&nbsp;&mdash; &laquo;Амкар&raquo; (Пермь)&nbsp;&mdash; 1:2,</p><p>&laquo;Сатурн&raquo; (Московская область)&nbsp;&mdash; &laquo;Локомотив&raquo; (Москва)&nbsp;&mdash; 2:0,</p><p>&laquo;Спартак&raquo; (Нальчик)&nbsp;&mdash; &laquo;Зенит&raquo; (Санкт-Петербург)&nbsp;&mdash; 2:2.</p><p>Турнирная таблица чемпионата России:</p><p>1. &laquo;Москва&raquo;&nbsp;&mdash; 23 очка (12 матчей),</p><p>2. &laquo;Рубин&raquo;&nbsp;&mdash; 21 (11),</p><p>3. &laquo;Спартак&raquo; М&nbsp;&mdash; 20 (12),</p><p>4. &laquo;Крылья Советов&raquo;&nbsp;&mdash; 20 (12),</p><p>5. &laquo;Динамо&raquo;&nbsp;&mdash; 20 (12),</p><p>6. ЦСКА&nbsp;&mdash; 19 (11),</p><p>7. &laquo;Зенит&raquo;&nbsp;&mdash; 19 (12),</p><p>8. &laquo;Ростов&raquo;&nbsp;&mdash; 18 (12),</p><p>9. &laquo;Локомотив&raquo;&nbsp;&mdash; 16 (12),</p><p>10. &laquo;Терек&raquo;&nbsp;&mdash; 16 (12),</p><p>11. &laquo;Кубань&raquo;&nbsp;&mdash; 13 (12),</p><p>12. &laquo;Сатурн&raquo;&nbsp;&mdash; 13 (12),</p><p>13. &laquo;Томь&raquo;&nbsp;&mdash; 12 (12),</p><p>14. &laquo;Амкар&raquo;&nbsp;&mdash; 11 (12),</p><p>15. &laquo;Спартак&raquo; Нч&nbsp;&mdash; 8 (11),</p><p>16. &laquo;Химки&raquo;&nbsp;&mdash; 4 (11).</p>";
    
    // marked string (tested with firefox)
    string output = "<p></p><p>МОСКВА, 14 июн&nbsp;&mdash; РИА Новости. &laquo;Москва&raquo; ушла на&nbsp;перерыв в&nbsp;чемпионате России по&nbsp;футболу в&nbsp;ранге лидера, выиграв в&nbsp;матче 12-го тура со&nbsp;счетом 2:0 в&nbsp;столичном дерби у&nbsp;ЦСКА.</p><p>Результаты остальных матчей, сыгранных в&nbsp;российской премьер-лиге в&nbsp;выходные:</p><p>&laquo;Рубин&raquo; (<a class=\"gomail_search\" name=\"cln5173\" target=\"_blank\" href=\"http://map.mail.ru/?fullscreen=true&q=0JrQsNC30LDQvdGM\">Казань</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\"  width=\"13\" height=\"13\" alt=\"\" />)&nbsp;&mdash; &laquo;Ростов&raquo; (<a class=\"gomail_search\" name=\"cln5173\" target=\"_blank\" href=\"http://map.mail.ru/?fullscreen=true&q=0KDQvtGB0YLQvtCyLdC90LAt0JTQvtC90YM%3D\">Ростов-на-Дону</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\"  width=\"13\" height=\"13\" alt=\"\" />)&nbsp;&mdash; 0:2,</p><p>&laquo;Спартак&raquo; (Москва)&nbsp;&mdash; &laquo;Химки&raquo; (Московская область)&nbsp;&mdash; 1:0,</p><p>&laquo;Динамо&raquo; (Москва)&nbsp;&mdash; &laquo;Кубань&raquo; (<a class=\"gomail_search\" name=\"cln5173\" target=\"_blank\" href=\"http://map.mail.ru/?fullscreen=true&q=0JrRgNCw0YHQvdC%2B0LTQsNGA\">Краснодар</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\"  width=\"13\" height=\"13\" alt=\"\" />)&nbsp;&mdash; 1:1,</p><p>&laquo;Терек&raquo; (Грозный)&nbsp;&mdash; &laquo;Крылья Советов&raquo; (Самара)&nbsp;&mdash; 3:2,</p><p>&laquo;Томь&raquo; (Томск)&nbsp;&mdash; &laquo;Амкар&raquo; (Пермь)&nbsp;&mdash; 1:2,</p><p>&laquo;Сатурн&raquo; (Московская область)&nbsp;&mdash; &laquo;Локомотив&raquo; (Москва)&nbsp;&mdash; 2:0,</p><p>&laquo;Спартак&raquo; (Нальчик)&nbsp;&mdash; &laquo;Зенит&raquo; (Санкт-Петербург)&nbsp;&mdash; 2:2.</p><p>Турнирная таблица чемпионата России:</p><p>1. &laquo;Москва&raquo;&nbsp;&mdash; 23 очка (12 матчей),</p><p>2. &laquo;Рубин&raquo;&nbsp;&mdash; 21 (11),</p><p>3. &laquo;Спартак&raquo; М&nbsp;&mdash; 20 (12),</p><p>4. &laquo;Крылья Советов&raquo;&nbsp;&mdash; 20 (12),</p><p>5. &laquo;Динамо&raquo;&nbsp;&mdash; 20 (12),</p><p>6. ЦСКА&nbsp;&mdash; 19 (11),</p><p>7. &laquo;Зенит&raquo;&nbsp;&mdash; 19 (12),</p><p>8. &laquo;Ростов&raquo;&nbsp;&mdash; 18 (12),</p><p>9. &laquo;Локомотив&raquo;&nbsp;&mdash; 16 (12),</p><p>10. &laquo;Терек&raquo;&nbsp;&mdash; 16 (12),</p><p>11. &laquo;Кубань&raquo;&nbsp;&mdash; 13 (12),</p><p>12. &laquo;Сатурн&raquo;&nbsp;&mdash; 13 (12),</p><p>13. &laquo;Томь&raquo;&nbsp;&mdash; 12 (12),</p><p>14. &laquo;Амкар&raquo;&nbsp;&mdash; 11 (12),</p><p>15. &laquo;Спартак&raquo; Нч&nbsp;&mdash; 8 (11),</p><p>16. &laquo;Химки&raquo;&nbsp;&mdash; 4 (11).</p>";
    
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of marked is wrong", 3U, mrk.markup(input, os));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong escaping?", output, os);
    
    input = "<p>Алматы. 23 июня. &laquo;Казахстан Сегодня&raquo;&nbsp;&mdash; В Алматы 5 июля состоится казахстанская альпиниада &laquo;Нурсултан-2009&raquo;. Об этом сегодня сообщил на&nbsp;пресс-конференции вице-президент Национального олимпийского комитета (НОК) РК Павел Новиков, передает корреспондент агентства.<br /><br />П. Новиков сообщил, что для&nbsp;участия в&nbsp;альпиниаде приглашаются все желающие. <br /><br />Как рассказал П. Новиков, альпиниада проводится с&nbsp;целью подготовки спасательных служб города к&nbsp;проведению VII зимних Азиатских игр, подготовки горных подразделений служб безопасности РК по&nbsp;обеспечению безопасности в&nbsp;горных условиях, для&nbsp;дальнейшего привлечения населения города Алматы к&nbsp;здоровому образу жизни, для&nbsp;привлечения внимания руководителей всех уровней к&nbsp;постановке спортивно-массовой и&nbsp;физкультурно-оздоровительной работы.<br /><br />По словам П. Новикова, участники массового восхождения смогут добраться по&nbsp;канатной дороге от&nbsp;горнолыжного курорта &laquo;Шымбулак&raquo; (2200 метров над&nbsp;уровнем моря) до&nbsp;перевала Талгарский (3200 метров). Проезд бесплатный. Затем начнется восхождение с&nbsp;перевала Талгарский до&nbsp;перевала Карлытау (3800 метров). <br /><br />&laquo;Главное для&nbsp;нас&nbsp;&mdash; безопасность людей, поэтому маршрут будет промаркирован сигнальными флажками, а&nbsp;в случае тумана мы расставим людей&raquo;,&nbsp;&mdash; отметил П. Новиков. Далее участникам предстоит пройти скальную часть восхождения по&nbsp;перильным веревкам от&nbsp;перевала Карлытау до&nbsp;вершины пика &laquo;Нурсултан&raquo; (4376 метров).<br /><br />Кроме того, будет организован скоростной забег от&nbsp;гостиницы Шымбулак. Финиш скоростного забега у&nbsp;начала перильной части восхождения (4115 метров).<br /><br />По его словам, победители забега, занявшие призовые места, будут награждены дипломами НОК и&nbsp;ценными призами. Призы и&nbsp;грамоты получат лучший ветеран восхождения и&nbsp;самый юный участник альпиниады. Всем участникам альпиниады будут вручены сертификаты и&nbsp;памятные знаки &laquo;Нурсултан-2009&raquo;.<br /><br />&laquo;Все организационные вопросы решены, люди будут обеспечены транспортом. Будет открыто два пункта медицинской помощи и&nbsp;три пункта питания. Во время альпиниады будет дежурить вертолет &ldquo;Казавиаспаса&rdquo;,&nbsp;&mdash; рассказал П. Новиков.<br /><br />При использовании информации гиперссылка на&nbsp;информационное агентство &ldquo;Казахстан Сегодня&rdquo; обязательна. Авторские права на&nbsp;материалы агентства.</p>";
    
    output = "<p>Алматы. 23 июня. &laquo;Казахстан Сегодня&raquo;&nbsp;&mdash; В <a class=\"gomail_search\" name=\"cln5173\" target=\"_blank\" href=\"http://map.mail.ru/?fullscreen=true&q=0JDQu9C80LDRgtGL\">Алматы</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\"  width=\"13\" height=\"13\" alt=\"\" /> 5 июля состоится казахстанская альпиниада &laquo;Нурсултан-2009&raquo;. Об этом сегодня сообщил на&nbsp;пресс-конференции вице-президент Национального олимпийского комитета (НОК) РК Павел Новиков, передает корреспондент агентства.<br /><br />П. Новиков сообщил, что для&nbsp;участия в&nbsp;альпиниаде приглашаются все желающие. <br /><br />Как рассказал П. Новиков, альпиниада проводится с&nbsp;целью подготовки спасательных служб города к&nbsp;проведению VII зимних Азиатских игр, подготовки горных подразделений служб безопасности РК по&nbsp;обеспечению безопасности в&nbsp;горных условиях, для&nbsp;дальнейшего привлечения населения города Алматы к&nbsp;здоровому образу жизни, для&nbsp;привлечения внимания руководителей всех уровней к&nbsp;постановке спортивно-массовой и&nbsp;физкультурно-оздоровительной работы.<br /><br />По словам П. Новикова, участники массового восхождения смогут добраться по&nbsp;канатной дороге от&nbsp;горнолыжного курорта &laquo;Шымбулак&raquo; (2200 метров над&nbsp;уровнем моря) до&nbsp;перевала Талгарский (3200 метров). Проезд бесплатный. Затем начнется восхождение с&nbsp;перевала Талгарский до&nbsp;перевала Карлытау (3800 метров). <br /><br />&laquo;Главное для&nbsp;нас&nbsp;&mdash; безопасность людей, поэтому маршрут будет промаркирован сигнальными флажками, а&nbsp;в случае тумана мы расставим людей&raquo;,&nbsp;&mdash; отметил П. Новиков. Далее участникам предстоит пройти скальную часть восхождения по&nbsp;перильным веревкам от&nbsp;перевала Карлытау до&nbsp;вершины пика &laquo;Нурсултан&raquo; (4376 метров).<br /><br />Кроме того, будет организован скоростной забег от&nbsp;гостиницы Шымбулак. Финиш скоростного забега у&nbsp;начала перильной части восхождения (4115 метров).<br /><br />По его словам, победители забега, занявшие призовые места, будут награждены дипломами НОК и&nbsp;ценными призами. Призы и&nbsp;грамоты получат лучший ветеран восхождения и&nbsp;самый юный участник альпиниады. Всем участникам альпиниады будут вручены сертификаты и&nbsp;памятные знаки &laquo;Нурсултан-2009&raquo;.<br /><br />&laquo;Все организационные вопросы решены, люди будут обеспечены транспортом. Будет открыто два пункта медицинской помощи и&nbsp;три пункта питания. Во время альпиниады будет дежурить вертолет &ldquo;Казавиаспаса&rdquo;,&nbsp;&mdash; рассказал П. Новиков.<br /><br />При использовании информации гиперссылка на&nbsp;информационное агентство &ldquo;Казахстан Сегодня&rdquo; обязательна. Авторские права на&nbsp;материалы агентства.</p>";

    
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of marked is wrong", 1U, mrk.markup(input, os));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong escaping?", output, os);
  }
  
  void UseUdataAsFormatMeaningTest()
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    string os;
    const string input = "Have you ever taste Carte Noire or another one - Nescafe coffees?", 
                 output= "Have you ever taste <a href=\"http://www.starbucks.com/premium-catalog/?id=3\" title=\"Carte Noire\">"
                         "Carte Noire</a> or another one - "
                         "<a href=\"http://www.starbucks.com/catalog/?id=12\" title=\"Nescafe\">Nescafe</a> coffees?";
        
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of marked is wrong", 2U, mrk.markup(input, os));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong markup", output, os);
  }
  
  
  // Т.к. спецмаркеры вынесены за границу hex, то url-escaped строка должна остаться не тронутой
  // Во входном файле для данного класса строка маркера содержит url-escaping
  void UrlEscapedStringSkipWell()
  { 
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    
    std::string os;
    
    const string input = "did you mention what kdevelop 4 have cool C++ parser";
    const string output = "did you mention what <a href=\"q=%EF%E0%F0%F1%E5%F0\">KDevelop</a> 4 have cool C++ parser";
    
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of marked is wrong", 1U, mrk.markup(input, os));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong escaping?", output, os);
  }
  
  void EscapingSpecialMarkersTest()
  {
    XmlConfig cfg(CONFIG_PATH_MARKERCFG);
    PhraseCollectionLoader ldr(&lem);
    CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL_MESSAGE("Phrase index loading failed", true, ldr.loadByConfig(&cfg)));
    
    QCHtmlMarker mrk(ldr.getSearcher());
    mrk.loadSettings(&cfg);
    
    std::string os;
    
    const string input = "Глава 2: длинный посох с которым он путешествовал последние 20 лет";
    // href: %Q; title: %S
    const string output = "Глава 2: <a href=\"%D0%B4%D0%BB%D0%B8%D0%BD%D0%BD%D1%8B%D0%B9+%D0%BF%D0%BE%D1%81%D0%BE%D1%85\" title=\"%D0%94%D0%BB%D0%B8%D0%BD%D0%BD%D1%8B%D0%B9+%D0%9F%D0%BE%D1%81%D0%BE%D1%85\">длинный посох</a> с которым он путешествовал последние 20 лет";
    
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of marked is wrong", 1U, mrk.markup(input, os) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong escaping?", output, os );
  }
  
  public:
    CPPUNIT_TEST_SUITE (QCMarkupHtmlTest);
        CPPUNIT_TEST (PrepareIndex);
        CPPUNIT_TEST (LoadingTest);
        CPPUNIT_TEST (SimpleMarkupTest);
        CPPUNIT_TEST (MarkupFlagsTest);
        CPPUNIT_TEST (MarkerTest);
        CPPUNIT_TEST (LoadConfigSettingsTest);
        CPPUNIT_TEST (MarkerNothingToMarkTest);
        CPPUNIT_TEST (MarkerEncodeTest);
        CPPUNIT_TEST (MarkerSkipEscapesProperlyTest);
        CPPUNIT_TEST (UseUdataAsFormatMeaningTest);
        CPPUNIT_TEST (UrlEscapedStringSkipWell);
        CPPUNIT_TEST (EscapingSpecialMarkersTest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION (QCMarkupHtmlTest);
