#include <assert.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include <stdexcept>

#include "conv.hpp"

namespace gogo
{

const char *g_enc_names[] = { "UTF-8", "CP1251" };

bool conv_str(conv_enc_t from, conv_enc_t to, const char *in, std::string *out, size_t in_len /* = size_t(-1) */)
{
    out->clear();
    iconv_t cd = iconv_open(g_enc_names[to], g_enc_names[from]);
   
    if (in_len == size_t(-1))
        in_len = strlen(in);

    size_t outbytesleft = (to == CONV_UTF8 && from != CONV_UTF8) ? in_len * 4 : in_len;
    out->resize(outbytesleft);

    char *outbuf = const_cast<char *>(out->data());
    char *inbuf = const_cast<char *>(in);

    size_t ret = iconv(cd, &inbuf, &in_len, &outbuf, &outbytesleft);
    iconv_close(cd);

    if (in_len != 0) {
        assert(ret != E2BIG);
        if (ret == EILSEQ || ret == EINVAL) {
            return false;
        }
        throw std::invalid_argument("Should not be there assertion");
        //assert(0 && "Should not be there");
    }

    out->resize(out->size() - outbytesleft);
    return true;
}

} // namespace gogo
