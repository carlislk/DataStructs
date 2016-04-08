#ifndef ARRAY_PRIORITY_QUEUE_HPP_
#define ARRAY_PRIORITY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "priority_queue.hpp"


namespace ics {

template<class T> class ArrayPriorityQueue : public PriorityQueue<T>  {
  using PriorityQueue<T>::gt;  //Required because of templated classes
  public:
    ArrayPriorityQueue() = delete;
    explicit ArrayPriorityQueue(bool (*agt)(const T& a, const T& b));
    ArrayPriorityQueue(int initialLength,bool (*agt)(const T& a, const T& b));
    ArrayPriorityQueue(const ArrayPriorityQueue<T>& to_copy);
    ArrayPriorityQueue(std::initializer_list<T> il,bool (*agt)(const T& a, const T& b));
    ArrayPriorityQueue(ics::Iterator<T>& start, const ics::Iterator<T>& stop,bool (*agt)(const T& a, const T& b));
    virtual ~ArrayPriorityQueue();

    virtual bool empty      () const;
    virtual int  size       () const;
    virtual T&   peek       () const;
    virtual std::string str () const;

    virtual int  enqueue (const T& element);
    virtual T    dequeue ();
    virtual void clear   ();

    virtual int enqueue (ics::Iterator<T>& start, const ics::Iterator<T>& stop);

    virtual ArrayPriorityQueue<T>& operator = (const ArrayPriorityQueue<T>& rhs);
    virtual bool operator == (const PriorityQueue<T>& rhs) const;
    virtual bool operator != (const PriorityQueue<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const ArrayPriorityQueue<T2>& p);

    class Iterator : public ics::Iterator<T> {
      public:
        //KLUDGE should be callable only in begin/end
        Iterator(ArrayPriorityQueue<T>* iterate_over, int initial);
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
        int                    current;  //if can_erase is false, this value is unusable
        ArrayPriorityQueue<T>* ref_pq;
        int                    expected_mod_count;
        bool                   can_erase = true;
    };

    //For explicit use: Iterator<...>& it = c.ibegin(); ... or for (Iterator<...>& it = c.ibegin(); it != c.iend(); ++it)...
    virtual ics::Iterator<T>& ibegin () const;
    virtual ics::Iterator<T>& iend   () const;

    //For implicit use: for (... i : c)...
    virtual Iterator begin () const;
    virtual Iterator end   () const;

  private:
    T*  pq;
    //See base class PriorityQueue
    //bool (*gt)(const T& a, const T& b);// gt(a,b) = true iff a has higher priority than b
    int length    = 0;                   //Physical length of array (must be > .size()
    int used      = 0;                   //Amount of array used
    int mod_count = 0;                   //For sensing concurrent modification
    int erase_at(int i);
    void ensure_length(int new_length);
  };





template<class T>
ArrayPriorityQueue<T>::ArrayPriorityQueue(bool (*agt)(const T& a, const T& b)) : PriorityQueue<T>(agt) {
  pq = new T[length];
}


template<class T>
ArrayPriorityQueue<T>::ArrayPriorityQueue(int initial_length, bool (*agt)(const T& a, const T& b))
  : PriorityQueue<T>(agt), length(initial_length) {
  if (length < 0)
    length = 0;
  pq = new T[length];
}


template<class T>
ArrayPriorityQueue<T>::ArrayPriorityQueue(const ArrayPriorityQueue<T>& to_copy)
  : PriorityQueue<T>(to_copy.gt), length(to_copy.length), used(to_copy.used) {
  pq = new T[length];
  for (int i=0; i<to_copy.used; ++i)
    pq[i] = to_copy.pq[i];
}


template<class T>
ArrayPriorityQueue<T>::ArrayPriorityQueue(ics::Iterator<T>& start, const ics::Iterator<T>& stop, bool (*agt)(const T& a, const T& b))
  : PriorityQueue<T>(agt) {
  pq = new T[length];
  enqueue(start,stop);
}


template<class T>
ArrayPriorityQueue<T>::ArrayPriorityQueue(std::initializer_list<T> il, bool (*agt)(const T& a, const T& b))
  : PriorityQueue<T>(agt) {
  pq = new T[length];
  for (T pq_elem : il)
    enqueue(pq_elem);
}


template<class T>
ArrayPriorityQueue<T>::~ArrayPriorityQueue() {
  delete[] pq;
}


template<class T>
inline bool ArrayPriorityQueue<T>::empty() const {
  return used == 0;
}


template<class T>
int ArrayPriorityQueue<T>::size() const {
  return used;
}


template<class T>
T& ArrayPriorityQueue<T>::peek () const {
  if (empty())
    throw EmptyError("ArrayPriorityQueue::peek");

  return pq[used-1];
}


template<class T>
std::string ArrayPriorityQueue<T>::str() const {
  std::ostringstream answer;
  answer << *this << "(length=" << length << ",used=" << used << ",mod_count=" << mod_count << ")";
  return answer.str();
}


template<class T>
int ArrayPriorityQueue<T>::enqueue(const T& element) {
  this->ensure_length(used+1);
  pq[used++] = element;
  for (int i=used-2; i>=0; --i)
    if (gt(pq[i],pq[i+1])) {//gt is in the base class; KLUDGE swap<T>?
      T temp = pq[i];
      pq[i] = pq[i+1];
      pq[i+1] = temp;
    }else
      break;
  ++mod_count;
  return 1;
}


template<class T>
T ArrayPriorityQueue<T>::dequeue() {
  if (this->empty())
    throw EmptyError("ArrayPriorityQueue::dequeue");

  ++mod_count;
  return pq[--used];
}


template<class T>
void ArrayPriorityQueue<T>::clear() {
  used = 0;
  ++mod_count;
}


template<class T>
int ArrayPriorityQueue<T>::enqueue(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; ++start)
    count += enqueue(*start);

