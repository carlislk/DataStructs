#ifndef HASH_SET_HPP_
#define HASH_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "pair.hpp"
#include "iterator.hpp"
#include "set.hpp"


namespace ics {

template<class T> class HashSet : public Set<T>	{
  public:
    HashSet() = delete;
    HashSet(int (*ahash)(const T& element), double the_load_factor = 1.0);
    HashSet(int initial_bins, int (*ahash)(const T& element), double the_load_factor = 1.0);
    HashSet(const HashSet<T>& to_copy);
    HashSet(std::initializer_list<T> il, int (*ahash)(const T& element), double the_load_factor = 1.0);
    HashSet(ics::Iterator<T>& start, const ics::Iterator<T>& stop, int (*ahash)(const T& element), double the_load_factor = 1.0);
    virtual ~HashSet();

    virtual bool empty      () const;
    virtual int  size       () const;
    virtual bool contains   (const T& element) const;
    virtual std::string str () const;

    virtual bool contains (ics::Iterator<T>& start, const ics::Iterator<T>& stop) const;

    virtual int  insert (const T& element);
    virtual int  erase  (const T& element);
    virtual void clear  ();

    virtual int insert (ics::Iterator<T>& start, const ics::Iterator<T>& stop);
    virtual int erase  (ics::Iterator<T>& start, const ics::Iterator<T>& stop);
    virtual int retain (ics::Iterator<T>& start, const ics::Iterator<T>& stop);

    virtual HashSet<T>& operator = (const HashSet<T>& rhs);
    virtual bool operator == (const Set<T>& rhs) const;
    virtual bool operator != (const Set<T>& rhs) const;
    virtual bool operator <= (const Set<T>& rhs) const;
    virtual bool operator <  (const Set<T>& rhs) const;
    virtual bool operator >= (const Set<T>& rhs) const;
    virtual bool operator >  (const Set<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const HashSet<T2>& s);

    virtual ics::Iterator<T>& ibegin () const;
    virtual ics::Iterator<T>& iend   () const;

  private:
    class LN;

  public:
    class Iterator : public ics::Iterator<T> {
      public:
        //KLUDGE should be callable only in begin/end
        Iterator(HashSet<T>* iterate_over, bool begin);
        Iterator(const Iterator& i);
        virtual ~Iterator();
        virtual T           erase();
        virtual std::string str  () const;
        virtual const ics::Iterator<T>& operator ++ ();
        virtual const ics::Iterator<T>& operator ++ (int);
        virtual bool operator == (const ics::Iterator<T>& rhs) const;
        virtual bool operator != (const ics::Iterator<T>& rhs) const;
        virtual T& operator *  () const;
        virtual T* operator -> () const;
      private:
        ics::pair<int,LN*> current; //Bin Index and Cursor; stop: LN* == nullptr
        HashSet<T>*        ref_set;
        int                expected_mod_count;
        bool               can_erase = true;
        void advance_cursors();
    };

    virtual Iterator begin () const;
    virtual Iterator end   () const;

  private:
    class LN {
      public:
        LN ()                      : next(nullptr){}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next;
    };

    LN** set      = nullptr;
    int (*hash)(const T& element);
    double load_factor;//used/bins <= load_factor
    int bins      = 1; //# bins in array
    int used      = 0; //# of key->value pairs in the hash table
    int mod_count = 0; //For sensing concurrent modification
    int   hash_compress (const T& element) const;
    void  ensure_load_factor(int new_used);
    LN*   find_element (int bin, const T& element) const;
    LN*   copy_list(LN*   l) const;
    LN**  copy_hash_table(LN** ht, int bins) const;
    void  delete_hash_table(LN**& ht, int bins);
  };





template<class T>
HashSet<T>::HashSet(int (*ahash)(const T& element), double the_load_factor)
    : hash(ahash), load_factor(the_load_factor) {
  set = new LN*[bins];
  for (int b=0; b<bins; ++b)
    set[b] = new LN();
}

template<class T>
HashSet<T>::HashSet(int initial_bins, int (*ahash)(const T& element), double the_load_factor)
    : bins(initial_bins), hash(ahash), load_factor(the_load_factor) {
  set = new LN*[bins];
  for (int b=0; b<bins; ++b)
    set[b] = new LN();
}

template<class T>
HashSet<T>::HashSet(const HashSet<T>& to_copy)
    : hash(to_copy.hash), load_factor(to_copy.load_factor), bins(to_copy.bins), used(to_copy.used) {
  set  = copy_hash_table(to_copy.set,bins);
}

template<class T>
HashSet<T>::HashSet(ics::Iterator<T>& start, const ics::Iterator<T>& stop, int (*ahash)(const T& element), double the_load_factor)
    : hash(ahash), load_factor(the_load_factor) {
  set = new LN*[bins];
  for (int b=0; b<bins; ++b)
    set[b] = new LN();
  insert(start,stop);
}

template<class T>
HashSet<T>::HashSet(std::initializer_list<T> il,int (*ahash)(const T& k), double the_load_factor)
    : hash(ahash), load_factor(the_load_factor) {
  set = new LN*[bins];
  for (int b=0; b<bins; ++b)
    set[b] = new LN();
  for (T s_elem : il)
    insert(s_elem);
}

template<class T>
HashSet<T>::~HashSet() {
  delete_hash_table(set,bins);
}


template<class T>
inline bool HashSet<T>::empty() const {
  return used == 0;
}

template<class T>
int HashSet<T>::size() const {
  return used;
}

template<class T>
bool HashSet<T>::contains (const T& element) const {
  return find_element(hash_compress(element),element) != nullptr;
}

template<class T>
std::string HashSet<T>::str() const {
  std::ostringstream answer;
  if (bins == 0)
    answer << "empty";
  else
    answer << std::endl;
    for (int b=0; b<bins; ++b) {
      answer << "bin[" << b << "] = ";
      for (LN* c = set[b]; c->next!=nullptr; c=c->next)
        answer << c->value << " -> " ;
      answer << "#" << std::endl;
    }
  answer  << "(load_factor=" << load_factor << ",bins=" << bins << ",used=" <<used <<",mod_count=" << mod_count << ")";
  return answer.str();
}

template<class T>
bool HashSet<T>::contains(ics::Iterator<T>& start, const ics::Iterator<T>& stop) const {
  for (; start != stop; ++start)
    if (!contains(*start))
      return false;

  return true;
}

template<class T>
int HashSet<T>::insert(const T& element) {
  int bin = hash_compress(element);
  LN* c = find_element(bin,element);
  if (c != nullptr)
      return 0;

  ensure_load_factor(used+1);
  bin = hash_compress(element);  //bins may have changed
  set[bin] = new LN(element,set[bin]);
  ++used;
  ++mod_count;
  return 1;
}

template<class T>
int HashSet<T>::erase(const T& element) {
  LN* c = find_element(hash_compress(element),element);
  if (c == nullptr)
    return 0;

  LN* to_delete = c->next;
  *c = *(c->next);
  delete to_delete;
  --used;
  ++mod_count;
  return 1;
}

template<class T>
void HashSet<T>::clear() {
  for (int b=0; b<bins; ++b) {
    LN* l=set[b];
    for (; l->next!=nullptr; /*See body*/) {
      LN* to_delete = l;
      l = l->next;
      delete to_delete;
    }
    set[b] = l;
  }

  used = 0;
  ++mod_count;
}

template<class T>
int HashSet<T>::insert(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; ++start)
    count += insert(*start);

