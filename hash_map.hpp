#ifndef HASH_MAP_HPP_
#define HASH_MAP_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "pair.hpp"
#include "map.hpp"
#include "array_queue.hpp"   //For traversal


namespace ics {

template<class KEY,class T> class HashMap : public Map<KEY,T>	{
  public:
    typedef ics::pair<KEY,T> Entry;
    HashMap() = delete;
    HashMap(int (*ahash)(const KEY& k), double the_load_factor = 1.0);
    HashMap(int initial_bins, int (*ahash)(const KEY& k), double the_load_factor = 1.0);
	  HashMap(const HashMap<KEY,T>& to_copy);
	  HashMap(std::initializer_list<Entry> il, int (*ahash)(const KEY& k), double the_load_factor = 1.0);
    HashMap(ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop, int (*ahash)(const KEY& k), double the_load_factor = 1.0);
	  virtual ~HashMap();

    virtual bool empty      () const;
    virtual int  size       () const;
    virtual bool has_key    (const KEY& key) const;
    virtual bool has_value  (const T& value) const;
    virtual std::string str () const;

    virtual T    put   (const KEY& key, const T& value);
    virtual T    erase (const KEY& key);
    virtual void clear ();

    virtual int put   (ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop);

    virtual T&       operator [] (const KEY&);
    virtual const T& operator [] (const KEY&) const;
    virtual HashMap<KEY,T>& operator = (const HashMap<KEY,T>& rhs);
    virtual bool operator == (const Map<KEY,T>& rhs) const;
    virtual bool operator != (const Map<KEY,T>& rhs) const;

    template<class KEY2,class T2>
    friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY2,T2>& m);

    virtual ics::Iterator<Entry>& ibegin () const;
    virtual ics::Iterator<Entry>& iend   () const;

   private:
     class LN;

   public:
     class Iterator : public ics::Iterator<Entry> {
       public:
        //KLUDGE should be callable only in begin/end
        Iterator(HashMap<KEY,T>* iterate_over, bool begin);
        Iterator(const Iterator& i);
        virtual ~Iterator();
        virtual Entry       erase();
        virtual std::string str  () const;
        virtual const ics::Iterator<Entry>& operator ++ ();
        virtual const ics::Iterator<Entry>& operator ++ (int);
        virtual bool operator == (const ics::Iterator<Entry>& rhs) const;
        virtual bool operator != (const ics::Iterator<Entry>& rhs) const;
        virtual Entry& operator *  () const;
        virtual Entry* operator -> () const;
      private:
        ics::pair<int,LN*> current; //Bin Index and Cursor; stop: LN* == nullptr
        HashMap<KEY,T>*    ref_map;
        int                expected_mod_count;
        bool               can_erase = true;
        void advance_cursors();
    };

    virtual Iterator begin () const;
    virtual Iterator end   () const;
    //KLUDGE: define
    //virtual ics::Iterator<KEY>&  begin_key   () const;
    //virtual ics::Iterator<KEY>&  end_key     () const;
    //virtual ics::Iterator<T>&    begin_value () const;
    //virtual ics::Iterator<T>&    end_value   () const;

    private:
      class LN {
        public:
          LN ()                         : next(nullptr){}
          LN (const LN& ln)             : value(ln.value), next(ln.next){}
          LN (Entry v, LN* n = nullptr) : value(v), next(n){}

          Entry value;
          LN*   next;
      };

      LN** map      = nullptr;
      int (*hash)(const KEY& k);
      double load_factor;//used/bins <= load_factor
      int bins      = 1; //# bins in array
      int used      = 0; //# of key->value pairs in the hash table
      int mod_count = 0; //For sensing concurrent modification
      int   hash_compress (const KEY& key) const;
      void  ensure_load_factor(int new_used);
      LN*   find_key (int bin, const KEY& key) const;
      bool  find_value (const T& value) const;
      LN*   copy_list(LN*   l) const;
      LN**  copy_hash_table(LN** ht, int bins) const;
      void  delete_hash_table(LN**& ht, int bins);
  };





template<class KEY,class T>
HashMap<KEY,T>::HashMap(int (*ahash)(const KEY& k), double the_load_factor)
    : hash(ahash), load_factor(the_load_factor) {
  map = new LN*[bins];
  for (int b=0; b<bins; ++b)
    map[b] = new LN();
}

template<class KEY,class T>
HashMap<KEY,T>::HashMap(int initial_bins, int (*ahash)(const KEY& k), double the_load_factor)
    : bins(initial_bins), hash(ahash), load_factor(the_load_factor) {
  map = new LN*[bins];
  for (int b=0; b<bins; ++b)
    map[b] = new LN();
}

