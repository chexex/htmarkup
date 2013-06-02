#include <stdexcept>
#include "icuincls.h"

void
UnicodeString2UTF8(const UnicodeString &us, std::string *os)
{
    UErrorCode status = U_ZERO_ERROR;
    os->resize(us.length() * 3);

    int32_t len = us.extract(const_cast<char *>(os->data()), os->size()+1, NULL, status);

    if (U_ZERO_ERROR != status || len >= os->size()) {
        throw std::runtime_error("Failed to convert UnicodeString to UTF-8");
    }

    os->resize(len);
}

UnicodeString
UTF8toUnicodeString(const std::string &s)
{
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString us(s.c_str(), s.length(), NULL, status);

    if (U_ZERO_ERROR != status) {
        throw std::runtime_error("Failed to convert UTF-8 to UnicodeString");
    }
    return us;
}
