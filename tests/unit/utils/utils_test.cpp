//-----------------------------------------------------------------------------
/// @file     utils_test.cpp
/// @brief    testing for utils library
/// @author   Kisel Jan <kisel@corp.mail.ru>
//-----------------------------------------------------------------------------

#include <stdexcept>
#include <algorithm>
#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Interfaces/cpp/LemInterface.hpp"
#include "utils/stringutils.hpp"

class StringUtilsTest : public CppUnit::TestFixture
{
  public:
    
    void NormalizeTest()
    {
#define CHECK_NORMALIZE(str, normalized_str) { std::string __s(str); strNormalize(__s); CPPUNIT_ASSERT_EQUAL((std::string)normalized_str, __s); }

        CHECK_NORMALIZE("test", "TEST");
        CHECK_NORMALIZE("Test", "TEST");
        CHECK_NORMALIZE("TEST", "TEST");

        CHECK_NORMALIZE("Теперь, по-русски", "ТЕПЕРЬ, ПО-РУССКИ");
        CHECK_NORMALIZE("Теперь, по-русски!!!", "ТЕПЕРЬ, ПО-РУССКИ!!!");

        CHECK_NORMALIZE("Ёжики-мухрёжики", "ЕЖИКИ-МУХРЕЖИКИ");
    }

    void LemInterfaceTest()
    {
        LemInterface lem(true /* UTF8 */);
#define CHECK_LEM(str, lem_str) { std::string __s(str); lem.FirstForm(str, &__s); CPPUNIT_ASSERT_EQUAL((std::string)lem_str, __s); }

        CHECK_LEM("ПАРКОВ", "ПАРК");
        CHECK_LEM("ОКНУ", "ОКНО");
        CHECK_LEM("SIMPLE", "SIMPLE");
    }

    CPPUNIT_TEST_SUITE (StringUtilsTest);
      CPPUNIT_TEST (NormalizeTest);
      CPPUNIT_TEST (LemInterfaceTest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION (StringUtilsTest);
