#ifndef ARRAY_SET_HPP_
#define ARRAY_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "set.hpp"


namespace ics {

template<class T> class ArraySet : public Set<T>	{
  public:
	  ArraySet();
	  explicit ArraySet(int initialLength);
	  ArraySet(const ArraySet<T>& to_copy);
	  ArraySet(std::initializer_list<T> il);
    ArraySet(ics::Iterator<T>& start, const ics::Iterator<T>& stop);
	  virtual ~ArraySet();

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

    virtual ArraySet<T>& operator = (const ArraySet<T>& rhs);
    virtual bool operator == (const Set<T>& rhs) const;
    virtual bool operator != (const Set<T>& rhs) const;
    virtual bool operator <= (const Set<T>& rhs) const;
    virtual bool operator <  (const Set<T>& rhs) const;
    virtual bool operator >= (const Set<T>& rhs) const;
    virtual bool operator >  (const Set<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const ArraySet<T2>& s);

    class Iterator : public ics::Iterator<T> {
      public:
        //KLUDGE should be callable only in begin/end
        Iterator(ArraySet<T>* iterate_over, int initial);
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
        int          current;  //if can_erase is false, this value is unusable
        ArraySet<T>* ref_set;
        int          expected_mod_count;
        bool         can_erase = true;
    };

    //For explicit use: Iterator<...>& it = c.ibegin(); ... or for (Iterator<...>& it = c.ibegin(); it != c.iend(); ++it)...
    virtual ics::Iterator<T>& ibegin () const;
    virtual ics::Iterator<T>& iend   () const;

    //For implicit use: for (... i : c)...
    virtual Iterator begin () const;
    virtual Iterator end   () const;

  private:
    T*  set;
    int length    = 0; //Physical length of array
    int used      = 0; //Amount of array used
    int mod_count = 0; //For sensing concurrent modification
    int erase_at(int i);
    void ensure_length(int new_length);
  };





template<class T>
ArraySet<T>::ArraySet() {
  set = new T[length];
}


template<class T>
ArraySet<T>::ArraySet(int initial_length) : length(initial_length) {
  if (length < 0)
    length = 0;
  set = new T[length];
}


template<class T>
ArraySet<T>::ArraySet(const ArraySet<T>& to_copy) : length(to_copy.length), used(to_copy.used) {
  set = new T[length];
  for (int i=0; i<to_copy.used; ++i)
    set[i] = to_copy.set[i];
}


template<class T>
ArraySet<T>::ArraySet(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  set = new T[length];
  insert(start,stop);
}


template<class T>
ArraySet<T>::ArraySet(std::initializer_list<T> il) {
  set = new T[length];
  for (T s_elem : il)
    insert(s_elem);
}


template<class T>
ArraySet<T>::~ArraySet() {
  delete[] set;
}


template<class T>
inline bool ArraySet<T>::empty() const {
  return used == 0;
}


template<class T>
int ArraySet<T>::size() const {
  return used;
}


template<class T>
bool ArraySet<T>::contains (const T& element) const {
  for (int i=0; i<used; ++i)
    if (set[i] == element)
      return true;

  return false;
}


template<class T>
std::string ArraySet<T>::str() const {
  std::ostringstream answer;
  answer << *this << "(length=" << length << ",used=" << used << ",mod_count=" << mod_count << ")";
  return answer.str();
}


template<class T>
bool ArraySet<T>::contains(ics::Iterator<T>& start, const ics::Iterator<T>& stop) const {
  for (; start != stop; ++start)
    if (!contains(*start))
      return false;

  return true;
}


template<class T>
int ArraySet<T>::insert(const T& element) {
  for (int i=0; i<used; ++i)
    if (set[i] == element)
      return 0;

  this->ensure_length(used+1);
  set[used++] = element;
  ++mod_count;
  return 1;
}


template<class T>
int ArraySet<T>::erase(const T& element) {
  for (int i=0; i<used; ++i)
    if (set[i] == element)
      return erase_at(i);

  return 0;
}


template<class T>
void ArraySet<T>::clear() {
  used = 0;
  ++mod_count;
}


template<class T>
int ArraySet<T>::insert(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; ++start)
    count += insert(*start);

  return count;
}

template<class T>
int ArraySet<T>::erase(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; start++)
    count += erase(*start);
  return count;
}


template<class T>
int ArraySet<T>::retain(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  ArraySet<T> s(start,stop);
  int count = 0;
  for (int i=0; i<used; i++)
    if (!s.contains(set[i])) {
      erase_at(i);
      --i;
    }else
      ++count;

  return count;
}


template<class T>
ArraySet<T>& ArraySet<T>::operator = (const ArraySet<T>& rhs) {
  if (this == &rhs)
    return *this;
  this->ensure_length(rhs.used);
  used = rhs.used;
  for (int i=0; i<used; ++i)
    set[i] = rhs.set[i];
  ++mod_count;
  return *this;
}


template<class T>
bool ArraySet<T>::operator == (const Set<T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used != rhs.size())
    return false;
  for (int i=0; i<used; ++i)
    if (!rhs.contains(set[i]))
      return false;

  return true;
}


