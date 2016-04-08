#ifndef ARRAY_MAP_HPP_
#define ARRAY_MAP_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "pair.hpp"
#include "map.hpp"


namespace ics {

template<class KEY,class T> class ArrayMap : public Map<KEY,T>	{
  public:
    typedef ics::pair<KEY,T> Entry;
	  ArrayMap();
	  explicit ArrayMap(int initialLength);
	  ArrayMap(const ArrayMap<KEY,T>& to_copy);
	  ArrayMap(std::initializer_list<Entry> il);
    ArrayMap(ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop);
	  virtual ~ArrayMap();

    virtual bool empty      () const;
    virtual int  size       () const;
    virtual bool has_key    (const KEY& key) const;
    virtual bool has_value  (const T& value) const;
    virtual std::string str () const;

    virtual T    put   (const KEY& key, const T& value);
    virtual T    erase (const KEY& key);
    virtual void clear ();

    virtual int put (ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop);

    virtual T&       operator [] (const KEY&);
    virtual const T& operator [] (const KEY&) const;
    virtual ArrayMap<KEY,T>& operator = (const ArrayMap<KEY,T>& rhs);
    virtual bool operator == (const Map<KEY,T>& rhs) const;
    virtual bool operator != (const Map<KEY,T>& rhs) const;

    template<class KEY2,class T2>
    friend std::ostream& operator << (std::ostream& outs, const ArrayMap<KEY2,T2>& m);

    class Iterator : public ics::Iterator<Entry> {
      public:
        //KLUDGE should be callable only in begin/end
        Iterator(ArrayMap<KEY,T>* iterate_over, int initial);
        virtual ~Iterator() {}
        virtual Entry       erase();
        virtual std::string str  () const;
        virtual const ics::Iterator<Entry>& operator ++ ();
        virtual const ics::Iterator<Entry>& operator ++ (int);
        virtual bool operator == (const ics::Iterator<Entry>& rhs) const;
        virtual bool operator != (const ics::Iterator<Entry>& rhs) const;
        virtual Entry& operator *  () const;
        virtual Entry* operator -> () const;
      private:
        int              current;  //if can_erase is false, this value is unusable
        ArrayMap<KEY,T>* ref_map;
        int              expected_mod_count;
        bool             can_erase = true;
    };

    //For explicit use: Iterator<...>& it = c.ibegin(); ... or for (Iterator<...>& it = c.ibegin(); it != c.iend(); ++it)...
    virtual ics::Iterator<Entry>& ibegin () const;
    virtual ics::Iterator<Entry>& iend   () const;

    //For implicit use: for (... i : c)...
    virtual Iterator begin () const;
    virtual Iterator end   () const;

    //KLUDGE: could define
    //virtual ics::Iterator<KEY>&  begin_key   () const;
    //virtual ics::Iterator<KEY>&  end_key     () const;
    //virtual ics::Iterator<T>&    begin_value () const;
    //virtual ics::Iterator<T>&    end_value   () const;

    private://KLUDGE: private: friend not found? now seems to work (but maybe haven't recompiled)
      Entry* map;
      int length    = 0; //Physical length of array
      int used      = 0; //Amount of array used
      int mod_count = 0; //For sensing concurrent modification
      int  index_of (const KEY& key) const;
      T    change_at(int i, const T& value);
      T    erase_at(int i);
      void ensure_length(int new_length);
  };





template<class KEY,class T>
ArrayMap<KEY,T>::ArrayMap() {
  map = new Entry[length];
}


template<class KEY,class T>
ArrayMap<KEY,T>::ArrayMap(int initial_length) : length(initial_length) {
  if (length < 0)
    length = 0;
  map = new Entry[length];
}


template<class KEY,class T>
ArrayMap<KEY,T>::ArrayMap(const ArrayMap<KEY,T>& to_copy) : length(to_copy.length), used(to_copy.used) {
  map = new Entry[length];
  for (int i=0; i<to_copy.used; ++i)
    map[i] = to_copy.map[i];
}


template<class KEY,class T>
ArrayMap<KEY,T>::ArrayMap(ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop) {
  map = new Entry[length];
  put(start,stop);
}


template<class KEY,class T>
ArrayMap<KEY,T>::ArrayMap(std::initializer_list<Entry> il) {
  map = new Entry[length];
  for (Entry m_entry : il)
    put(m_entry.first,m_entry.second);
}


template<class KEY,class T>
ArrayMap<KEY,T>::~ArrayMap() {
  delete[] map;
}



template<class KEY,class T>
inline bool ArrayMap<KEY,T>::empty() const {
  return used == 0;
}


template<class KEY,class T>
int ArrayMap<KEY,T>::size() const {
  return used;
}


template<class KEY,class T>
bool ArrayMap<KEY,T>::has_key (const KEY& element) const {
  for (int i=0; i<used; ++i)
    if (map[i].first == element)
      return true;

  return false;
}


template<class KEY,class T>
bool ArrayMap<KEY,T>::has_value (const T& element) const {
  for (int i=0; i<used; ++i)
    if (map[i].second == element)
      return true;

  return false;
}


template<class KEY,class T>
std::string ArrayMap<KEY,T>::str() const {
  std::ostringstream answer;
  answer << *this << "(length=" << length << ",used=" << used << ",mod_count=" << mod_count << ")";
  return answer.str();
}


template<class KEY,class T>
T ArrayMap<KEY,T>::put(const KEY& key, const T& value) {
  int i = index_of(key);
  if (i != -1)
    return change_at(i,value);

  this->ensure_length(used+1);
  map[used++] =  ics::pair<KEY,T>(key,value);
  ++mod_count;
  return map[used-1].second;
}


template<class KEY,class T>
T ArrayMap<KEY,T>::erase(const KEY& key) {
  int i = index_of(key);
  if (i != -1)
    return erase_at(i);

  std::ostringstream answer;
  answer << "ArrayMap::erase: key(" << key << ") not in Map";
  throw KeyError(answer.str());
}


template<class KEY,class T>
void ArrayMap<KEY,T>::clear() {
  used = 0;
  ++mod_count;
}


template<class KEY,class T>
int ArrayMap<KEY,T>::put (ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop) {
  int count = 0;
  for (; start != stop; ++start) {
    ++count;
    put(start->first,start->second);
  }

  return count;
}


template<class KEY,class T>
T& ArrayMap<KEY,T>::operator [] (const KEY& key) {
  int i = index_of(key);
  if (i != -1)
    return map[i].second;

  this->ensure_length(used+1);
  map[used++] = ics::pair<KEY,T>(key,T());
  ++mod_count;
  return map[used-1].second;
}


template<class KEY,class T>
const T& ArrayMap<KEY,T>::operator [] (const KEY& key) const {
  int i = index_of(key);
  if (i != -1)
    return map[i].second;

  std::ostringstream answer;
  answer << "ArrayMap::operator []: key(" << key << ") not in Map";
  throw KeyError(answer.str());
}


template<class KEY,class T>
ArrayMap<KEY,T>& ArrayMap<KEY,T>::operator = (const ArrayMap<KEY,T>& rhs) {
  if (this == &rhs)
    return *this;
  this->ensure_length(rhs.used);
  used = rhs.used;
  for (int i=0; i<used; ++i)
    map[i] = rhs.map[i];
  ++mod_count;
  return *this;
}


template<class KEY,class T>
bool ArrayMap<KEY,T>::operator == (const Map<KEY,T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used != rhs.size())
    return false;
  for (int i=0; i<used; ++i)
    // Uses ! and ==, so != on T need not be defined
    if (!rhs.has_key(map[i].first) || !(map[i].second == rhs[map[i].first]))
      return false;

