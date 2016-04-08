#ifndef ARRAY_STACK_HPP_
#define ARRAY_STACK_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "stack.hpp"


namespace ics {

template<class T> class ArrayStack : public Stack<T>  {
  public:
    ArrayStack();
    explicit ArrayStack(int initialLength);
    ArrayStack(const ArrayStack<T>& to_copy);
    ArrayStack(std::initializer_list<T> il);
    ArrayStack(ics::Iterator<T>& start, const ics::Iterator<T>& stop);
    virtual ~ArrayStack();

    virtual bool empty      () const;
    virtual int  size       () const;
    virtual T&   peek       () const;
    virtual std::string str () const;

    virtual int  push (const T& element);
    virtual T    pop  ();
    virtual void clear();

    virtual int push (ics::Iterator<T>& start, const ics::Iterator<T>& stop);

    virtual ArrayStack<T>& operator = (const ArrayStack<T>& rhs);
    virtual bool operator == (const Stack<T>& rhs) const;
    virtual bool operator != (const Stack<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const ArrayStack<T2>& s);

    class Iterator : public ics::Iterator<T> {
      public:
        //KLUDGE should be callable only in begin/end
        Iterator(ArrayStack<T>* iterate_over, int initial);
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
        ArrayStack<T>* ref_stack;
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
      T*  stack;
      int length    = 0; //Physical length of array
      int used      = 0; //Amount of array used
      int mod_count = 0; //For sensing concurrent modification
      int erase_at(int i);
      void ensure_length(int new_length);
  };





template<class T>
ArrayStack<T>::ArrayStack() {
  stack = new T[length];
}


template<class T>
ArrayStack<T>::ArrayStack(int initial_length) : length(initial_length) {
  if (length < 0)
    length = 0;
  stack = new T[length];
}


template<class T>
ArrayStack<T>::ArrayStack(const ArrayStack<T>& to_copy) : length(to_copy.length), used(to_copy.used) {
  stack = new T[length];
  for (int i=0; i<to_copy.used; ++i)
    stack[i] = to_copy.stack[i];
}


template<class T>
ArrayStack<T>::ArrayStack(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  stack = new T[length];
  push(start,stop);
}


template<class T>
ArrayStack<T>::ArrayStack(std::initializer_list<T> il) {
  stack = new T[length];
  for (T s_elem : il)
    push(s_elem);
}


template<class T>
ArrayStack<T>::~ArrayStack() {
  delete[] stack;
}


template<class T>
inline bool ArrayStack<T>::empty() const {
  return used == 0;
}


template<class T>
int ArrayStack<T>::size() const {
  return used;
}


template<class T>
T& ArrayStack<T>::peek () const {
  if (empty())
    throw EmptyError("ArrayStack::peek");

  return stack[used-1];
}


template<class T>
std::string ArrayStack<T>::str() const {
  std::ostringstream answer;
  answer << *this << "(length=" << length << ",used=" << used << ",mod_count=" << mod_count << ")";
  return answer.str();
}


template<class T>
int ArrayStack<T>::push(const T& element) {
  this->ensure_length(used+1);
  stack[used++] = element;
  ++mod_count;
  return 1;
}


template<class T>
T ArrayStack<T>::pop() {
  if (this->empty())
    throw EmptyError("ArrayStack::pop");

  ++mod_count;
  return stack[--used];
}


template<class T>
void ArrayStack<T>::clear() {
  used = 0;
  ++mod_count;
}


template<class T>
int ArrayStack<T>::push(ics::Iterator<T>& start, const ics::Iterator<T>& stop) {
  int count = 0;
  for (; start != stop; ++start)
    count += push(*start);

  return count;
}


template<class T>
ArrayStack<T>& ArrayStack<T>::operator = (const ArrayStack<T>& rhs) {
  if (this == &rhs)
    return *this;
  this->ensure_length(rhs.used);
  used = rhs.used;
  for (int i=0; i<used; ++i)
    stack[i] = rhs.stack[i];
  ++mod_count;
  return *this;
}


template<class T>
bool ArrayStack<T>::operator == (const Stack<T>& rhs) const {
  if (this == &rhs)
    return true;
  if (used != rhs.size())
    return false;
  ics::Iterator<T>& rhs_i = rhs.ibegin();
  // Uses ! and ==, so != on T need not be defined
  for (int i=used-1; i>=0; --i,++rhs_i)
    if (!(stack[i] == *rhs_i))
      return false;

  return true;
}


template<class T>
bool ArrayStack<T>::operator != (const Stack<T>& rhs) const {
  return !(*this == rhs);
}


template<class T>
std::ostream& operator << (std::ostream& outs, const ArrayStack<T>& s) {
  if (s.empty()) {
    outs << "stack[]:top";
  }else{
    outs << "stack[";
    for (int i=0; i<s.used; ++i) {
      outs << s.stack[i];
      outs << (i == s.used-1 ? "]:top" : ",");
    }
  }
  return outs;
}


//KLUDGE: memory-leak
template<class T>
auto ArrayStack<T>::ibegin () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArrayStack<T>*>(this),used-1));
}


