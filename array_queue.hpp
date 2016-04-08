#ifndef ARRAY_QUEUE_HPP_
#define ARRAY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "queue.hpp"


namespace ics {

template<class T> class ArrayQueue : public Queue<T>  {
  public:
    ArrayQueue();
    explicit ArrayQueue(int initialLength);
    ArrayQueue(const ArrayQueue<T>& to_copy);
    ArrayQueue(std::initializer_list<T> il);
    ArrayQueue(ics::Iterator<T>& start, const ics::Iterator<T>& stop);
    virtual ~ArrayQueue();

    virtual bool empty      () const;
    virtual int  size       () const;
    virtual T&   peek       () const;
    virtual std::string str () const;

    virtual int  enqueue (const T& element);
    virtual T    dequeue ();
    virtual void clear   ();

    virtual int enqueue (ics::Iterator<T>& start, const ics::Iterator<T>& stop);

    virtual ArrayQueue<T>& operator = (const ArrayQueue<T>& rhs);
    virtual bool operator == (const Queue<T>& rhs) const;
    virtual bool operator != (const Queue<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const ArrayQueue<T2>& q);

    class Iterator : public ics::Iterator<T> {
      public:
        //KLUDGE should be callable only in begin/end
        Iterator(ArrayQueue<T>* iterate_over, int initial);
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
        int            current;  //if can_erase is false, this value is unusable
        ArrayQueue<T>* ref_queue;
        int            expected_mod_count;
        bool           can_erase = true;
    };

    //For explicit use: Iterator<...>& it = c.ibegin(); ... or for (Iterator<...>& it = c.ibegin(); it != c.iend(); ++it)...
    virtual ics::Iterator<T>& ibegin () const;
    virtual ics::Iterator<T>& iend   () const;

    //For implicit use: for (... i : c)...
    virtual Iterator begin () const;
    virtual Iterator end   () const;

  private:
    T*  queue;
    int length    =  1; //Physical length of array (must be > .size())
    int front     =  0; //Array index of front in queue
    int rear      =  0; //Array index one beyond rear in queue
    int mod_count =  0; //For sensing concurrent modification
    int  erase_at(int i);
    void ensure_length(int new_length);
    bool is_in(int i) const;
  };




template<class T>
ArrayQueue<T>::ArrayQueue() {
  queue = new T[length];
}


template<class T>
ArrayQueue<T>::ArrayQueue(int initial_length) : length(initial_length) {
  if (length < 1)
    length = 1;
  queue = new T[length];
}


template<class T>
ArrayQueue<T>::ArrayQueue(const ArrayQueue<T>& to_copy) : length(to_copy.length)  {
  queue = new T[length];
  front = 0;
  rear = to_copy.size();
  for (int i=0; i<rear; ++i)
    queue[i] = to_copy.queue[(to_copy.front+i)%to_copy.length];
}


template<class T>
ArrayQueue<T>::ArrayQueue(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  queue = new T[length];
  enqueue(start,stop);
}


template<class T>
ArrayQueue<T>::ArrayQueue(std::initializer_list<T> il) {
  queue = new T[length];
  for (T q_elem : il)
    enqueue(q_elem);
}


template<class T>
ArrayQueue<T>::~ArrayQueue() {
  delete[] queue;
}


template<class T>
bool ArrayQueue<T>::empty() const {
  return front == rear;
}


template<class T>
inline int ArrayQueue<T>::size() const {
  return rear >= front ? rear-front : length-(front-rear);
}


template<class T>
T& ArrayQueue<T>::peek () const {
  if (this->empty())
    throw EmptyError("ArrayQueue::peek");

  return queue[front];
}


template<class T>
std::string ArrayQueue<T>::str() const {
  std::ostringstream answer;
  if (length == 0) {
    answer << "queue[]";
  }else{
    answer << "queue[";
    for (int i=0; i<length; ++i) {
      if (this->is_in(i))
        answer << queue[i];
      answer << (i == length-1 ? "]" : ",");
    }
  }
  answer << "(length=" << length << ",front=" << front << ",rear=" << rear << ",mod_count=" << mod_count << ")";
  return answer.str();
}


template<class T>
int ArrayQueue<T>::enqueue(const T& element) {
  this->ensure_length(this->size()+2);
  queue[rear] = element;
  rear = (rear+1)%length;
  ++mod_count;
  return 1;
}


template<class T>
T ArrayQueue<T>::dequeue() {
  if (this->empty())
    throw EmptyError("ArrayQueue::dequeue");

  T answer = queue[front];
  front = (front+1)%length;
  ++mod_count;
  return answer;
}


template<class T>
void ArrayQueue<T>::clear() {
  front = 0;
  rear  = 0;
  ++mod_count;
}


template<class T>
int ArrayQueue<T>::enqueue(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; ++start)
    count += enqueue(*start);

  return count;
}


template<class T>
ArrayQueue<T>& ArrayQueue<T>::operator = (const ArrayQueue<T>& rhs) {
  if (this == &rhs)
    return *this;
  front = 0;
  rear = rhs.size();
  this->ensure_length(rear+1);
  for (int i=0; i<rear; ++i)
    queue[i] = rhs.queue[(rhs.front+i)%rhs.length];
  ++mod_count;
  return *this;
}


template<class T>
bool ArrayQueue<T>::operator == (const Queue<T>& rhs) const {
  if (this == &rhs)
    return true;
  int used = this->size();
  if (used != rhs.size())
    return false;
  ics::Iterator<T>& rhs_i = rhs.ibegin();
  for (int i=0; i<used; ++i,++rhs_i)
    // Uses ! and ==, so != on T need not be defined
    if (!(queue[(front+i)%length] == *rhs_i))
      return false;

  return true;
}