template<class KEY,class T>
HashMap<KEY,T>::HashMap(const HashMap<KEY,T>& to_copy)
    : hash(to_copy.hash), load_factor(to_copy.load_factor), bins(to_copy.bins), used(to_copy.used) {
  map  = copy_hash_table(to_copy.map,bins);
}

template<class KEY,class T>
HashMap<KEY,T>::HashMap(ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop, int (*ahash)(const KEY& k), double the_load_factor)
    : hash(ahash), load_factor(the_load_factor) {
  map = new LN*[bins];
  for (int b=0; b<bins; ++b)
    map[b] = new LN();
  put(start,stop);
}

template<class KEY,class T>
HashMap<KEY,T>::HashMap(std::initializer_list<Entry> il,int (*ahash)(const KEY& k), double the_load_factor)
    : hash(ahash), load_factor(the_load_factor) {
  map = new LN*[bins];
  for (int b=0; b<bins; ++b)
    map[b] = new LN();
  for (Entry m_entry : il)
    put(m_entry.first,m_entry.second);
}

template<class KEY,class T>
HashMap<KEY,T>::~HashMap() {
  delete_hash_table(map,bins);
}


template<class KEY,class T>
inline bool HashMap<KEY,T>::empty() const {
  return used == 0;
}

template<class KEY,class T>
int HashMap<KEY,T>::size() const {
  return used;
}

template<class KEY,class T>
bool HashMap<KEY,T>::has_key (const KEY& key) const {
  return find_key(hash_compress(key),key) != nullptr;
}

template<class KEY,class T>
bool HashMap<KEY,T>::has_value (const T& value) const {
  return find_value(value);
}

template<class KEY,class T>
std::string HashMap<KEY,T>::str() const {
  std::ostringstream answer;
  if (bins == 0)
    answer << "empty";
  else
    answer << std::endl;
    for (int b=0; b<bins; ++b) {
      answer << "bin[" << b << "] = ";
      for (LN* c = map[b]; c->next!=nullptr; c=c->next)
        answer << c->value << " -> " ;
      answer << "#" << std::endl;
    }
  answer  << "(load_factor=" << load_factor << ",bins=" << bins << ",used=" <<used <<",mod_count=" << mod_count << ")";
  return answer.str();
}

template<class KEY,class T>
T HashMap<KEY,T>::put(const KEY& key, const T& value) {
  int bin = hash_compress(key);
  T to_return;
  LN* c = find_key(bin,key);
  if (c != nullptr) {
    to_return = c->value.second;
    c->value.second = value;
  }else{
    to_return = value;
    ensure_load_factor(used+1);
    ++used;
    bin = hash_compress(key);  //bins may have changed
    map[bin] = new LN(ics::make_pair(key,value),map[bin]);
  }
  ++mod_count;
  return to_return;
}

template<class KEY,class T>
T HashMap<KEY,T>::erase(const KEY& key) {
  LN* c = find_key(hash_compress(key),key);
  if (c == nullptr) {
    std::ostringstream answer;
    answer << "HashMap::erase: key(" << key << ") not in Map";
    throw KeyError(answer.str());
  }
  T to_return = c->value.second;
  LN* to_delete = c->next;
  *c = *(c->next);
  delete to_delete;
  --used;
  ++mod_count;
  return to_return;
}

template<class KEY,class T>
void HashMap<KEY,T>::clear() {
  for (int b=0; b<bins; ++b) {
    LN* c=map[b];
    for (; c->next!=nullptr; /*See body*/) {
      LN* to_delete = c;
      c = c->next;
      delete to_delete;
    }
    map[b] = c;
  }

  used = 0;
  ++mod_count;
}

template<class KEY,class T>
int HashMap<KEY,T>::put (ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop) {
  int count = 0;
  for (; start != stop; ++start) {
    ++count;
    put(start->first,start->second);
  }

  return count;
}

template<class KEY,class T>
T& HashMap<KEY,T>::operator [] (const KEY& key) {
  int bin = hash_compress(key);
  LN* c = find_key(bin,key);
  if (c != nullptr) {
    return c->value.second;
  }

  ensure_load_factor(used+1);
  ++used;
  ++mod_count;
  bin = hash_compress(key);  //bins may have changed
  map[bin] = new LN(ics::make_pair(key,T()),map[bin]);
  return map[bin]->value.second;
}

