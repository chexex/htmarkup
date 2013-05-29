#ifndef __HASHES_HPP
#define __HASHES_HPP

#include <string>
#include <cstring>
#include <stdint.h>

inline int64_t hash_64( const char *szKey, int len = -1, const int64_t iM = 0x7FFFFFFFFFFFFFFFll )
{
    if ( len == -1 )
        len = strlen( szKey );

    int64_t iH = 0, iA = 31415, iB = 27183;
    for ( ; len >= 0; szKey++, len--, iA = iA * iB % ( iM - 1 ) ) { iH = ( iA * iH + *szKey ) * iM; }
    return ( iH < 0 ) ? ( iH + iM ) : iH;
}

template < typename CT, typename Alloc >
int64_t hash_64( const std::basic_string<char, CT, Alloc> &sKey, const int64_t iM = 0x7FFFFFFFFFFFFFFFll )
{
  return hash_64( sKey.c_str(), sKey.size(), iM );
}

/////////////////////////////////////////////////////////////////////////////////////////////
// very fast and strong enought hash taken from
// http://murmurhash.googlepages.com/

typedef uint32_t Hash32;
typedef uint64_t Hash64;

Hash32 MurmurHash32 ( const void * key, int len, uint32_t seed = 0);
Hash64 MurmurHash64 ( const void * key, int len, uint64_t seed = 0);

static inline void MurmurHash(const void *s, size_t len, uint32_t *res, uint32_t seed = 0) {
  *res = MurmurHash32(s, len, seed);
}

static inline void MurmurHash(const void *s, size_t len, uint64_t *res, uint64_t seed = 0) {
  *res = MurmurHash64(s, len, seed);
}

static inline void MurmurHash( const std::string &s, uint32_t *res, uint32_t seed = 0) {
  *res = MurmurHash32(s.data(), s.length(), seed);
}

static inline void MurmurHash( const std::string &s, uint64_t *res, uint64_t seed = 0) {
  *res = MurmurHash64(s.data(), s.length(), seed);
}

template<typename HashT>
struct MMHash
{
  static inline HashT make(const void *ptr, size_t len, HashT seed = static_cast<HashT>(0)) {
    HashT res;
    MurmurHash(ptr, len, &res, seed);
    return res;
  }
  static inline HashT make(const char *s) {
    return MMHash<HashT>::make(static_cast<const void*>(s), strlen(s));
  }
  static inline HashT make(const std::string &s) {
    return MMHash<HashT>::make(s.c_str(), s.length());
  }
};

template<typename THash, class TData>
struct MMHashAccumulator
{
  THash hash;
  
  MMHashAccumulator() : hash(static_cast<THash>(0)) {}
  MMHashAccumulator<THash, TData>& operator+=(const TData &data) {
    hash = MMHash<THash>::make(&data, sizeof(data), hash);
    return *this;
  }
  operator THash() { return hash; }
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif
