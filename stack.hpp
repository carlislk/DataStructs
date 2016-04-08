#ifndef STACK_HPP_
#define STACK_HPP_

#include <iostream>
#include "iterator.hpp"

namespace ics {

template<class T> class Stack  {
  public:
    virtual ~Stack();

    virtual bool empty      () const = 0;
    virtual int  size       () const = 0;
    virtual T&   peek       () const = 0;
    virtual std::string str () const = 0;

    virtual int  push (const T& element) = 0;
    virtual T    pop  () = 0;
    virtual void clear() = 0;

    virtual int push (Iterator<T>& start, const Iterator<T>& stop) = 0;

    virtual bool operator == (const Stack<T>& rhs) const = 0;
    virtual bool operator != (const Stack<T>& rhs) const = 0;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const Stack<T2>& s);

    virtual Iterator<T>& ibegin () const = 0;
    virtual Iterator<T>& iend   () const = 0;
  };





template<class T>
Stack<T>::~Stack(){}


template<class T2>
std::ostream& operator << (std::ostream& outs, const Stack<T2>& s) {
  outs << "stack[";

  if (!s.empty()) {
    T2* temp = new T2[s.size()];
    int t = 0;
    for (ics::Iterator<T2>& i = s.ibegin(); i != s.iend(); ++i,++t)
      temp[t] = *i;
    for (t = s.size()-1; t >= 0; --t)
      outs << (t == s.size()-1 ? "" : ",") << temp[t];
  }

  outs <<"]:top";
  return outs;
}

}

#endif /* STACK_HPP_ */
