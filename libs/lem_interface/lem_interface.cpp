#include <stdexcept>
#include <CMorphology.hpp>

#include "lem_interface.hpp"
#include "utils/conv.hpp"

lemInterface::lemInterface() {
    morph_ = new CMorphology();

    if (!morph_->Agramtab.LoadFromRegistry() || !morph_->RussianDict.LoadDictionariesRegistry()) {
        delete morph_;
        throw std::runtime_error("Failed to load russiandictionary");
    }
}

lemInterface::~lemInterface() {
    delete morph_;
}

/**
 * Input and output is always UTF-8
 * but lemmatizer itself works only with CP1251.
 * So, we have to convert given `word'
 */
bool lemInterface::FirstForm(const std::string &word, std::string *out)
{
    std::string word_cp1251;
    gogo::conv_str(gogo::CONV_UTF8, gogo::CONV_CP1251, word, &word_cp1251);

    std::vector<std::string> fforms = morph_->GetFirstForm(morphRussian, word_cp1251);
    if (fforms.empty())
        return false;

    gogo::conv_str(gogo::CONV_CP1251, gogo::CONV_UTF8, fforms[0], out);
    return true;
}