template<class T>
bool ArrayQueue<T>::operator != (const Queue<T>& rhs) const {
  return !(*this == rhs);
}


template<class T>
std::ostream& operator << (std::ostream& outs, const ArrayQueue<T>& q) {
  outs << "queue[";

  if (!q.empty()) {
    outs << q.queue[q.front];
    for (int i=(q.front+1)%q.length; i!=q.rear; i=(i+1)%q.length)
      outs << ","<< q.queue[i];
  }

  outs << "]:rear";
  return outs;
}

//KLUDGE: memory-leak
template<class T>
auto ArrayQueue<T>::ibegin () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArrayQueue<T>*>(this),front));
}


//KLUDGE: memory-leak
template<class T>
auto ArrayQueue<T>::iend () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArrayQueue<T>*>(this),rear));
}


template<class T>
auto ArrayQueue<T>::begin () const -> ArrayQueue<T>::Iterator {
  return Iterator(const_cast<ArrayQueue<T>*>(this),front);
}


template<class T>
auto ArrayQueue<T>::end () const -> ArrayQueue<T>::Iterator {
  return Iterator(const_cast<ArrayQueue<T>*>(this),rear);
}


template<class T>
int ArrayQueue<T>::erase_at(int i) {
  int shift_count = (rear >= i ? rear-i-1: length-(i-rear))-1;
  int to   = i;
  int from = (to+1)%length;
  for (int i=0; i<=shift_count; ++i) {
    queue[to] = queue[from];
    to = from;
    from = (from+1)%length;
  }
  rear = (rear == 0 ? length-1 : rear - 1);
  ++mod_count;
  return 1;
}


template<class T>
void ArrayQueue<T>::ensure_length(int new_length) {
  if (length >= new_length)
    return;
  T*  old_queue  = queue;
  int old_length = length;
  int used = this->size(); //must precede length change!
  length = std::max(new_length,2*length);
  queue = new T[length];
  for (int i=0; i<used; ++i)
    queue[i] = old_queue[(front+i)%old_length];
  front = 0;
  rear  = used;

  delete [] old_queue;
}


template<class T>
bool ArrayQueue<T>::is_in(int i) const {
  return  rear >= front ? (i>=front && i<rear) : (i>=front || i<rear);
}





template<class T>
ArrayQueue<T>::Iterator::Iterator(ArrayQueue<T>* iterate_over, int initial) : current(initial), ref_queue(iterate_over) {
  expected_mod_count = ref_queue->mod_count;
}

template<class T>
ArrayQueue<T>::Iterator::~Iterator() {}

template<class T>
T ArrayQueue<T>::Iterator::erase() {
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("ArrayQueue::Iterator::erase Iterator cursor already erased");
  if (!ref_queue->is_in(current))
    throw CannotEraseError("ArrayQueue::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  T to_return = ref_queue->queue[current];
  ref_queue->erase_at(current);
  expected_mod_count = ref_queue->mod_count;
  return to_return;
}


template<class T>
std::string ArrayQueue<T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_queue->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}


template<class T>
const ics::Iterator<T>& ArrayQueue<T>::Iterator::operator ++ () {
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::operator ++");

  if (current == ref_queue->rear)
    return *this;

  if (!can_erase)
    can_erase = true;
  else
    current = (current+1)%ref_queue->length;

  return *this;
}


//KLUDGE: can create garbage! (can return local value!)
template<class T>
const ics::Iterator<T>& ArrayQueue<T>::Iterator::operator ++ (int) {
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::operator ++(int)");

  if (current == ref_queue->rear)
    return *this;

  int before = (current == 0 ? current-1 : current - 1);
  Iterator* to_return = new Iterator(this->ref_queue,before);
  if (!can_erase)
    can_erase = true;
  else{
    to_return->current = (to_return->current+1)%ref_queue->length;
    current = (current+1)%ref_queue->length;
  }

  return *to_return;
}


template<class T>
bool ArrayQueue<T>::Iterator::operator == (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayQueue::Iterator::operator ==");
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::operator ==");
  if (ref_queue != rhsASI->ref_queue)
    throw ComparingDifferentIteratorsError("ArrayQueue::Iterator::operator ==");

  return current == rhsASI->current;
}


template<class T>
bool ArrayQueue<T>::Iterator::operator != (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayQueue::Iterator::operator !=");
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::operator !=");
  if (ref_queue != rhsASI->ref_queue)
    throw ComparingDifferentIteratorsError("ArrayQueue::Iterator::operator !=");

  return current != rhsASI->current;
}


template<class T>
T& ArrayQueue<T>::Iterator::operator *() const {
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::operator *");
  if (!can_erase || !ref_queue->is_in(current)) {
    std::ostringstream where;
    where << current
          << " when front = " << ref_queue->front << " and "
          << " and rear = " << ref_queue->rear;
    throw IteratorPositionIllegal("ArrayQueue::Iterator::operator * Iterator illegal: "+where.str());
  }

  return ref_queue->queue[current];
}


template<class T>
T* ArrayQueue<T>::Iterator::operator ->() const {
  if (expected_mod_count != ref_queue->mod_count)
    throw ConcurrentModificationError("ArrayQueue::Iterator::operator ->");
  if (!can_erase || !ref_queue->is_in(current)) {
    std::ostringstream where;
    where << current
          << " when front = " << ref_queue->front << " and "
          << " and rear = " << ref_queue->rear;
    throw IteratorPositionIllegal("ArrayQueue::Iterator::operator -> Iterator illegal: "+where.str());
  }

  return &ref_queue->queue[current];
}

}

#endif /* ARRAY_QUEUE_HPP_ */