template<class KEY,class T>
const T& HashMap<KEY,T>::operator [] (const KEY& key) const {
  int bin = hash_compress(key);
  LN* c = find_key(bin,key);
  if (c != nullptr)
    return c->value.second;

  std::ostringstream answer;
  answer << "HashMap::operator []: key(" << key << ") not in Map";
  throw KeyError(answer.str());
}

template<class KEY,class T>
bool HashMap<KEY,T>::operator == (const Map<KEY,T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used != rhs.size())
    return false;

  for (int b=0; b<bins; ++b)
    for (LN* c=map[b]; c->next!=nullptr; c=c->next)
       if (rhs.has_key(c->value.first) && c->value.second !=  rhs[c->value.first])
         return false;

  return true;
}

template<class KEY,class T>
HashMap<KEY,T>& HashMap<KEY,T>::operator = (const HashMap<KEY,T>& rhs) {
  if (this == &rhs)
    return *this;

  delete_hash_table(map,bins);
  map         = copy_hash_table(rhs.map,rhs.bins);
  hash        = rhs.hash;
  load_factor = rhs.load_factor;
  bins        = rhs.bins;
  used        = rhs.used;

  ++mod_count;
  return *this;
}

template<class KEY,class T>
bool HashMap<KEY,T>::operator != (const Map<KEY,T>& rhs) const {
  return !(*this == rhs);
}


template<class KEY,class T>
std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T>& m) {
  if (m.empty()) {
    outs << "map[]";
  }else{
    outs << "map[";
    ics::Iterator<ics::pair<KEY,T>>& i = m.ibegin();
    outs << i->first << "->" << i->second;
    ++i;
    for (/*See above*/; i != m.iend(); ++i)
      outs << "," << i->first << "->" << i->second;
    outs << "]";
  }
  return outs;
}

//KLUDGE: memory-leak
template<class KEY,class T>
auto HashMap<KEY,T>::ibegin () const -> ics::Iterator<Entry>& {
  return *(new Iterator(const_cast<HashMap<KEY,T>*>(this),true));
}

//KLUDGE: memory-leak
template<class KEY,class T>
auto HashMap<KEY,T>::iend () const -> ics::Iterator<Entry>& {
  return *(new Iterator(const_cast<HashMap<KEY,T>*>(this),false));
}

template<class KEY,class T>
auto HashMap<KEY,T>::begin () const -> HashMap<KEY,T>::Iterator {
  return Iterator(const_cast<HashMap<KEY,T>*>(this),true);
}

template<class KEY,class T>
auto HashMap<KEY,T>::end () const -> HashMap<KEY,T>::Iterator {
  return Iterator(const_cast<HashMap<KEY,T>*>(this),false);
}

template<class KEY,class T>
int HashMap<KEY,T>::hash_compress (const KEY& key) const {
  return abs(hash(key)) % bins;
}

template<class KEY,class T>
void HashMap<KEY,T>::ensure_load_factor(int new_used) {
  if (double(new_used)/double(bins) <= load_factor)
    return;

  LN** old_map  = map;
  int  old_bins = bins;

  bins = 2*old_bins;
  map = new LN*[bins];

  for (int b=0; b<bins; ++b)
    map[b] = new LN();

  for (int b=0; b<old_bins; ++b) {
    LN* c=old_map[b];
    for (; c->next!=nullptr; /*See body*/) {
      int bin = hash_compress(c->value.first);
      LN* to_move = c;
      c = c->next;
      to_move->next = map[bin];
      map[bin] = to_move;
    }
    delete c;
  }
  delete [] old_map;
}

template<class KEY,class T>
typename HashMap<KEY,T>::LN* HashMap<KEY,T>::find_key (int bin, const KEY& key) const {
  for (LN* c = map[bin]; c->next!=nullptr; c=c->next)
    if (key == c->value.first)
      return c;

  return nullptr;
}

template<class KEY,class T>
bool HashMap<KEY,T>::find_value (const T& value) const {
  for (int b=0; b<bins; ++b)
    for (LN* c = map[b]; c->next!=nullptr; c=c->next)
      if (value == c->value.second)
        return true;

  return false;
}

template<class KEY,class T>
typename HashMap<KEY,T>::LN* HashMap<KEY,T>::copy_list (LN* l) const {
  if (l == nullptr)
    return nullptr;
  else
    return new LN(l->value, copy_list(l->next));
}

template<class KEY,class T>
typename HashMap<KEY,T>::LN** HashMap<KEY,T>::copy_hash_table (LN** ht, int bins) const {
  LN** answer = new LN*[bins];
  for (int b=0; b<bins; ++b)
     answer[b] = copy_list(ht[b]);
  return answer;
}

