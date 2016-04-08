#ifndef ITERATOR_HPP_
#define ITERATOR_HPP_

#include <string>

namespace ics {

template<class T>
class Iterator {
  public:
    virtual ~Iterator(){}
    virtual T           erase() = 0;
    virtual std::string str  () const = 0;
    virtual const Iterator<T>& operator ++ ()    = 0;
    virtual const Iterator<T>& operator ++ (int) = 0;
    virtual bool operator == (const Iterator<T>& rhs) const = 0;
    virtual bool operator != (const Iterator<T>& rhs) const = 0;
    virtual T& operator *  () const  = 0;
    virtual T* operator -> () const = 0;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const Iterator<T2>& s);
};





template<class T2>
std::ostream& operator << (std::ostream& outs, const Iterator<T2>& s) {
  outs << s.str(); //Same as debugging
  return outs;
}

}

#endif /* ITERATOR_HPP_ */
