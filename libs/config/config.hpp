#ifndef GOGO_CONFIG_HPP__
#define GOGO_CONFIG_HPP__

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

#include <expat.h>

class XmlConfig {
public:
    XmlConfig();
    XmlConfig(const char *path);
    bool Load(const char *path);

    const char *GetStr(const std::string &section, const std::string &name) const;
    void        GetStr(const std::string &section, const std::string &name, std::string &out, const std::string &def) const;
    int         GetInt(const std::string &section, const std::string &name, int def) const;
    double      GetDouble(const std::string &section, const std::string &name, double def) const;
    bool        GetBool(const std::string &section, const std::string &name, bool def = false) const;

    /**
     * Get list of section-names wich has given prefix
     */
    void GetSections(std::vector<std::string> *lst, const std::string &prefix) const;

    typedef std::map<std::string, std::string> section_t;

    struct parser_state {
        XML_Parser xmlp;
        bool root_found;
        XmlConfig::section_t *cur_section;
        std::string cur_name;
        std::string cur_text;

        std::string error_msg;

        parser_state () : xmlp(NULL), root_found(false) {}

        void reset() {
            free_mem();
            xmlp = XML_ParserCreate(NULL);
            if (!xmlp)
                throw std::runtime_error("Failed to create XML_Parser");
        }

        void free_mem() {
            if (xmlp) {
                XML_ParserFree(xmlp);
                xmlp = 0;
            }
            root_found = false;
            error_msg.clear();
        }
    };

private:
    typedef std::map<std::string, section_t> sections_coll_t;
    sections_coll_t sections_;

    void onTagOpen(const char *tag, const char **attrs);
    void onTagClose(const char *tag);
    void onText(const char *data, int len);


    static void __start_adapter(void *data, const char *el, const char **attrs)  { 
        static_cast<XmlConfig*>(data)->onTagOpen(el, attrs);
    }

    static void __end_adapter(void *data, const char *el)  { 
        static_cast<XmlConfig*>(data)->onTagClose(el);
    }

    static void __characters_adapter(void *data, const char *chars, int len)  { 
        static_cast<XmlConfig*>(data)->onText(chars, len);
    }

    static int __unknown_encoding_handler(void *encodingHandlerData, 
                                          const XML_Char *szName,
                                          XML_Encoding *pEncodingInfo);

    parser_state state_;
};

#endif // GOGO_CONFIG_HPP__