template<class KEY,class T>
void HashMap<KEY,T>::delete_hash_table (LN**& ht, int bins) {
  for (int b=0; b<bins; ++b)
    for (LN* c=ht[b]; c!=nullptr; /*See body*/) {
      LN* to_delete = c;
      c = c->next;
      delete to_delete;
  }
  delete[] ht;
  ht = nullptr;
}


template<class KEY,class T>
void HashMap<KEY,T>::Iterator::advance_cursors(){
  if (current.second != nullptr && current.second->next != nullptr && current.second->next->next != nullptr) {
    current.second = current.second->next;
    return;
  }else
    for (int b=current.first+1; b<ref_map->bins; ++b)
      if (ref_map->map[b]->next != nullptr) {
        current.first  = b;
        current.second = ref_map->map[b];
        return;
      }
  current.first  = -1;
  current.second = nullptr;
}

template<class KEY,class T>
HashMap<KEY,T>::Iterator::Iterator(HashMap<KEY,T>* iterate_over, bool begin) : ref_map(iterate_over) {
  current = ics::pair<int,LN*>(-1,nullptr);
  if (begin)
     advance_cursors();
  expected_mod_count = ref_map->mod_count;
}

template<class KEY,class T>
HashMap<KEY,T>::Iterator::Iterator(const Iterator& i) :
    current(i.current), ref_map(i.ref_map), expected_mod_count(i.expected_mod_count), can_erase(i.can_erase) {}

template<class KEY,class T>
HashMap<KEY,T>::Iterator::~Iterator()
{}

template<class KEY,class T>
auto HashMap<KEY,T>::Iterator::erase() -> Entry {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("HashMap::Iterator::erase Iterator cursor already erased");
  if (current.second == nullptr)
    throw CannotEraseError("HashMap::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  Entry to_return = current.second->value;
  LN* to_delete = current.second->next;

  *current.second = *(current.second->next);
  --ref_map->used;
  ++ref_map->mod_count;
  expected_mod_count = ref_map->mod_count;
  delete to_delete;

  return to_return;
}

template<class KEY,class T>
std::string HashMap<KEY,T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_map->str() << "(current=" << current.first << "/" << current.second << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}

//KLUDGE: cannot use Entry
template<class KEY,class T>
auto  HashMap<KEY,T>::Iterator::operator ++ () -> const ics::Iterator<ics::pair<KEY,T>>& {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator ++");

  if (current.second == nullptr)
    return *this;

  if (can_erase || current.second->next == nullptr)
    advance_cursors();

  can_erase = true;
  return *this;
}

//KLUDGE: creates garbage! (can return local value!)
template<class KEY,class T>
auto HashMap<KEY,T>::Iterator::operator ++ (int) -> const ics::Iterator<ics::pair<KEY,T>>&{
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator ++(int)");

  Iterator* to_return = new Iterator(*this);
  if (current.second == nullptr)
    return *to_return;

  if (can_erase || current.second->next == nullptr)
    advance_cursors();

  can_erase = true;
  return *to_return;
}

template<class KEY,class T>
bool HashMap<KEY,T>::Iterator::operator == (const ics::Iterator<Entry>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("HashMap::Iterator::operator ==");
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator ==");
  if (ref_map != rhsASI->ref_map)
    throw ComparingDifferentIteratorsError("HashMap::Iterator::operator ==");

  return this->current.second == rhsASI->current.second;
}


template<class KEY,class T>
bool HashMap<KEY,T>::Iterator::operator != (const ics::Iterator<Entry>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("HashMap::Iterator::operator !=");
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator !=");
  if (ref_map != rhsASI->ref_map)
    throw ComparingDifferentIteratorsError("HashMap::Iterator::operator !=");

  return this->current.second != rhsASI->current.second;
}

template<class KEY,class T>
ics::pair<KEY,T>& HashMap<KEY,T>::Iterator::operator *() const {
  if (expected_mod_count !=
      ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator *");
  if (!can_erase || current.second == nullptr)
    throw IteratorPositionIllegal("HashMap::Iterator::operator * Iterator illegal: exhausted");

  return current.second->value;
}

template<class KEY,class T>
ics::pair<KEY,T>* HashMap<KEY,T>::Iterator::operator ->() const {
  if (expected_mod_count !=
      ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator *");
  if (!can_erase || current.second == nullptr)
    throw IteratorPositionIllegal("HashMap::Iterator::operator -> Iterator illegal: exhausted");

  return &(current.second->value);
}

}

#endif /* HASH_MAP_HPP_ */