  return count;
}

template<class T>
int HashSet<T>::erase(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; ++start)
    count += erase(*start);
  return count;
}

template<class T>
int HashSet<T>::retain(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  HashSet<T> s(start,stop,hash);
  int count = 0;
  for (int b=0; b<bins; ++b)
    for (LN* c=set[b]; c->next!=nullptr; /*See body*/) {
      if (s.contains(c->value))
        c = c-> next;
      else{
        LN* to_delete = c->next;
        *c = *(c->next);
        delete to_delete;
        ++count;
      }
    }

  used -= count;
  return count;
}

template<class T>
HashSet<T>& HashSet<T>::operator = (const HashSet<T>& rhs) {
  if (this == &rhs)
    return *this;

  delete_hash_table(set,bins);
  set         = copy_hash_table(rhs.set,rhs.bins);
  hash        = rhs.hash;
  load_factor = rhs.load_factor;
  bins        = rhs.bins;
  used        = rhs.used;

  ++mod_count;
  return *this;
}

template<class T>
bool HashSet<T>::operator == (const Set<T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used != rhs.size())
    return false;

  for (int b=0; b<bins; ++b)
    for (LN* c=set[b]; c->next!=nullptr; c=c->next)
       if (!rhs.contains(c->value))
         return false;

  return true;
}