  return true;
}


template<class KEY,class T>
bool ArrayMap<KEY,T>::operator != (const Map<KEY,T>& rhs) const {
  return !(*this == rhs);
}



template<class KEY,class T>
std::ostream& operator << (std::ostream& outs, const ArrayMap<KEY,T>& m) {
  outs << "map[";

  if (!m.empty()) {
    //m.map[i] couts as map[pair[key,value]]
    outs << m.map[0].first << "->" << m.map[0].second;
    for (int i = 1; i < m.used; ++i) {
      outs << "," << m.map[i].first << "->" << m.map[i].second;
    }
  }

  outs << "]";
  return outs;
}


//KLUDGE: memory-leak
template<class KEY,class T>
auto ArrayMap<KEY,T>::ibegin () const -> ics::Iterator<Entry>& {
  return *(new Iterator(const_cast<ArrayMap<KEY,T>*>(this),0));
}

//KLUDGE: memory-leak
template<class KEY,class T>
auto ArrayMap<KEY,T>::iend () const -> ics::Iterator<Entry>& {
  return *(new Iterator(const_cast<ArrayMap<KEY,T>*>(this),used));
}


template<class KEY,class T>
auto ArrayMap<KEY,T>::begin () const -> ArrayMap<KEY,T>::Iterator {
   return Iterator(const_cast<ArrayMap<KEY,T>*>(this),0);
}


