#ifndef SYSERROR_HPP__
#define SYSERROR_HPP__

#include <string>
#include <stdexcept>
#include <sstream>
#include <cerrno>
#include <cstring>

struct SysErrFormat {
  void operator()(const char *a_text, int a_errno, std::stringstream &ss) 
  {
    ss << "system error: " << a_text << "; errno=" << a_errno << 
        " (" << strerror(a_errno) << ")";
  }
};

struct FileOpenErrFormat {
  void operator()(const char *a_text, int a_errno, std::stringstream &ss) 
  {
    ss << "failed to open file: " << a_text << "; errno=" << a_errno << 
        " (" << strerror(a_errno) << ")";
  }
};

template<class FormatStrategy = SysErrFormat>
class SysError : public std::exception {
  protected:
    std::string msg_;
    int saved_errno_;
  
  public:
    int getErrno() const { return saved_errno_; }
    
    /// @brief  format message according to selected strategy
    void SetErrorMessage(const char *s) {
      saved_errno_ = errno;
      std::stringstream ss;  
      FormatStrategy()(s, saved_errno_, ss);
      msg_ = ss.str();
    }
    
    SysError(const char *s) throw() {
      SetErrorMessage(s);
    }
    SysError(const std::string &s) throw() {
      SetErrorMessage(s.c_str());
    }
    
    virtual const char *what() const throw() { return msg_.c_str(); }
    virtual ~SysError() throw() {}
};

typedef SysError<> SystemError;

#endif // SYSERROR_HPP__
