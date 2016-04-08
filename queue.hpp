#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <iostream>
#include "iterator.hpp"


namespace ics {

template<class T> class Queue  {
  public:
    virtual ~Queue();

    virtual bool empty      () const = 0;
    virtual int  size       () const = 0;
    virtual T&   peek       () const = 0;
    virtual std::string str () const = 0;

    virtual int  enqueue (const T& element) = 0;
    virtual T    dequeue () = 0;
    virtual void clear   () = 0;

    virtual int enqueue (Iterator<T>& start, const Iterator<T>& stop) = 0;

    virtual bool operator == (const Queue<T>& rhs) const = 0;
    virtual bool operator != (const Queue<T>& rhs) const = 0;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const Queue<T2>& q);

    virtual Iterator<T>& ibegin () const = 0;
    virtual Iterator<T>& iend   () const = 0;
};





template<class T>
Queue<T>::~Queue(){}


template<class T2>
std::ostream& operator << (std::ostream& outs, const Queue<T2>& q) {
  outs << "queue[";

  if (!q.empty()) {
    ics::Iterator<T2>& i = q.ibegin();
    outs << *i ;
    ++i;
    for (; i != q.iend(); ++i)
      outs << "," << *i;
  }

  outs <<"]:rear";
  return outs;
}

}

#endif /* QUEUE_HPP_ */
