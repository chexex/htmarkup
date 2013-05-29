//-----------------------------------------------------------------------------
/// @file     config_test.cpp
/// @brief    testing for XML-config works fine
/// @author   Kisel Jan <kisel@corp.mail.ru>
//-----------------------------------------------------------------------------

#include <stdexcept>
#include <algorithm>
#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include "config/config.hpp"
#include "utils/syserror.hpp"

class XmlConfigTest : public CppUnit::TestFixture
{
  public:
    
    void LoadTest()
    {
        bool ret;
        XmlConfig cfg;
        CPPUNIT_ASSERT_NO_THROW(ret = cfg.Load("testconfig.xml"));
        CPPUNIT_ASSERT_MESSAGE("Load goes fine, but return value != true?", ret == true);

        CPPUNIT_ASSERT_THROW(cfg.Load("missing_config.xml"), SysError<FileOpenErrFormat>);
        CPPUNIT_ASSERT_THROW(cfg.Load("badconfig.xml"), std::runtime_error);
    }

    void AccessorsTest()
    {
        XmlConfig cfg;
        CPPUNIT_ASSERT_NO_THROW(cfg.Load("testconfig.xml"));

        CPPUNIT_ASSERT_MESSAGE("Surely, option exists", cfg.GetStr("Section1", "Name"));
        CPPUNIT_ASSERT_EQUAL((std::string)"Bunker", (std::string)cfg.GetStr("Section1", "Name"));
        {
            std::string s;
            cfg.GetStr("Section1", "Location", s, "");
            CPPUNIT_ASSERT_EQUAL((std::string)"Jurrasic Park, Sorna isl.", s);
        }
        
        CPPUNIT_ASSERT(!cfg.GetBool("Section1", "ObjectA"));
        CPPUNIT_ASSERT(cfg.GetBool("Section1", "ObjectB"));

        CPPUNIT_ASSERT_EQUAL(10, cfg.GetInt("Section2", "coord_x", 0));
        CPPUNIT_ASSERT_EQUAL(-10, cfg.GetInt("Section2", "coord_y", 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.8, cfg.GetDouble("Section2", "price", 0.0), 0.001);

        // GetSections test
        std::vector<std::string> v;
        cfg.GetSections(&v, "sudo_");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There are 3 sections in config", (size_t)3, v.size());
        std::sort(v.begin(), v.end()); // do not rely on sections order
        CPPUNIT_ASSERT_EQUAL(std::string("sudo_date"),     v[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("sudo_reboot"),   v[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("sudo_shutdown"), v[2]);
        
    }

    void NonEnglishTextTest()
    {
        XmlConfig cfg;

        CPPUNIT_ASSERT_NO_THROW(cfg.Load("testconfig.xml"));
        std::string s;
        cfg.GetStr("Section1", "LocationRU", s, "");
        CPPUNIT_ASSERT_EQUAL((std::string)"Парк Юрского Периода, ост. Сорна", s);
    }

    // Deal with missing sections/values
    void DefaultsTest()
    {
        XmlConfig cfg;
        CPPUNIT_ASSERT_NO_THROW(cfg.Load("testconfig.xml"));

        CPPUNIT_ASSERT(!cfg.GetStr("Section1", "SurName"));
        {
            std::string s;
            cfg.GetStr("Section2", "Location", s, "not_found");
            CPPUNIT_ASSERT_EQUAL((std::string)"not_found", s);
        }

        CPPUNIT_ASSERT(!cfg.GetBool("Section2", "ObjectC"));
        CPPUNIT_ASSERT(!cfg.GetBool("Section3", "ObjectA"));

        CPPUNIT_ASSERT_EQUAL(244, cfg.GetInt("Section2", "coord_z", 244));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(55.555, cfg.GetDouble("Section3", "price", 55.555), 0.001);
    }
    

    CPPUNIT_TEST_SUITE (XmlConfigTest);
      CPPUNIT_TEST (LoadTest);
      CPPUNIT_TEST (AccessorsTest);
      CPPUNIT_TEST (NonEnglishTextTest);
      CPPUNIT_TEST (DefaultsTest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION (XmlConfigTest);