template<class T>
bool HashSet<T>::operator != (const Set<T>& rhs) const {
  return !(*this == rhs);
}

template<class T>
bool HashSet<T>::operator <= (const Set<T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used > rhs.size())
    return false;
  for (int b=0; b<bins; ++b)
    for (LN* c=set[b]; c->next!=nullptr; c=c->next)
      if (!rhs.contains(c->value))
        return false;

  return true;
}

template<class T>
bool HashSet<T>::operator < (const Set<T>& rhs) const {
  if (this == &rhs)
    return false;
  if (used >= rhs.size())
    return false;
  for (int b=0; b<bins; ++b)
    for (LN* c=set[b]; c->next!=nullptr; c=c->next)
      if (!rhs.contains(c->value))
        return false;

  return true;
}

template<class T>
bool HashSet<T>::operator >= (const Set<T>& rhs) const {
  return rhs <= *this;
}

template<class T>
bool HashSet<T>::operator > (const Set<T>& rhs) const {
  return rhs < *this;
}

template<class T>
std::ostream& operator << (std::ostream& outs, const HashSet<T>& s) {
  if (s.empty()) {
    outs << "set[]";
  }else{
    outs << "set[";
    ics::Iterator<T>& i = s.ibegin();
    outs << *i;
    ++i;
    for (/*See above*/; i != s.iend(); ++i)
      outs << "," << *i;
    outs << "]";
  }
  return outs;
}

//KLUDGE: memory-leak
template<class T>
auto HashSet<T>::ibegin () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<HashSet<T>*>(this),true));
}

//KLUDGE: memory-leak
template<class T>
auto HashSet<T>::iend () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<HashSet<T>*>(this),false));
}

template<class T>
auto HashSet<T>::begin () const -> HashSet<T>::Iterator {
  return Iterator(const_cast<HashSet<T>*>(this),true);
}

template<class T>
auto HashSet<T>::end () const -> HashSet<T>::Iterator {
  return Iterator(const_cast<HashSet<T>*>(this),false);
}

template<class T>
int HashSet<T>::hash_compress (const T& element) const {
  return abs(hash(element)) % bins;
}

template<class T>
void HashSet<T>::ensure_load_factor(int new_used) {
  if (double(new_used)/double(bins) <= load_factor)
    return;

  LN** old_set  = set;
  int  old_bins = bins;

  bins = 2*old_bins;
  set = new LN*[bins];

  for (int b=0; b<bins; ++b)
    set[b] = new LN();

  for (int b=0; b<old_bins; ++b) {
    LN* c=old_set[b];
    for (; c->next!=nullptr; /*See body*/) {
      int bin = hash_compress(c->value);
      LN* to_move = c;
      c = c->next;
      to_move->next = set[bin];
      set[bin] = to_move;
    }
    delete c;
  }
  delete [] old_set;
}

template<class T>
typename HashSet<T>::LN* HashSet<T>::find_element (int bin, const T& element) const {
  for (LN* c = set[bin]; c->next!=nullptr; c=c->next)
    if (element == c->value)
      return c;

  return nullptr;
}

template<class T>
typename HashSet<T>::LN* HashSet<T>::copy_list (LN* l) const {
  if (l == nullptr)
    return nullptr;
  else
    return new LN(l->value, copy_list(l->next));
}

template<class T>
typename HashSet<T>::LN** HashSet<T>::copy_hash_table (LN** ht, int bins) const {
  LN** answer = new LN*[bins];
  for (int b=0; b<bins; ++b)
     answer[b] = copy_list(ht[b]);
  return answer;
}