template<class T>
bool ArraySet<T>::operator != (const Set<T>& rhs) const {
  return !(*this == rhs);
}

template<class T>
bool ArraySet<T>::operator <= (const Set<T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used > rhs.size())
    return false;
  for (int i=0; i<used; ++i)
    if (!rhs.contains(set[i]))
      return false;

  return true;
}


template<class T>
bool ArraySet<T>::operator < (const Set<T>& rhs) const {
  if (this == &rhs)
    return false;
  if (used >= rhs.size())
    return false;
  for (int i=0; i<used; ++i)
    if (!rhs.contains(set[i]))
      return false;

  return true;
}


template<class T>
bool ArraySet<T>::operator >= (const Set<T>& rhs) const {
  return rhs <= *this;
}


template<class T>
bool ArraySet<T>::operator > (const Set<T>& rhs) const {
  return rhs < *this;
}


template<class T>
std::ostream& operator << (std::ostream& outs, const ArraySet<T>& s) {
  outs << "set[";

  if (!s.empty()) {
    outs << s.set[0];
    for (int i = 1; i < s.used; ++i)
      outs << ","<< s.set[i];
  }

  outs << "]";
  return outs;
}


//KLUDGE: memory-leak
template<class T>
auto ArraySet<T>::ibegin () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArraySet<T>*>(this),0));
}


//KLUDGE: memory-leak
template<class T>
auto ArraySet<T>::iend () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArraySet<T>*>(this),used));
}


template<class T>
auto ArraySet<T>::begin () const -> ArraySet<T>::Iterator {
  return Iterator(const_cast<ArraySet<T>*>(this),0);
}


template<class T>
auto ArraySet<T>::end () const -> ArraySet<T>::Iterator {
  return Iterator(const_cast<ArraySet<T>*>(this),used);
}


template<class T>
int ArraySet<T>::erase_at(int i) {
  set[i] = set[--used];
  ++mod_count;
  return 1;
}


template<class T>
void ArraySet<T>::ensure_length(int new_length) {
  if (length >= new_length)
    return;
  T*  old_set  = set;
  length = std::max(new_length,2*length);
  set = new T[length];
  for (int i=0; i<used; ++i)
    set[i] = old_set[i];

  delete [] old_set;
}





template<class T>
ArraySet<T>::Iterator::Iterator(ArraySet<T>* iterate_over, int initial) : current(initial), ref_set(iterate_over) {
  expected_mod_count = ref_set->mod_count;
}


template<class T>
ArraySet<T>::Iterator::~Iterator() {}


template<class T>
T ArraySet<T>::Iterator::erase() {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("ArraySet::Iterator::erase Iterator cursor already erased");
  if (current < 0 || current >= ref_set->used)
    throw CannotEraseError("ArraySet::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  T to_return = ref_set->set[current];
  ref_set->erase_at(current);
  expected_mod_count = ref_set->mod_count;
  return to_return;
}


template<class T>
std::string ArraySet<T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_set->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}


template<class T>
const ics::Iterator<T>& ArraySet<T>::Iterator::operator ++ () {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::operator ++");

  if (current >= ref_set->used)
    return *this;

  if (!can_erase)
    can_erase = true;
  else
    ++current;

  return *this;
}


//KLUDGE: can create garbage! (can return local value!)
template<class T>
const ics::Iterator<T>& ArraySet<T>::Iterator::operator ++ (int) {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::operator ++(int)");

  if (current >= ref_set->used)
    return *this;

  Iterator* to_return = new Iterator(this->ref_set,current-1);
  if (!can_erase)
    can_erase = true;
  else{
    ++to_return->current;
    ++current;
  }

  return *to_return;
}


template<class T>
bool ArraySet<T>::Iterator::operator == (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArraySet::Iterator::operator ==");
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::operator ==");
  if (ref_set != rhsASI->ref_set)
    throw ComparingDifferentIteratorsError("ArraySet::Iterator::operator ==");

  return current == rhsASI->current;
}


template<class T>
bool ArraySet<T>::Iterator::operator != (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArraySet::Iterator::operator !=");
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::operator !=");
  if (ref_set != rhsASI->ref_set)
    throw ComparingDifferentIteratorsError("ArraySet::Iterator::operator !=");

  return current != rhsASI->current;
}


template<class T>
T& ArraySet<T>::Iterator::operator *() const {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::operator *");
  if (!can_erase || current < 0 || current >= ref_set->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_set->size();
    throw IteratorPositionIllegal("ArraySet::Iterator::operator * Iterator illegal: "+where.str());
  }

  return ref_set->set[current];
}


template<class T>
T* ArraySet<T>::Iterator::operator ->() const {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("ArraySet::Iterator::operator ->");
  if (!can_erase || current < 0 || current >= ref_set->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_set->size();
    throw IteratorPositionIllegal("ArraySet::Iterator::operator -> Iterator illegal: "+where.str());
  }

  return &ref_set->set[current];
}

}

#endif /* ARRAY_SET_HPP_ */