//KLUDGE: memory-leak
template<class T>
auto ArrayStack<T>::iend () const -> ics::Iterator<T>& {
  return *(new Iterator(const_cast<ArrayStack<T>*>(this),-1));
}


template<class T>
auto ArrayStack<T>::begin () const -> ArrayStack<T>::Iterator {
  return Iterator(const_cast<ArrayStack<T>*>(this),used-1);
}


template<class T>
auto ArrayStack<T>::end () const -> ArrayStack<T>::Iterator {
  return Iterator(const_cast<ArrayStack<T>*>(this),-1);
}


template<class T>
int ArrayStack<T>::erase_at(int i) {
  for (int j=i; j<used-1; ++j)
    stack[j] = stack[j+1];
  --used;
  ++mod_count;
  return 1;
}


template<class T>
void ArrayStack<T>::ensure_length(int new_length) {
  if (length >= new_length)
    return;
  T*  old_stack  = stack;
  length = std::max(new_length,2*length);
  stack = new T[length];
  for (int i=0; i<used; ++i)
    stack[i] = old_stack[i];

  delete [] old_stack;
}





template<class T>
ArrayStack<T>::Iterator::Iterator(ArrayStack<T>* iterate_over, int initial) : current(initial), ref_stack(iterate_over) {
  expected_mod_count = ref_stack->mod_count;
}

template<class T>
ArrayStack<T>::Iterator::~Iterator() {}

template<class T>
T ArrayStack<T>::Iterator::erase() {
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("ArrayStack::Iterator::erase Iterator cursor already erased");
  if (current < 0 || current >= ref_stack->used)
    throw CannotEraseError("ArrayStack::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  T to_return = ref_stack->stack[current];
  ref_stack->erase_at(current);
  --current;
  expected_mod_count = ref_stack->mod_count;
  return to_return;
}

template<class T>
std::string ArrayStack<T>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_stack->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}

template<class T>
const ics::Iterator<T>& ArrayStack<T>::Iterator::operator ++ () {
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::operator ++");

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
const ics::Iterator<T>& ArrayStack<T>::Iterator::operator ++ (int) {
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::operator ++(int)");

  if (current < 0)
    return *this;

  Iterator* to_return = new Iterator(this->ref_stack,current+1);
  if (!can_erase)
    can_erase = true;
  else{
    --to_return->current;
    --current;
  }

  return *to_return;
}

template<class T>
bool ArrayStack<T>::Iterator::operator == (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayStack::Iterator::operator ==");
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::operator ==");
  if (ref_stack != rhsASI->ref_stack)
    throw ComparingDifferentIteratorsError("ArrayStack::Iterator::operator ==");

  return current == rhsASI->current;
}


template<class T>
bool ArrayStack<T>::Iterator::operator != (const ics::Iterator<T>& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("ArrayStack::Iterator::operator !=");
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::operator !=");
  if (ref_stack != rhsASI->ref_stack)
    throw ComparingDifferentIteratorsError("ArrayStack::Iterator::operator !=");

  return current != rhsASI->current;
}

template<class T>
T& ArrayStack<T>::Iterator::operator *() const {
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::operator *");
  if (!can_erase || current < 0 || current >= ref_stack->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_stack->size();
    throw IteratorPositionIllegal("ArrayStack::Iterator::operator * Iterator illegal: "+where.str());
  }

  return ref_stack->stack[current];
}

template<class T>
T* ArrayStack<T>::Iterator::operator ->() const {
  if (expected_mod_count != ref_stack->mod_count)
    throw ConcurrentModificationError("ArrayStack::Iterator::operator ->");
  if (!can_erase || current < 0 || current >= ref_stack->used) {
    std::ostringstream where;
    where << current << " when size = " << ref_stack->size();
    throw IteratorPositionIllegal("ArrayStack::Iterator::operator -> Iterator illegal: "+where.str());
  }

  return &ref_stack->stack[current];
}

}

#endif /* ARRAY_STACK_HPP_ */
