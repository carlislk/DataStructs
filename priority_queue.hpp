#ifndef PRIORITY_QUEUE_HPP_
#define PRIORITY_QUEUE_HPP_

#include <iostream>
#include "queue.hpp"


namespace ics {

template<class T> class PriorityQueue : public Queue<T> {
  public:
    virtual ~PriorityQueue();

    virtual bool empty      () const = 0;
    virtual int  size       () const = 0;
    virtual T&   peek       () const = 0;
    virtual std::string str () const = 0;

    virtual int  enqueue (const T& element) = 0;
    virtual T    dequeue () = 0;
    virtual void clear   () = 0;

    virtual int enqueue (Iterator<T>& start, const Iterator<T>& stop) = 0;

    //A PriorityQueue is never equal to a Queue; not virtual because not overridable
    bool operator == (const Queue<T>& rhs) const {return false;}
    bool operator != (const Queue<T>& rhs) const {return false;}

    virtual bool operator == (const PriorityQueue<T>& rhs) const = 0;
    virtual bool operator != (const PriorityQueue<T>& rhs) const = 0;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const PriorityQueue<T2>& p);

    virtual Iterator<T>& ibegin () const = 0;
    virtual Iterator<T>& iend   () const = 0;

  //KLUDGE: should be protected but != not work in operator== in derived classes: e.g., ArrayPriorityWQueue
  public:
    PriorityQueue(bool (*agt)(const T& a, const T& b)) : gt(agt) {}
    bool (*gt)(const T& a, const T& b);  // gt(a,b) = true iff a has higher priority than b
  };





template<class T>
PriorityQueue<T>::~PriorityQueue(){}


template<class T2>
std::ostream& operator << (std::ostream& outs, const PriorityQueue<T2>& p) {
  outs << "priority_queue[";

  if (!p.empty()) {
    T2* temp = new T2[p.size()];
    int t = 0;
    for (ics::Iterator<T2>& i = p.ibegin(); i != p.iend(); ++i,++t)
      temp[t] = *i;
    for (t = p.size()-1; t >= 0; --t)
      outs << (t == p.size()-1 ? "" : ",") << temp[t];
  }

  outs <<"]:highest";
  return outs;
}

}

#endif /* PRIORITY_QUEUE_HPP_ */
