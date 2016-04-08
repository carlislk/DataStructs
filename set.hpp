#ifndef SET_HPP_
#define SET_HPP_

#include <iostream>
#include "iterator.hpp"


namespace ics {

template<class T> class Set  {
  public:
    virtual ~Set();

    virtual bool empty      () const = 0;
    virtual int  size       () const = 0;
    virtual bool contains   (const T& element) const = 0;
    virtual std::string str () const = 0;

    virtual bool contains (Iterator<T>& start, const Iterator<T>& stop) const = 0;

    virtual int  insert (const T& element) = 0;
    virtual int  erase  (const T& element) = 0;
    virtual void clear  () = 0;

    virtual int insert (Iterator<T>& start, const Iterator<T>& stop) = 0;
    virtual int erase  (Iterator<T>& start, const Iterator<T>& stop) = 0;
    virtual int retain (Iterator<T>& start, const Iterator<T>& stop) = 0;

    virtual bool operator == (const Set<T>& rhs) const = 0;
    virtual bool operator != (const Set<T>& rhs) const = 0;
    virtual bool operator <= (const Set<T>& rhs) const = 0;
    virtual bool operator <  (const Set<T>& rhs) const = 0;
    virtual bool operator >= (const Set<T>& rhs) const = 0;
    virtual bool operator >  (const Set<T>& rhs) const = 0;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const Set<T2>& s);

    virtual Iterator<T>& ibegin () const = 0;
    virtual Iterator<T>& iend   () const = 0;
  };





template<class T>
Set<T>::~Set(){}

template<class T2>
std::ostream& operator << (std::ostream& outs, const Set<T2>& s) {
  outs << "set[";

  if (!s.empty()) {
    ics::Iterator<T2>& i = s.ibegin();
    outs << *i ;
    ++i;
    for (; i != s.iend(); ++i)
      outs << "," << *i;
  }

  outs <<"]";
  return outs;
}

}

#endif /* SET_HPP_ */
