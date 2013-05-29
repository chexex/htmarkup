/**
 * xmlconfig.cpp
 * parsing XML config-file
 *
 * We strongly rely on expat parser, so state machine is simple:
 * we never check for tag' pairness
 */
#include <cstring>
#include <cassert>

#include "utils/memfile.hpp"
#include "utils/syserror.hpp"
#include "utils/stringutils.hpp"
#include "config.hpp"


XmlConfig::XmlConfig() 
{
}

XmlConfig::XmlConfig(const char *path) 
{
    Load(path);
}

/* static */int
XmlConfig::__unknown_encoding_handler(
    void * /*encodingHandlerData*/,
    const XML_Char *szName,
    XML_Encoding *pEncodingInfo)
{
  if (strcmp(szName, "windows-1251") == 0)
  {
    for (int i = 0; i < 256; i++) {
        pEncodingInfo->map[i] = i;
    }

    pEncodingInfo->convert = NULL;
    pEncodingInfo->data    = NULL;
    pEncodingInfo->release = NULL;
    return 1;
  }

  return 0;
}

void XmlConfig::onTagOpen(const char *tag, const char ** /* attrs */)
{
    if (state_.root_found) {
        if (!state_.cur_section) {
            state_.cur_section = &sections_[tag];
        }
        else if (state_.cur_name.empty()) {
            state_.cur_name = tag;
            state_.cur_text.clear();
        }
        else {
            state_.error_msg = "XmlConfig supports only section:name schema (i.e. max depth=2)";
            XML_StopParser(state_.xmlp, XML_FALSE);
        }
    }
    else {
        if (!strcasecmp(tag, "Config")) {
            state_.root_found = true;
            state_.cur_section = NULL;
        }
        else {
            state_.error_msg = "Root config-section must be called `Config'";
            XML_StopParser(state_.xmlp, XML_FALSE);
        }
    }
}

void XmlConfig::onTagClose(const char *tag)
{
    if (!state_.cur_name.empty()) {
        (*state_.cur_section)[state_.cur_name] = state_.cur_text;
        state_.cur_name.clear();
    }
    else if (state_.cur_section) {
        state_.cur_section = NULL;
    }
    else {
        assert(!strcasecmp(tag, "Config"));
        state_.root_found = false;
        XML_StopParser(state_.xmlp, XML_TRUE);
    }
}

void XmlConfig::onText(const char *data, int len)
{
  if (!state_.cur_name.empty()) {
      for (int i = 0; i < len; i++) {
        if ((unsigned char)(data[i]) == 0xC3)
          state_.cur_text += data[++i] + 0x40;
        else
          state_.cur_text += data[i];
      }
  }
}

bool XmlConfig::Load(const char *path) 
{
    gogo::FileMemHolder mf;
    if (!mf.load(path, false, false)) {
        throw SystemError((std::string)"Failed to load config-file: " + path);
    }

    state_.reset();

    XML_SetUserData(state_.xmlp, this);
    XML_SetElementHandler(state_.xmlp, XmlConfig::__start_adapter, XmlConfig::__end_adapter);
    XML_SetCharacterDataHandler(state_.xmlp, XmlConfig::__characters_adapter);
    XML_SetUnknownEncodingHandler(state_.xmlp, XmlConfig::__unknown_encoding_handler, NULL);

    XML_Status ParseStatus = XML_Parse(state_.xmlp, (char *)mf.get(), mf.size(), true);

    if (ParseStatus == XML_STATUS_ERROR) {
        throw std::runtime_error((std::string)"Error parsing xml config " + path);
    }

    if (!state_.error_msg.empty()) {
        throw std::runtime_error((std::string)"Error parsing xml config " + path + "; " + state_.error_msg);
    }

    state_.free_mem();
    return true;
}

void XmlConfig::GetSections(std::vector<std::string> *lst, const std::string &prefix) const
{
    lst->clear();
    const char *pref = prefix.c_str();
    size_t len = prefix.length();

    for (sections_coll_t::const_iterator it = sections_.begin();
         it != sections_.end();
         it++)
    {
        if (!strncmp(pref, it->first.c_str(), len))
            lst->push_back(it->first);
    }
}

const char *XmlConfig::GetStr(const std::string &section, const std::string &name) const
{
    sections_coll_t::const_iterator it = sections_.find(section);

    if (it == sections_.end())
        return NULL;

    section_t::const_iterator it_val = it->second.find(name);
    return  (it_val == it->second.end()) ? NULL : it_val->second.c_str();
}

void XmlConfig::GetStr(const std::string &section, const std::string &name, std::string &out, const std::string &def) const
{
    const char *pval = GetStr(section, name);
    out = (pval) ?: def;
}

int XmlConfig::GetInt(const std::string &section, const std::string &name, int def) const
{
    const char *pval = GetStr(section, name);
    return (pval) ? atoi(pval) : def;
}

double XmlConfig::GetDouble(const std::string &section, const std::string &name, double def) const
{
    const char *pval = GetStr(section, name);
    return (pval) ? atof(pval) : def;
}

bool XmlConfig::GetBool(const std::string &section, const std::string &name, bool def /* = false */) const
{
    const char *pval = GetStr(section, name);
    return (pval) ? str2bool(pval) : def;
}