  return count;
}


template<class T>
ArrayPriorityQueue<T>& ArrayPriorityQueue<T>::operator = (const ArrayPriorityQueue<T>& rhs) {
  if (this == &rhs)
    return *this;
  this->ensure_length(rhs.used);
  gt   = rhs.gt;  //gt is in the base class
  used = rhs.used;
  for (int i=0; i<used; ++i)
    pq[i] = rhs.pq[i];
  ++mod_count;
  return *this;
}


template<class T>
  bool ArrayPriorityQueue<T>::operator == (const PriorityQueue<T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used != rhs.size())
    return false;
  if (gt != rhs.gt)
    return false;
  if (used != rhs.size())
    return false;
  //KLUDGE: should check for same == function used to prioritize, but cannot unless
  //  it is made part of the PriorityQueue class (should it be? protected)?
  ics::Iterator<T>& rhs_i = rhs.ibegin();
  for (int i=used-1; i>=0; --i,++rhs_i)
    // Uses ! and ==, so != on T need not be defined
    if (!(pq[i] == *rhs_i))
      return false;

  return true;
}


template<class T>
bool ArrayPriorityQueue<T>::operator != (const PriorityQueue<T>& rhs) const {
  return !(*this == rhs);
}


template<class T>
std::ostream& operator << (std::ostream& outs, const ArrayPriorityQueue<T>& p) {
  outs << "priority_queue[";

  if (!p.empty()) {
    outs << p.pq[0];
    for (int i = 1; i < p.used; ++i)
      outs << ","<< p.pq[i];
  }

  outs << "]:highest";
  return outs;
}


//KLUDGE: memory-leak
template<class T>
auto ArrayPriorityQueue<T>::ibegin () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArrayPriorityQueue<T>*>(this),used-1));
}


//KLUDGE: memory-leak
template<class T>
auto ArrayPriorityQueue<T>::iend () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArrayPriorityQueue<T>*>(this),-1));
}


template<class T>
auto ArrayPriorityQueue<T>::begin () const -> ArrayPriorityQueue<T>::Iterator {
  return Iterator(const_cast<ArrayPriorityQueue<T>*>(this),used-1);
}


