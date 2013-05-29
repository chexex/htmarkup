//------------------------------------------------------------
/// @file  hash_array.hpp
/// @brief Hash array implemntation: indexer and searcher
/// @author Kisel Jan, <kisel@corp.mail.ru>
/// @date   05.05.2009
//------------------------------------------------------------

#ifndef GOGO_HASH_ARRAY_HPP__
#define GOGO_HASH_ARRAY_HPP__

#include <unistd.h> // for sysconf
#include <stdint.h>
#include <cassert>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "memio.hpp"
#include "defs.hpp"
#include <stdio.h>

#ifdef HASH_ARRAY_DEBUG
#include <stdio.h>
#  define HA_DBG(x) {x;}
#else
#  define HA_DBG(x) {}
#endif

/*
 EXAMPLE OF USAGE:
 
  HashArrayIndexer<uint64_t, uint32_t> ha (1280);

  ha.add (2, 2);
  ha.add (2, 4);
  ha.add (2, 8);
  ha.add (2, 16);
  ha.add (3, 3);
  ha.add (3, 9);
  ha.add (3, 27);
  ha.add (5, 555);
  ha.add (7, 2178);

  size_t memsz = ha.size();
  char *data = new char[memsz];
  MemWriter mwr (data);

  ha.index();
  ha.save (mwr);

  MemReader mrd (data);
  HashArraySearcher<uint64_t, uint32_t> srch;
  srch.load (mrd);

  uint32_t val;
  assert (srch.search (5, val));
  assert (val == 555);

  assert (srch.search (7, val));
  assert (val == 2178);

  assert (srch.search (2, val));
  assert (srch.search (3, val));

  assert (!srch.search (0, val));
  assert (!srch.search (321, val));
*/

namespace gogo
{
  
template<typename Tkey, typename Tval>
struct hash_entry {
  Tkey key;
  Tval value;

  bool operator < (const struct hash_entry &e) const { return (key < e.key); }
} __PACKED;


template<typename Tkey, typename Tval, typename hash_entry_t = hash_entry<Tkey, Tval> >
class HashArrayIndexer : public QSerializerOut
{
    typedef std::vector<hash_entry_t> bucket_t;
    std::vector<bucket_t> buckets;
    unsigned hash_value;
    unsigned m_nentries;
    bool     m_dirty;
    
  protected:
    void index() {
      typename std::vector<bucket_t>::iterator it;

      for (it = buckets.begin(); it != buckets.end(); it++) {
        if (!it->empty())
          std::stable_sort (it->begin(), it->end());
      }
    }

  public:
    HashArrayIndexer () { clear(); m_dirty = false; };
    HashArrayIndexer (unsigned n) { clear(); init(n); }
    
    void init(unsigned n) 
    {
      unsigned vpageSize = (unsigned) sysconf (_SC_PAGESIZE);
      size_t   sz = sizeof (hash_entry_t) * n;
      
      clear();

      unsigned ratio = (sz > vpageSize) ? sz / vpageSize : 1;
      unsigned hash_size;

      for (hash_size = 0; ratio > 0; hash_size++) {
        ratio >>= 1;
      }

      hash_size = (1 << hash_size);
      hash_value = hash_size - 1;
      buckets.resize (hash_size);

      //printf ("[pagesize=%d] n=%d; sz=%d; hash_value=%02X\n", vpageSize, n, (unsigned) sz, hash_value);
      m_dirty = true;
    }
    void clear() { buckets.clear(); m_nentries = 0; m_dirty = true; }
    
    virtual ~HashArrayIndexer() {};
    void add (Tkey k, Tval v) {
      unsigned ibucket = k & hash_value;

      hash_entry_t e;
      e.key = k;
      e.value = v;

      buckets[ibucket].push_back (e);
      m_nentries++;
      m_dirty = true;
    }
    
    // export facilities

    // store format is following:
    // [NBUCKETS: 4][BUCKETS_SIZES: x4][ENTRIES]
    size_t size() const {
      return (sizeof (uint32_t) + 
          buckets.size()*sizeof (uint32_t) + 
          m_nentries*sizeof (hash_entry_t)
      );
    }

