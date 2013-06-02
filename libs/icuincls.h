#ifndef ICUINCLS_HPP__
#define ICUINCLS_HPP__

#include <string>

// libicu
#include <unicode/utypes.h>
#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

void UnicodeString2UTF8(const UnicodeString &us, std::string *os);
UnicodeString UTF8toUnicodeString(const std::string &s);

#endif
