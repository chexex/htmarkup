#ifndef GOGO_LEMINTERFACE_HPP__
#define GOGO_LEMINTERFACE_HPP__

class CMorphology;

class lemInterface {
public:
    lemInterface();
    ~lemInterface();
    bool FirstForm(const std::string &word, std::string *out);

private:
    CMorphology *morph_;
};

#endif // GOGO_LEMINTERFACE_HPP__
