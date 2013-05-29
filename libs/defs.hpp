#ifndef GOGO_DEFS_HPP__
#define GOGO_DEFS_HPP__

#define VSIZE(a) ( sizeof(a) / sizeof(a[0]) )

/* gcc-specific */
#define __UNUSED(x) __attribute__ ((unused)) x
#define __PACKED __attribute__((__packed__))
// FIXME hardcoded size of memory cacheline - 64 bytes
#define __cacheline_aligned__ __attribute__ ((aligned (64)))

// '!!' are here to ensure that __builtin_expect's first param is either 1 or 0
#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect(!!(x),0)

#define _B2KB(x) ((x) >> 20)
#define _B2MB(x) ((x) >> 20)
#define _B2GB(x) ((x) >> 30)

#define _KB2B(x) ((x) << 20)
#define _MB2B(x) ((x) << 20)
#define _GB2B(x) ((x) << 30)

/// @brief bicycle
template <typename T>
class auto_ptr_arr {
  T* p;
  public:
    auto_ptr_arr(T *_p) { p = _p; }
    ~auto_ptr_arr() { delete[] p; }
    T* get() { return p; }
};

#endif // GOGO_DEFS_HPP__
