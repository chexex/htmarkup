#ifndef GOGO_FILEUTILS_HPP__
#define GOGO_FILEUTILS_HPP__

#include <string>
#include "config.h"

bool file_exist(const char *pathname);
static inline bool file_exist(std::string s) { return file_exist(s.c_str()); }

bool Write(int fd, const void *data, size_t len);
bool Read(int fd, void *data, size_t len);

off_t file_size(int fd);
off_t file_size(const char *path);

#if HAVE_PREAD
bool PRead(int fd, void *buf, size_t len, off_t offset);
#endif

#endif // GOGO_FILEUTILS_HPP__