template<class T>
void HashSet<T>::delete_hash_table (LN**& ht, int bins) {
  for (int b=0; b<bins; ++b)
    for (LN* c=ht[b]; c!=nullptr; /*See body*/) {
      LN* to_delete = c;
      c = c->next;
      delete to_delete;
  }
  delete[] ht;
  ht = nullptr;
}


template<class T>
void HashSet<T>::Iterator::advance_cursors(){
  if (current.second != nullptr && current.second->next != nullptr && current.second->next->next != nullptr) {
    current.second = current.second->next;
    return;
  }else
    for (int b=current.first+1; b<ref_set->bins; ++b)
      if (ref_set->set[b]->next != nullptr) {
        current.first  = b;
        current.second = ref_set->set[b];
        return;
      }
  current.first  = -1;
  current.second = nullptr;
}




template<class T>
HashSet<T>::Iterator::Iterator(HashSet<T>* iterate_over, bool begin) : ref_set(iterate_over) {
  current = ics::pair<int,LN*>(-1,nullptr);
  if (begin)
     advance_cursors();
  expected_mod_count = ref_set->mod_count;
}


template<class T>
HashSet<T>::Iterator::Iterator(const Iterator& i) :
    current(i.current), ref_set(i.ref_set), expected_mod_count(i.expected_mod_count), can_erase(i.can_erase) {}

template<class T>
HashSet<T>::Iterator::~Iterator()
{}

template<class T>
T HashSet<T>::Iterator::erase() {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("HashSet::Iterator::erase Iterator cursor already erased");
  if (current.second == nullptr)
    throw CannotEraseError("HashSet::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  T to_return = current.second->value;
  LN* to_delete = current.second->next;

  *current.second = *(current.second->next);
  --ref_set->used;
  ++ref_set->mod_count;
  expected_mod_count = ref_set->mod_count;
  delete to_delete;

  return to_return;
}

template<class T>
std::string HashSet<T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_set->str() << "(current=" << current.first << "/" << current.second << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}

template<class T>
const ics::Iterator<T>& HashSet<T>::Iterator::operator ++ () {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator ++");

  if (current.second == nullptr)
    return *this;

  if (can_erase || current.second->next == nullptr)
    advance_cursors();

  can_erase = true;
  return *this;
}

//KLUDGE: creates garbage! (can return local value!)
template<class T>
const ics::Iterator<T>& HashSet<T>::Iterator::operator ++ (int) {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator ++(int)");

  Iterator* to_return = new Iterator(*this);
  if (current.second == nullptr)
    return *to_return;

  if (can_erase || current.second->next == nullptr)
    advance_cursors();

  can_erase = true;
  return *to_return;
}

template<class T>
bool HashSet<T>::Iterator::operator == (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("HashSet::Iterator::operator ==");
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator ==");
  if (ref_set != rhsASI->ref_set)
    throw ComparingDifferentIteratorsError("HashSet::Iterator::operator ==");

  return this->current.second == rhsASI->current.second;
}


template<class T>
bool HashSet<T>::Iterator::operator != (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("HashSet::Iterator::operator !=");
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator !=");
  if (ref_set != rhsASI->ref_set)
    throw ComparingDifferentIteratorsError("HashSet::Iterator::operator !=");

  return this->current.second != rhsASI->current.second;
}

template<class T>
T& HashSet<T>::Iterator::operator *() const {
  if (expected_mod_count !=
      ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator *");
  if (!can_erase || current.second == nullptr)
    throw IteratorPositionIllegal("HashSet::Iterator::operator * Iterator illegal: exhausted");

  return current.second->value;
}

template<class T>
T* HashSet<T>::Iterator::operator ->() const {
  if (expected_mod_count !=
      ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator *");
  if (!can_erase || current.second == nullptr)
    throw IteratorPositionIllegal("HashSet::Iterator::operator * Iterator illegal: exhausted");

  return &(current.second->value);
}

}

#endif /* HASH_SET_HPP_ */