    virtual void save (MemWriter &wr) 
    {
      if (m_dirty) { 
        index();
        m_dirty = false;
      }
      
      wr << (uint32_t) buckets.size();
      typename std::vector<bucket_t>::const_iterator it;
      for (it = buckets.begin(); it != buckets.end(); it++) {
        wr << (uint32_t) it->size();
        HA_DBG (printf ("BUCKET-SIZE: %d\n", (unsigned) it->size()));
      }

      for (it = buckets.begin(); it != buckets.end(); it++) {

        HA_DBG (printf ("+BUCKET: "));
        for (typename bucket_t::const_iterator b_it = it->begin(); b_it != it->end(); b_it++) {
          wr << *b_it;
          HA_DBG (printf ("(%d, %d)", (unsigned) b_it->key, (unsigned) b_it->value));
        }
        HA_DBG (printf ("\n"));
      }
    }
};


template<typename Tkey, typename Tval, typename hash_entry_t = hash_entry<Tkey, Tval> >
class HashArraySearcher : public QSerializerIn
{
  public:
    const hash_entry_t *m_pentries;
    unsigned hash_value;

  private:
    class BucketIndex
    {
        struct elem {
          unsigned offset;
          uint32_t n;
        };
        std::vector<elem> idx;

      public:

        // build bucket index: compute `offset' field
        void load (MemReader &rdr) {
          unsigned offset = 0;
          uint32_t n;
          elem e;

          rdr >> n;
          idx.reserve (n);
          while (n--) {
            e.offset = offset;
            rdr >> e.n;
            offset += e.n;
            idx.push_back (e);
          }
        }

        unsigned amount() const { 
          return (idx.empty()) ? 0 : idx.back().offset + idx.back().n; 
        }
        unsigned size() const { return idx.size(); }
        unsigned get (unsigned i, unsigned &offset) const {
          offset = idx[i].offset;
          return idx[i].n;
        }
    };

    BucketIndex bucketIdx;

  public:
    // import facility
    void load (MemReader &rdr) {
      bucketIdx.load (rdr);
      m_pentries = reinterpret_cast<const hash_entry_t *> (rdr.get());
      hash_value = bucketIdx.size() - 1;
      rdr.advance (bucketIdx.amount() * sizeof (hash_entry_t));
    }
    HashArraySearcher() : m_pentries(NULL) {}
    virtual ~HashArraySearcher() {};
      

    // searh key bucklet, then the value itself by bsearch
    bool search (Tkey k, Tval &v) const 
    {
      if (!m_pentries)
        return false;
      
      unsigned l, i;
      unsigned u = bucketIdx.get (k & hash_value, l); // = n elements in bucket

      u += l;
      while (l < u) {
        i = (l + u) / 2;
        if (m_pentries[i].key > k)
          u = i;
        else if (m_pentries[i].key < k)
          l = i + 1;
        else {
          v = m_pentries[i].value;
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    /// @return index of element with key @arg k, or ~0U if not found
    unsigned takeIndex(Tkey k) const
    {
      if (!m_pentries)
        return ~0U;
      
      unsigned l, i;
      unsigned u = bucketIdx.get (k & hash_value, l); // = n elements in bucket

      u += l;
      while (l < u) {
        i = (l + u) / 2;
        if (m_pentries[i].key > k)
          u = i;
        else if (m_pentries[i].key < k)
          l = i + 1;
        else {
          return i;
        }
      }

      return ~0U;
    }
    //-------------------------------------------------------------------------

    //-------------------------------------------------------
    /// @return reference to element at index @arg i
    /// @warning throw range_error if index out of range
    const Tval& at(unsigned i) const {
      if (i >= bucketIdx.amount())
        throw std::out_of_range("hash_array: requested index is out of range");

      return m_pentries[i].value;
    }
    //-------------------------------------------------------
    
    // searh key bucklet, then the values itself by bsearch
    unsigned search(Tkey k, std::vector<Tval> &vec) const 
    {
      if (!m_pentries)
        return 0U;
      
      unsigned l, i;
      unsigned n = bucketIdx.get (k & hash_value, l);
      unsigned u = l + n;
      
      unsigned _u = u;
      
      while (l < u) 
      {
        i = (l + u) / 2;
        if (m_pentries[i].key > k)
          u = i;
        else if (m_pentries[i].key < k)
          l = i + 1;
        else {
          // look for bounds of equal range
          for (l = i; l > 0 && m_pentries[l - 1].key == k; l--)
            ;
          for (u = i+1; u < _u && m_pentries[u].key == k; u++)
            ;
          vec.clear();
          for (; l < u; l++)
            vec.push_back(m_pentries[l].value);
          return vec.size();
        }
      }
      
      return 0;
    }
};

} // namespace gogo

#undef HA_DBG

#endif // GOGO_HASH_ARRAY_HPP__