template<class T>
auto ArrayPriorityQueue<T>::end () const -> ArrayPriorityQueue<T>::Iterator {
  return Iterator(const_cast<ArrayPriorityQueue<T>*>(this),-1);
}


template<class T>
int ArrayPriorityQueue<T>::erase_at(int i) {
  for (int j=i; j<used-1; ++j)
    pq[j] = pq[j+1];
  --used;
  ++mod_count;
  return 1;
}


template<class T>
void ArrayPriorityQueue<T>::ensure_length(int new_length) {
  if (length >= new_length)
    return;
  T*  old_pq  = pq;
  length = std::max(new_length,2*length);
  pq = new T[length];
  for (int i=0; i<used; ++i)
    pq[i] = old_pq[i];

  delete [] old_pq;
}





template<class T>
ArrayPriorityQueue<T>::Iterator::Iterator(ArrayPriorityQueue<T>* iterate_over, int initial) : current(initial), ref_pq(iterate_over) {
  expected_mod_count = ref_pq->mod_count;
}


template<class T>
ArrayPriorityQueue<T>::Iterator::~Iterator() {}


template<class T>
T ArrayPriorityQueue<T>::Iterator::erase() {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("ArrayPriorityQueue::Iterator::erase Iterator cursor already erased");
  if (current < 0 || current >= ref_pq->used)
    throw CannotEraseError("ArrayPriorityQueue::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  T to_return = ref_pq->pq[current];
  ref_pq->erase_at(current);
  --current;
  expected_mod_count = ref_pq->mod_count;
  return to_return;
}


template<class T>
std::string ArrayPriorityQueue<T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_pq->str() << "/current=" << current << "/expected_mod_count=" << expected_mod_count << "/can_erase=" << can_erase;
  return answer.str();
}


template<class T>
const ics::Iterator<T>& ArrayPriorityQueue<T>::Iterator::operator ++ () {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator ++");

  if (current < 0)
    return *this;

  if (!can_erase)
    can_erase = true;
  else
    --current;

  return *this;
}


//KLUDGE: can create garbage! (can return local value!)
template<class T>
const ics::Iterator<T>& ArrayPriorityQueue<T>::Iterator::operator ++ (int) {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator ++(int)");

  if (current < 0)
    return *this;

  Iterator* to_return = new Iterator(this->ref_pq,current+1);
  if (!can_erase)
    can_erase = true;
  else{
    --to_return->current;
    --current;
  }

  return *to_return;
}


template<class T>
bool ArrayPriorityQueue<T>::Iterator::operator == (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayPriorityQueue::Iterator::operator ==");
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator ==");
  if (ref_pq != rhsASI->ref_pq)
    throw ComparingDifferentIteratorsError("ArrayPriorityQueue::Iterator::operator ==");

  return current == rhsASI->current;
}


template<class T>
bool ArrayPriorityQueue<T>::Iterator::operator != (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayPriorityQueue::Iterator::operator !=");
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator !=");
  if (ref_pq != rhsASI->ref_pq)
    throw ComparingDifferentIteratorsError("ArrayPriorityQueue::Iterator::operator !=");

  return current != rhsASI->current;
}


template<class T>
T& ArrayPriorityQueue<T>::Iterator::operator *() const {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator *");
  if (!can_erase || current < 0 || current >= ref_pq->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_pq->size();
    throw IteratorPositionIllegal("ArrayPriorityQueue::Iterator::operator * Iterator illegal: "+where.str());
  }

  return ref_pq->pq[current];
}


template<class T>
T* ArrayPriorityQueue<T>::Iterator::operator ->() const {
  if (expected_mod_count !=  ref_pq->mod_count)
    throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator ->");
  if (!can_erase || current < 0 || current >= ref_pq->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_pq->size();
    throw IteratorPositionIllegal("ArrayPriorityQueue::Iterator::operator -> Iterator illegal: "+where.str());
  }

  return &ref_pq->pq[current];
}

}

#endif /* ARRAY_PRIORITY_QUEUE_HPP_ */