template<class KEY,class T>
auto ArrayMap<KEY,T>::end () const -> ArrayMap<KEY,T>::Iterator {
  return Iterator(const_cast<ArrayMap<KEY,T>*>(this),used);
}


template<class KEY,class T>
int ArrayMap<KEY,T>::index_of(const KEY& key) const {
  for (int i=0; i<used; ++i)
    if (map[i].first == key)
      return i;

  return -1;
}


template<class KEY,class T>
T ArrayMap<KEY,T>::change_at(int i, const T& value) {
  T old_value = map[i].second;
  map[i].second = value;
  ++mod_count;
  return old_value;
}


template<class KEY,class T>
T ArrayMap<KEY,T>::erase_at(int i) {
  T erased = map[i].second;
  map[i] = map[--used];
  ++mod_count;
  return erased;
}


template<class KEY,class T>
void ArrayMap<KEY,T>::ensure_length(int new_length) {
  if (length >= new_length)
    return;
  Entry*  old_map  = map;
  length = std::max(new_length,2*length);
  map = new Entry[length];
  for (int i=0; i<used; ++i)
    map[i] = old_map[i];

  delete [] old_map;
}


template<class KEY,class T>
ArrayMap<KEY,T>::Iterator::Iterator(ArrayMap<KEY,T>* iterate_over, int initial) : current(initial), ref_map(iterate_over) {
  expected_mod_count = ref_map->mod_count;
}


//KLUDGE: must define in .hpp
//template<class KEY,class T>
//ArrayMap<KEY,T>::Iterator::~Iterator() {}

template<class KEY,class T>
auto ArrayMap<KEY,T>::Iterator::erase() -> Entry {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("ArrayMap::Iterator::erase Iterator cursor already erased");
  if (current < 0 || current >= ref_map->used)
    throw CannotEraseError("ArrayMap::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  Entry to_return = ref_map->map[current];
  ref_map->erase_at(current);
  expected_mod_count = ref_map->mod_count;
  return to_return;
}


template<class KEY,class T>
std::string ArrayMap<KEY,T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_map->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}


//KLUDGE: cannot use Entry
template<class KEY,class T>
auto  ArrayMap<KEY,T>::Iterator::operator ++ () -> const ics::Iterator<ics::pair<KEY,T>>& {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::operator ++");

  if (current >= ref_map->used)
    return *this;

  if (!can_erase)
    can_erase = true;
  else
    ++current;

  return *this;
}


//KLUDGE: can create garbage! (can return local value!)
template<class KEY,class T>
auto ArrayMap<KEY,T>::Iterator::operator ++ (int) -> const ics::Iterator<ics::pair<KEY,T>>&{
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::operator ++(int)");

  if (current >= ref_map->used)
    return *this;

  Iterator* to_return = new Iterator(this->ref_map,current-1);
  if (!can_erase)
    can_erase = true;
  else{
    ++to_return->current;
    ++current;
  }

  return *to_return;
}


template<class KEY,class T>
bool ArrayMap<KEY,T>::Iterator::operator == (const ics::Iterator<Entry>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayMap::Iterator::operator ==");
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::operator ==");
  if (ref_map != rhsASI->ref_map)
    throw ComparingDifferentIteratorsError("ArrayMap::Iterator::operator ==");

  return current == rhsASI->current;
}


template<class KEY,class T>
bool ArrayMap<KEY,T>::Iterator::operator != (const ics::Iterator<Entry>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayMap::Iterator::operator !=");
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::operator !=");
  if (ref_map != rhsASI->ref_map)
    throw ComparingDifferentIteratorsError("ArrayMap::Iterator::operator !=");

  return current != rhsASI->current;
}


template<class KEY,class T>
ics::pair<KEY,T>& ArrayMap<KEY,T>::Iterator::operator *() const {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::operator *");
  if (!can_erase || current < 0 || current >= ref_map->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_map->size();
    throw IteratorPositionIllegal("ArrayMap::Iterator::operator * Iterator illegal: "+where.str());
  }

  return ref_map->map[current];
}


template<class KEY,class T>
ics::pair<KEY,T>* ArrayMap<KEY,T>::Iterator::operator ->() const {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("ArrayMap::Iterator::operator ->");
  if (!can_erase || current < 0 || current >= ref_map->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_map->size();
    throw IteratorPositionIllegal("ArrayMap::Iterator::operator -> Iterator illegal: "+where.str());
  }

  return &(ref_map->map[current]);
}

}

#endif /* ARRAY_MAP_HPP_ */
